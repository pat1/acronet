/*
 * ACRONET Project
 * http://www.acronet.cc
 *
 * Copyright ( C ) 2014 Acrotec srl
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the EUPL v.1.1 license.  See http://ec.europa.eu/idabc/eupl.html for details.
 *
 * 
 * 
 */

#include "Acronet/setup.h"


#include <asf.h>


#include "Acronet/globals.h"
#include <stdio.h>
#include <conf_board.h>
#include <conf_usart_serial.h>
#include "config/conf_usart_serial.h"
#include "Acronet/services/NMEA/nmea.h"
#include "Acronet/drivers/SP336/SP336.h"
#include "Acronet/Sensors/CV7L/CV7L.h"
#include "Acronet/services/config/config.h"
#include "Acronet/drivers/StatusLED/status_led.h"
#include "Acronet/drivers/UART_INT/cbuffer_usart.h"


#define NMEA_SENTENCE_MAX_LENGTH 90

enum {CV7L_STAT_OPERATOR_MEAN,CV7L_STAT_OPERATOR_MAX,CV7L_STAT_OPERATOR_MIN,CV7L_STAT_OPERATOR_GUST_SPEED,CV7L_STAT_OPERATOR_END};


typedef struct {
	char m_fmt[8];
	uint16_t m_fct;
	uint8_t m_oper;
} CV7L_STAT_FORMAT;


typedef struct
{
	float data[CV7L_STAT_END];
	uint8_t samples[CV7L_STAT_END];
	float wind_gust[128];

	volatile uint8_t sig_data_busy;
} CV7L_PRIVATE_DATA;


static const CV7L_STAT_FORMAT g_stat_fmt[CV7L_STAT_END]  PROGMEM = {	{"WTav=%d", 10 ,CV7L_STAT_OPERATOR_MEAN},
																		{"WTmx=%d", 10 ,CV7L_STAT_OPERATOR_MAX },
																		{"WTmn=%d", 10 ,CV7L_STAT_OPERATOR_MIN },
																		{"Wdav=%d", 10 ,CV7L_STAT_OPERATOR_MEAN},
																		{"Wsav=%d", 10 ,CV7L_STAT_OPERATOR_MEAN},
																		{"Wsmx=%d", 10 ,CV7L_STAT_OPERATOR_GUST_SPEED} };



static const char szNMEA_IIMWV[]			PROGMEM = "$IIMWV";
static const char szNMEA_WIXDR[]			PROGMEM = "$WIXDR";
//static const char szNMEA_PLCJE83C8[]		PROGMEM = "$PLCJE83C8";
//static const char szNMEA_PLCJ[]				PROGMEM = "$PLCJ";


static const char * const tbl_NMEAin[] PROGMEM = {	
													szNMEA_IIMWV,
													szNMEA_WIXDR//,
													//szNMEA_PLCJE83C8,
													//szNMEA_PLCJ
												};


enum {	NMEA_FIRST_ENTRY = 0, 
		NMEA_IIMWV = NMEA_FIRST_ENTRY,
		NMEA_WIXDR,	//NMEA_PLCJE83C8,	NMEA_PLCJ,
		NMEA_LAST_ENTRY
	};

#define NUM_OF_NMEA_in (sizeof(tbl_NMEAin)/sizeof(char *))

static void CV7L_NMEA_Handler_IIMWV(CV7L_PRIVATE_DATA * const pSelf,char * const psz);
static void CV7L_NMEA_Handler_WIXDR(CV7L_PRIVATE_DATA * const pSelf,char * const psz);


typedef void (*NMEA_FN_HANDLER)(CV7L_PRIVATE_DATA * const ,char * const);

static const NMEA_FN_HANDLER tbl_NMEAfn[] PROGMEM = {
												CV7L_NMEA_Handler_IIMWV,
												CV7L_NMEA_Handler_WIXDR
												};
	
	

RET_ERROR_CODE CV7L_Data2String(const CV7L_DATA * const st,char * const sz, uint16_t * const len_sz)
{
	uint16_t len = 0;
	char sBuf[12] = "&";

	char * psz = sBuf;
	char * const p = sBuf+1;

	for (uint8_t ix=CV7L_STAT_BEG;ix<CV7L_STAT_END;++ix)
	{

		CV7L_STAT_FORMAT af;

		nvm_flash_read_buffer((flash_addr_t)&g_stat_fmt[ix],&af,sizeof(CV7L_STAT_FORMAT));
		strncpy(p,af.m_fmt,sizeof(af.m_fmt));

		
		float vf = st->m_stat[ix];
		if(-9999.0F!=vf) { vf*=af.m_fct; }
		const uint32_t vi = (uint32_t) vf;
		len += snprintf(sz+len,*len_sz-len,psz,vi);
		if(len>=*len_sz) {return AC_BUFFER_OVERFLOW;}  
		//psz = sBuf;
	}
	

	*len_sz = len;

	return AC_ERROR_OK;
}

//#define SWITCH_PIN					IOPORT_CREATE_PIN(PORTD, 1)

static void	CV7L_powercycle(void)
{
//	return;
	//gpio_toggle_pin(SWITCH_PIN);
	//delay_ms(3000);
	//gpio_toggle_pin(SWITCH_PIN);
	//delay_ms(3000);
}



static void CV7L_process_NMEA_Statement(CV7L_PRIVATE_DATA * const pSelf,char * const psz)
{
	const uint8_t le = NMEA_LAST_ENTRY;

	for(uint8_t i = NMEA_FIRST_ENTRY;i<le;++i)	{
		char * p = (char *) nvm_flash_read_word( (flash_addr_t) (tbl_NMEAin+i) );
		if (0==strncasecmp_P(psz,p,6))
		{
			//char szBuf[64];
			//sprintf_P(szBuf,PSTR("string = %s\r\n"),psz);
			//debug_string(NORMAL,szBuf,RAM_STRING);
			NMEA_FN_HANDLER fn = (NMEA_FN_HANDLER) nvm_flash_read_word( (flash_addr_t) (tbl_NMEAfn+i) );
			fn(pSelf,psz);
		}
	}
}


#define WIND_GUST_SAMPLES_WINDOW 6

static float CV7L_compute_stats(CV7L_PRIVATE_DATA * const pSelf,const uint8_t id)
{
	if (pSelf->samples[id]==0) { return -9999.0F; }

	const uint8_t op = nvm_flash_read_byte( (flash_addr_t) &g_stat_fmt[id].m_oper);
	
	if(op==CV7L_STAT_OPERATOR_MEAN) {
		return pSelf->data[id] / pSelf->samples[id];
	}
	float gust_max = 0;
	if(op==CV7L_STAT_OPERATOR_GUST_SPEED) {
		if (pSelf->samples[id] < WIND_GUST_SAMPLES_WINDOW)
			return -9999.0F;
		for (uint8_t i = 0; i <= (pSelf->samples[id])-WIND_GUST_SAMPLES_WINDOW; i++) {
			float gust_temp = 0;
			for (uint8_t j = 0; j < 6 ; j++) {
				gust_temp += pSelf->wind_gust[i+j];
			}
			if (gust_temp > gust_max)
				gust_max = gust_temp;
		}
		gust_max/=WIND_GUST_SAMPLES_WINDOW;
		return gust_max;
	}
	
	//const uint32_t vi = (uint32_t) g_data[id];
	//char szBuf[128];
	//sprintf_P(szBuf,PSTR("%d - %lu\r\n"),id,vi);
	//debug_string(NORMAL,szBuf,RAM_STRING);
	
	return pSelf->data[id];
}


static RET_ERROR_CODE CV7L_get_data(CV7L_PRIVATE_DATA * const pSelf,CV7L_DATA * const ps)
{
	irqflags_t flags = cpu_irq_save();
	simple_signal_wait(&(pSelf->sig_data_busy));
	SIGNAL_SET_AND_CLEAR_AUTOMATIC((pSelf->sig_data_busy));
	cpu_irq_restore(flags);

	for(uint8_t ix = CV7L_STAT_BEG;ix<CV7L_STAT_END;++ix)
	{
		ps->m_stat[ix] = CV7L_compute_stats(pSelf,ix);
	}

	return AC_ERROR_OK;
}


static RET_ERROR_CODE CV7L_reset_data(CV7L_PRIVATE_DATA * const pSelf)
{
	irqflags_t flags = cpu_irq_save();
	simple_signal_wait(&(pSelf->sig_data_busy));
	SIGNAL_SET_AND_CLEAR_AUTOMATIC(pSelf->sig_data_busy);
	cpu_irq_restore(flags);

	for(uint8_t ix = CV7L_STAT_BEG;ix<CV7L_STAT_END;++ix)
	{
		pSelf->samples[ix] = 0;
	}


	return AC_ERROR_OK;
}

static void CV7L_NMEA_UpdateStats(CV7L_PRIVATE_DATA * const pSelf,const uint8_t id,const char * const p)
{
	const float val = atof(p);
	uint8_t op;
	//const uint32_t vi = (uint32_t) val;
	//char szBuf[128];
	//sprintf_P(szBuf,PSTR("%d - a = %s - atoi = %lu\r\n"),id,p,vi);
	//debug_string(NORMAL,szBuf,RAM_STRING);
	
	op = nvm_flash_read_byte( (flash_addr_t) &g_stat_fmt[id].m_oper);
	
	//static float gust_temp = -9999.0F;

	irqflags_t flags = cpu_irq_save();
	simple_signal_wait(&(pSelf->sig_data_busy));
	SIGNAL_SET_AND_CLEAR_AUTOMATIC(pSelf->sig_data_busy);
	cpu_irq_restore(flags);

	
	if(op==CV7L_STAT_OPERATOR_MEAN) {
		
		if(pSelf->samples[id]==0) {
			pSelf->data[id]=val;
			} else {
			pSelf->data[id]+=val;
		}
		
	} else if(op==CV7L_STAT_OPERATOR_MAX) {
		
		if(pSelf->samples[id]==0) {
			pSelf->data[id]=val;
			} else {
			if(pSelf->data[id]<val) {pSelf->data[id]=val;}
		}
		
	} else if(op==CV7L_STAT_OPERATOR_MIN) {

		if(pSelf->samples[id]==0) {
			pSelf->data[id]=val;
			} else {
			if(pSelf->data[id]>val) {pSelf->data[id]=val;}  
			}
	} else if(op==CV7L_STAT_OPERATOR_GUST_SPEED) {
		if (pSelf->samples[id] < sizeof(pSelf->wind_gust))
		{
			pSelf->wind_gust[pSelf->samples[id]] = val;
		}
	}

	pSelf->samples[id]++;

}

static void CV7L_process_temperature(CV7L_PRIVATE_DATA * const pSelf,const char * const p)
{
	CV7L_NMEA_UpdateStats(pSelf,CV7L_STAT_TEMPERATURE,p);
	CV7L_NMEA_UpdateStats(pSelf,CV7L_STAT_TEMPERATURE_MAX,p);
	CV7L_NMEA_UpdateStats(pSelf,CV7L_STAT_TEMPERATURE_MIN,p);
}

static void CV7L_process_WinDir(CV7L_PRIVATE_DATA * const pSelf,const char * const p)
{
	CV7L_NMEA_UpdateStats(pSelf,CV7L_STAT_WINDIR,p);
	
}

static void CV7L_process_WindSpeed(CV7L_PRIVATE_DATA * const pSelf,const char * const p)
{
	CV7L_NMEA_UpdateStats(pSelf,CV7L_STAT_WINSPEED,p);
	CV7L_NMEA_UpdateStats(pSelf,CV7L_STAT_WINSPEED_GUST,p);
}

static void CV7L_NMEA_Handler_IIMWV(CV7L_PRIVATE_DATA * const pSelf,char * const psz)
{
	char * p = psz+7;
	char * q;
	uint8_t v;
	v = NMEALine_Tokenize(p,&q); //Wind Angle 000.0° to 359.0°
	if(v!=0) {CV7L_process_WinDir(pSelf,p);}
		
	v = NMEALine_Tokenize(q,&p); //Relative reference.
	
	v = NMEALine_Tokenize(p,&q); //Wind speed.
	if(v!=0) {CV7L_process_WindSpeed(pSelf,p);}
		
	v = NMEALine_Tokenize(q,&p); //Wind Speed unit N = knots, M = m/s, K = km/h.
	v = NMEALine_Tokenize(p,&q); //Status of CV3F A = available, V = alarm.
}

static void CV7L_NMEA_Handler_WIXDR(CV7L_PRIVATE_DATA * const pSelf,char * const psz)
{
	char * p = psz+7;
	char * q;
	uint8_t v;
	v = NMEALine_Tokenize(p,&q); //Unknown
	
	v = NMEALine_Tokenize(q,&p); //Temperature
	if(v!=0) {CV7L_process_temperature(pSelf,q);} 
		
	v = NMEALine_Tokenize(p,&q); //Unit of measure C = Celsius, F = Fahrenheit
	v = NMEALine_Tokenize(q,&p); //Empty
}

//static void CV7L_NMEA_Handler_PLCJE83C8(char * const psz)
//{
	//DEBUG_PRINT_FUNCTION_NAME(NORMAL,"NMEA_Handler_PLCJE83C8 (empty)");
//}
//
//static void CV7L_NMEA_Handler_PLCJ(char * const psz)
//{
	//DEBUG_PRINT_FUNCTION_NAME(NORMAL,"NMEA_Handler_PLCJ (empty)");
//}

#define MODULE_INTERFACE_PRIVATE_DATATYPE CV7L_PRIVATE_DATA


#define MODINST_PARAM_ID MOD_ID_CV7L
#include "Acronet/datalogger/modinst/module_interface_definition.h"
#undef MODINST_PARAM_ID