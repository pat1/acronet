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
#include "Acronet/channels/NMEA/nmea.h"
#include "Acronet/drivers/SP336/SP336.h"
#include "Acronet/Sensors/L8095-N/L8095-N.h"
#include "Acronet/services/config/config.h"
#include "Acronet/drivers/StatusLED/status_led.h"
#include "Acronet/drivers/UART_INT/cbuffer_usart.h"


/*

Sentence	| Description							| Enabled by Default	
------------+---------------------------------------+-----------------------
$WIMTA		| Air temperatura						|	YES					
------------+---------------------------------------+-----------------------
$WIMMB		| Barometric pressure					|	YES					
------------+---------------------------------------+-----------------------
$WIMHU		| Relative Humidity and Dew Point		|	YES					

*/

#define NMEA_SENTENCE_MAX_LENGTH 90


enum {L8095N_STAT_OPERATOR_MEAN,L8095N_STAT_OPERATOR_MAX,L8095N_STAT_OPERATOR_MIN,L8095N_STAT_OPERATOR_END};

typedef struct
{
	float   g_data   [L8095N_STAT_END];
	uint8_t g_samples[L8095N_STAT_END];
/*
	volatile char g_szNMEALine[1024];
	volatile uint16_t g_idxBufferNMEALine;
	volatile uint16_t g_idxProcessNMEALine;
*/
	volatile uint8_t sig_data_busy;
	
} L8095N_PRIVATE_DATA;


typedef struct {
	char m_fmt[8];
	uint16_t m_fct;
	uint8_t m_oper;
} L8095N_STAT_FORMAT;


static const L8095N_STAT_FORMAT g_stat_fmt[L8095N_STAT_END]  PROGMEM = {	{"Pav=%ld", 10 ,L8095N_STAT_OPERATOR_MEAN},
																			{"Tav=%ld", 10 ,L8095N_STAT_OPERATOR_MEAN},
																			{"Tmx=%ld", 10 ,L8095N_STAT_OPERATOR_MAX },
																			{"Tmn=%ld", 10 ,L8095N_STAT_OPERATOR_MIN },
																			{"RH=%ld",  10 ,L8095N_STAT_OPERATOR_MEAN},
																			{"DP=%ld",  10 ,L8095N_STAT_OPERATOR_MEAN} };



static const char szNMEA_WIMTA[]		PROGMEM = "$WIMTA";
static const char szNMEA_WIMMB[]		PROGMEM = "$WIMMB";
static const char szNMEA_WIMHU[]		PROGMEM = "$WIMHU";


static const char * const tbl_NMEAin[] PROGMEM = {	
													szNMEA_WIMTA,
													szNMEA_WIMMB,
													szNMEA_WIMHU
												};


enum {	NMEA_FIRST_ENTRY = 0, 
		NMEA_WIMTA = NMEA_FIRST_ENTRY,
		NMEA_WIMMB,
		NMEA_WIMHU,
		NMEA_LAST_ENTRY
	};

#define NUM_OF_NMEA_in (sizeof(tbl_NMEAin)/sizeof(char *))

static void L8095N_NMEA_Handler_WIMTA(L8095N_PRIVATE_DATA * const pSelf,char * const psz);
static void L8095N_NMEA_Handler_WIMMB(L8095N_PRIVATE_DATA * const pSelf,char * const psz);
static void L8095N_NMEA_Handler_WIMHU(L8095N_PRIVATE_DATA * const pSelf,char * const psz);

typedef void (*NMEA_FN_HANDLER)(L8095N_PRIVATE_DATA * const pSelf,char * const);

static const NMEA_FN_HANDLER tbl_NMEAfn[] PROGMEM = {
												L8095N_NMEA_Handler_WIMTA,
												L8095N_NMEA_Handler_WIMMB,
												L8095N_NMEA_Handler_WIMHU
												};
	
	
	


//USART_data_t g_l8095n_usart_data;


#ifdef RMAP_SERVICES

RET_ERROR_CODE l8095n_Data2String_RMAP(	 uint8_t * const subModule
										,const L8095N_DATA * const st
										,const uint32_t timeStamp
										,const uint16_t timeWindow
										,char * const szTopic
										,int16_t * const len_szTopic
										,char * const szMessage
										,int16_t * const len_szMessage )
{
	typedef struct { char topic[32]; float factor; float offset; uint8_t next; } RMAPDATA;

	static const __flash RMAPDATA mydata[L8095N_STAT_END]= {
															  {"/0,0,%d/103,2000,-,-/B10004", 10.0F,    0.0F,L8095N_STAT_TEMPERATURE}	//PRESSURE
															, {"/0,0,%d/103,2000,-,-/B12101",100.0F,27315.0F,L8095N_STAT_RH}			//TEMPERATURE
															, {"/2,0,%d/103,2000,-,-/B12101",100.0F,27315.0F,255}						//TEMPERATURE MAX
															, {"/3,0,%d/103,2000,-,-/B12101",100.0F,27315.0F,255}						//TEMPERATURE MIN
															, {"/0,0,%d/103,2000,-,-/B13003",  1.0F,     0.0F,L8095N_STAT_DEWPOINT}		//RH
															, {"/0,0,%d/103,2000,-,-/B12103",100.0F,27315.0F,255}						//DEWPOINT
														};
	

	
	const uint8_t ix = *subModule;
	
	if ( ix < L8095N_STAT_END )
	{
		(*subModule) = mydata[ix].next;
		
		struct calendar_date dt;
		calendar_timestamp_to_date(timeStamp,&dt);
			
		int16_t len = snprintf_P(	szTopic,
									*len_szTopic,
									mydata[ix].topic,
									timeWindow	);


		if(len >= *len_szTopic)
		{
			return AC_BUFFER_OVERFLOW;
		}

		*len_szTopic = len;

		if(st->m_stat[ix]!=-9999.0F) {
			const int32_t vf = (st->m_stat[ix]==-9999.0F) ? -9999 : ((int32_t) ((st->m_stat[ix] * mydata[ix].factor) + mydata[ix].offset));

			len = snprintf_P(	szMessage,
								*len_szMessage,
								PSTR("{\"v\":%ld, \"t\":\"%d-%02d-%02dT%02d:%02d:%02d\"}"),
								vf,dt.year,dt.month+1,dt.date+1,dt.hour,dt.minute,0);//dt.second-1 );
		} else {
			len = snprintf_P(	szMessage,
								*len_szMessage,
								PSTR("{\"v\":null, \"t\":\"%d-%02d-%02dT%02d:%02d:%02d\"}"),
								dt.year,dt.month+1,dt.date+1,dt.hour,dt.minute,0);//dt.second-1 );
		}


		if(len >= *len_szMessage)
		{
			return AC_BUFFER_OVERFLOW;
		}

		*len_szMessage = len;

	
	} else {
		return AC_ERROR_GENERIC;
	}
	 

	return AC_ERROR_OK;
}

#endif //RMAP_SERVICES

RET_ERROR_CODE l8095n_Data2String(const L8095N_DATA * const st,char * const sz, uint16_t * const len_sz)
{
	uint16_t len = 0;
	char sBuf[12] = "&";

	char * psz = sBuf;
	char * const p = sBuf+1;

	for (uint8_t ix=L8095N_STAT_BEG;ix<L8095N_STAT_END;++ix)
	{

		L8095N_STAT_FORMAT af;

		nvm_flash_read_buffer((flash_addr_t)&g_stat_fmt[ix],&af,sizeof(L8095N_STAT_FORMAT));
		strncpy(p,af.m_fmt,sizeof(af.m_fmt));

		
		float vf = st->m_stat[ix];
		if(-9999.0F!=vf) { vf*=af.m_fct; }
		const int32_t vi = (int32_t) vf;
		len += snprintf(sz+len,*len_sz-len,psz,vi);
		if(len>=*len_sz) {return AC_BUFFER_OVERFLOW;}  
		//psz = sBuf;
	}
	

	*len_sz = len;

	return AC_ERROR_OK;
}

//#define SWITCH_PIN					IOPORT_CREATE_PIN(PORTD, 1)

static void	l8095n_powercycle(void)
{
//	return;
	//gpio_toggle_pin(SWITCH_PIN);
	//delay_ms(3000);
	//gpio_toggle_pin(SWITCH_PIN);
	//delay_ms(3000);
}

/*
static RET_ERROR_CODE l8095n_init(void)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"L8095N Init");
	NMEALine_reset();
	l8095n_reset_data();
		
	//usart_interruptdriver_initialize(&g_l8095n_usart_data,L8095N_PUSART,USART_INT_LVL_LO);

//	ioport_configure_pin(SWITCH_PIN, IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);

	l8095n_powercycle();
	
	//L8095N sensor
	//ioport_configure_pin(L8095N_PIN_TX_ENABLE, IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
	//ioport_configure_pin(L8095N_PIN_TX_SIGNAL, IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
	//ioport_configure_pin(L8095N_PIN_RX_SIGNAL, IOPORT_DIR_INPUT);
//
	//
	//static usart_rs232_options_t usart_options = {
		//.baudrate = 4800,
		//.charlength = USART_CHSIZE_8BIT_gc,
		//.paritytype = USART_PMODE_DISABLED_gc,
		//.stopbits = false
	//};
	//
	//
	//
	//SP336_Config(L8095N_PUSART,&usart_options);
	//usart_tx_enable(L8095N_PUSART);
	//SP336_RegisterCallback(L8095N_PUSART,l8095n_rx);
//
	//usart_rx_disable(L8095N_PUSART);
	//usart_tx_disable(L8095N_PUSART);

	return AC_ERROR_OK;
}
*/


static void l8095n_process_NMEA_Statement(L8095N_PRIVATE_DATA * const pSelf,char * const psz)
{
	const uint8_t le = NMEA_LAST_ENTRY;

	for(uint8_t i = NMEA_FIRST_ENTRY;i<le;++i)	{
		char * p = (char *) nvm_flash_read_word( (flash_addr_t) (tbl_NMEAin+i) );
		if (0==strncasecmp_P(psz,p,6))
		{
			NMEA_FN_HANDLER fn = nvm_flash_read_word( (flash_addr_t) (tbl_NMEAfn+i) );
			fn(pSelf,psz);
		}
	}
}

/*
bool l8095n_Yield( void )
{
	static char szBuf[128];
	static uint8_t idx = 0;
	char c;
	while((c = NMEALine_getChar()) != 0)
	{
		if (c=='\r')	{
			szBuf[idx]=0;
			if(0==NMEALine_checksum_check(szBuf,idx)) {
				l8095n_process_NMEA_Statement(szBuf);
			}
			idx=0;
		} else if (c=='$') {
			szBuf[0]='$';
			szBuf[1]=0;
			idx = 1;
		} else if(idx>0) {
			szBuf[idx++] = c;
		}

		if (idx==(sizeof(szBuf)-1)) 
		{
			debug_string_2P(NORMAL,PSTR("l8095n_Yield"),PSTR("Synchronization lost"));
			szBuf[0]=0;
			idx = 0;
		}
		
		return true;
	}
	return false;
}

void l8095n_rx(const char c)
{
	//usart_putchar(USART_DEBUG,c);
	NMEALine_addChar(c);
}
*/


static float l8095n_compute_stats(L8095N_PRIVATE_DATA * const pSelf,const uint8_t id)
{
	if (pSelf->g_samples[id]==0) { return -9999.0F; }
	

	const uint8_t op = nvm_flash_read_byte( (flash_addr_t) &g_stat_fmt[id].m_oper);
	
	if(op==L8095N_STAT_OPERATOR_MEAN) {
		return pSelf->g_data[id] / pSelf->g_samples[id];
	} 
	
	return pSelf->g_data[id];
}


static RET_ERROR_CODE l8095n_get_data(L8095N_PRIVATE_DATA * const pSelf,L8095N_DATA * const ps)
{
	//simple_signal_wait(&(pSelf->sig_data_busy));
		//
	//SIGNAL_SET_AND_CLEAR_AUTOMATIC((pSelf->sig_data_busy));

	for(uint8_t ix = L8095N_STAT_BEG;ix<L8095N_STAT_END;++ix)
	{
		ps->m_stat[ix] = l8095n_compute_stats(pSelf,ix);
	}

	return AC_ERROR_OK;
}


static RET_ERROR_CODE l8095n_reset_data(L8095N_PRIVATE_DATA * const pSelf)
{
	//simple_signal_wait(&(pSelf->sig_data_busy));
	//
	//SIGNAL_SET_AND_CLEAR_AUTOMATIC((pSelf->sig_data_busy));

	for(uint8_t ix = L8095N_STAT_BEG;ix<L8095N_STAT_END;++ix)
	{
		pSelf->g_samples[ix] = 0;
	}


	return AC_ERROR_OK;
}

static void L8095N_NMEA_UpdateStats(L8095N_PRIVATE_DATA * const pSelf,const uint8_t id,const char * const p)
{
	const float val = atof(p);
	uint8_t op;

	op = nvm_flash_read_byte( (flash_addr_t) &g_stat_fmt[id].m_oper);

	//static float gust_temp = -9999.0F;
	
	//simple_signal_wait(&(pSelf->sig_data_busy));
	//
	//SIGNAL_SET_AND_CLEAR_AUTOMATIC((pSelf->sig_data_busy));
	
	if(op==L8095N_STAT_OPERATOR_MEAN) {
		
		if(pSelf->g_samples[id]==0) {
			pSelf->g_data[id]=val;
		} else {
			pSelf->g_data[id]+=val;
		}
		
	} else if(op==L8095N_STAT_OPERATOR_MAX) {
		
		if(pSelf->g_samples[id]==0) {
			pSelf->g_data[id]=val;
		} else {
			if(pSelf->g_data[id]<val) {pSelf->g_data[id]=val;}
		}
		
	} else if(op==L8095N_STAT_OPERATOR_MIN) {

		if(pSelf->g_samples[id]==0) {
			pSelf->g_data[id]=val;
		} else {
			if(pSelf->g_data[id]>val) {pSelf->g_data[id]=val;}  
		}

	}

	pSelf->g_samples[id]++;

}

static void l8095n_process_Barometric_pressure(L8095N_PRIVATE_DATA * const pSelf,const char * const p)
{
	L8095N_NMEA_UpdateStats(pSelf,L8095N_STAT_PRESSURE,p);
}

static void l8095n_process_temperature(L8095N_PRIVATE_DATA * const pSelf,const char * const p)
{
	L8095N_NMEA_UpdateStats(pSelf,L8095N_STAT_TEMPERATURE,p);
	L8095N_NMEA_UpdateStats(pSelf,L8095N_STAT_TEMPERATURE_MAX,p);
	L8095N_NMEA_UpdateStats(pSelf,L8095N_STAT_TEMPERATURE_MIN,p);
}

static void l8095n_process_RH(L8095N_PRIVATE_DATA * const pSelf,const char * const p)
{
	L8095N_NMEA_UpdateStats(pSelf,L8095N_STAT_RH,p);
}

static void l8095n_process_DEWPoint(L8095N_PRIVATE_DATA * const pSelf,const char * const p)
{
	L8095N_NMEA_UpdateStats(pSelf,L8095N_STAT_DEWPOINT,p);
}

static void L8095N_NMEA_Handler_WIMTA(L8095N_PRIVATE_DATA * const pSelf,char * const psz)
{

	char * p = psz+7;
	char * q;
	uint8_t v;
	
	v = NMEALine_Tokenize(p,&q); //Air Temperature
	if(v!=0) {l8095n_process_temperature(pSelf,p);}
	
	v = NMEALine_Tokenize(q,&p); //Air Temperature, name of dimension ( C = celsius degrees)

}

static void L8095N_NMEA_Handler_WIMMB(L8095N_PRIVATE_DATA * const pSelf,char * const psz)
{
//	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"NMEA_Handler_WIMWD");
//	debug_string(NORMAL,psz,RAM_STRING);

	char * p = psz+7;
	char * q;
	uint8_t v;
	
	v = NMEALine_Tokenize(p,&q);	//Empty
	v = NMEALine_Tokenize(q,&p);	//Empty
	
	v = NMEALine_Tokenize(p,&q); //Barometric pressure, bars, to the nearest .1 bar hPA
	if(v!=0) {l8095n_process_Barometric_pressure(pSelf,p);}
	
	v = NMEALine_Tokenize(q,&p); //Barometric pressure, name of dimension ( B = hPa)

}

static void L8095N_NMEA_Handler_WIMHU(L8095N_PRIVATE_DATA * const pSelf,char * const psz)
{
	char * p = psz+7;
	char * q;
	uint8_t v;
	
	v = NMEALine_Tokenize(p,&q); //Relative humidity
	if(v!=0) {l8095n_process_RH(pSelf,p);}
	
	v = NMEALine_Tokenize(q,&p); //Empty
		
	v = NMEALine_Tokenize(p,&q); //Dew point
	if(v!=0) {l8095n_process_DEWPoint(pSelf,p);}
	
	v = NMEALine_Tokenize(q,&p); //Dew point, name of dimension ( C = celsius degrees)

	
}

#define MODULE_INTERFACE_PRIVATE_DATATYPE L8095N_PRIVATE_DATA

#define MODINST_PARAM_ID MOD_ID_L8095N
#include "Acronet/datalogger/modinst/module_interface_definition.h"
#undef MODINST_PARAM_ID
