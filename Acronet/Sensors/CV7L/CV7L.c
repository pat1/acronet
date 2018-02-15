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

static void CV7L_NMEA_Handler_IIMWV(char * const psz);
static void CV7L_NMEA_Handler_WIXDR(char * const psz);
//static void CV7L_NMEA_Handler_PLCJE83C8(char * const psz);
//static void CV7L_NMEA_Handler_PLCJ(char * const psz);


typedef void (*NMEA_FN_HANDLER)(char * const);

static const NMEA_FN_HANDLER tbl_NMEAfn[] PROGMEM = {
												CV7L_NMEA_Handler_IIMWV,
												CV7L_NMEA_Handler_WIXDR//,
												//CV7L_NMEA_Handler_PLCJE83C8,
												//CV7L_NMEA_Handler_PLCJ
												};
	
	
	


static void CV7L_rx(const char c);

static volatile char g_szNMEALine[1024];
static volatile uint16_t g_idxBufferNMEALine;
static volatile uint16_t g_idxProcessNMEALine;

static volatile uint8_t sig_data_busy = 0;

static void NMEALine_reset(void)
{
	g_szNMEALine[0] = 0;
	g_idxBufferNMEALine = 0;
	g_idxProcessNMEALine = 0;
}

static char NMEALine_getChar(void)
{
	
	//TODO: add crit section here ?
	
	if(g_idxBufferNMEALine == 0) return 0;
	const char c = g_szNMEALine[g_idxProcessNMEALine++];
	if(g_idxProcessNMEALine==g_idxBufferNMEALine) {
		NMEALine_reset();
	}

	return c;
}
 
 
static void NMEALine_addChar(const char c)
{
	if(g_idxBufferNMEALine<sizeof(g_szNMEALine)) {
		g_szNMEALine[g_idxBufferNMEALine++]=c;
	}
}

static uint8_t NMEALine_Tokenize(char * psz,char ** pNext)
{
	uint8_t i = 0;
	char * const p = psz;
	
	do {
		const char c = p[i];
		if(c==',') {
			p[i] = 0;
			*pNext = p+(i+1);
			return i;
		} 
		
		if(c==0) {
			*pNext = NULL;
			return i;
		}
	} while(++i<NMEA_SENTENCE_MAX_LENGTH);
	*pNext = NULL;
	return 0;
}

static uint8_t __attribute__((const)) ascii_hex(const uint8_t c)
{
	if((c>47) && (c<58)) { // 0 to 9  
		return (c-48);
		} else if((c>64) && (c<71)) { // A to F
		return (c-55);
		} else if((c>96) && (c<103)) { // a to f
		return (c-87);
	}
	
	//ERROR TO HANDLE
	
	return 0xFF;
}


static uint8_t NMEALine_checksum_check(char * const psz,const uint8_t len_sz)
{
	uint8_t ix = 0;
	uint8_t r = 0;
	if(len_sz<10) {
		return 1;
	}
	if(psz[ix]=='$') ix++;
	while (ix<len_sz)
	{
		const char c = psz[ix++];
		if(c=='*') break;
		r ^= c;
	}
	
	uint8_t cs = (ascii_hex(psz[ix])<<4) | ascii_hex(psz[ix+1]);
	if (r!=cs)
	{
		//debug_string_2P(VERBOSE,PSTR("CV7L NMEA CHECKSUM"),PSTR("MISMATCH"));
		return 0xFF;
	} 
	return 0;
}

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

void CV7L_enable( void )
{
	usart_rx_enable(CV7L_PUSART);
}

void CV7L_disable( void )
{
	usart_rx_disable(CV7L_PUSART);
}

#define SWITCH_PIN					IOPORT_CREATE_PIN(PORTD, 1)

static void	CV7L_powercycle(void)
{
//	return;
	gpio_toggle_pin(SWITCH_PIN);
	delay_ms(3000);
	gpio_toggle_pin(SWITCH_PIN);
	delay_ms(3000);
}


RET_ERROR_CODE CV7L_init(void)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"CV7L Init");
	NMEALine_reset();
	CV7L_reset_data();
		
	//usart_interruptdriver_initialize(&g_CV7L_usart_data,CV7L_PUSART,USART_INT_LVL_LO);

	ioport_configure_pin(SWITCH_PIN, IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);

	CV7L_powercycle();
	
	//CV7L sensor
	ioport_configure_pin(CV7L_PIN_TX_SIGNAL, IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
	ioport_configure_pin(CV7L_PIN_RX_SIGNAL, IOPORT_DIR_INPUT);

	
	static usart_rs232_options_t usart_options = {
		.baudrate = 4800,
		.charlength = USART_CHSIZE_8BIT_gc,
		.paritytype = USART_PMODE_DISABLED_gc,
		.stopbits = false
	};
	
	
	
	SP336_Config(CV7L_PUSART,&usart_options);
	usart_tx_enable(CV7L_PUSART);
	SP336_RegisterCallback(CV7L_PUSART,CV7L_rx);

	usart_rx_disable(CV7L_PUSART);
	usart_tx_disable(CV7L_PUSART);

	return AC_ERROR_OK;
}



static void CV7L_process_NMEA_Statement(char * const psz)
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
			fn(psz);
		}
	}
}

bool CV7L_Yield( void )
{
	static char szBuf[128];
	static uint8_t idx = 0;
	char c;
	while((c = NMEALine_getChar()) != 0)
	{
		if (c=='\r')	{
			szBuf[idx]=0;
			if(szBuf[1] != 'P' && 0==NMEALine_checksum_check(szBuf,idx)) { //szBuf[1] != 'P' esclude le due stringhe di controllo
				CV7L_process_NMEA_Statement(szBuf);
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
			
			debug_string_2P(NORMAL,PSTR("CV7L_Yield"),PSTR("Synchronization lost"));
			szBuf[0]=0;
			idx = 0;
		}
		
		return true;
	}
	return false;
}

void CV7L_rx(const char c)
{
//	usart_putchar(USART_DEBUG,c);
	NMEALine_addChar(c);
}

static float g_data[CV7L_STAT_END];
static uint8_t g_samples[CV7L_STAT_END];
static float g_wind_gust[128];

#define WIND_GUST_SAMPLES_WINDOW 6

static float CV7L_compute_stats(const uint8_t id)
{
	if (g_samples[id]==0) { return -9999.0F; }

	const uint8_t op = nvm_flash_read_byte( (flash_addr_t) &g_stat_fmt[id].m_oper);
	
	if(op==CV7L_STAT_OPERATOR_MEAN) {
		return g_data[id] / g_samples[id];
	}
	float gust_max = 0;
	if(op==CV7L_STAT_OPERATOR_GUST_SPEED) {
		if (g_samples[id] < WIND_GUST_SAMPLES_WINDOW)
			return -9999.0F;
		for (uint8_t i = 0; i <= g_samples[id]- WIND_GUST_SAMPLES_WINDOW; i++) {
			float gust_temp = 0;
			for (uint8_t j = 0; j < 6 ; j++) {
				gust_temp += g_wind_gust[i+j];
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
	
	return g_data[id];
}


RET_ERROR_CODE CV7L_get_data(CV7L_DATA * const ps)
{
	irqflags_t flags = cpu_irq_save();
	simple_signal_wait(&sig_data_busy);
	SIGNAL_SET_AND_CLEAR_AUTOMATIC(sig_data_busy);
	cpu_irq_restore(flags);

	for(uint8_t ix = CV7L_STAT_BEG;ix<CV7L_STAT_END;++ix)
	{
		ps->m_stat[ix] = CV7L_compute_stats(ix);
	}

	return AC_ERROR_OK;
}


RET_ERROR_CODE CV7L_reset_data(void)
{
	irqflags_t flags = cpu_irq_save();
	simple_signal_wait(&sig_data_busy);
	SIGNAL_SET_AND_CLEAR_AUTOMATIC(sig_data_busy);
	cpu_irq_restore(flags);

	for(uint8_t ix = CV7L_STAT_BEG;ix<CV7L_STAT_END;++ix)
	{
		g_samples[ix] = 0;
	}


	return AC_ERROR_OK;
}

static void CV7L_NMEA_UpdateStats(const uint8_t id,const char * const p)
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
	simple_signal_wait(&sig_data_busy);
	SIGNAL_SET_AND_CLEAR_AUTOMATIC(sig_data_busy);
	cpu_irq_restore(flags);

	
	if(op==CV7L_STAT_OPERATOR_MEAN) {
		
		if(g_samples[id]==0) {
			g_data[id]=val;
			} else {
			g_data[id]+=val;
		}
		
	} else if(op==CV7L_STAT_OPERATOR_MAX) {
		
		if(g_samples[id]==0) {
			g_data[id]=val;
			} else {
			if(g_data[id]<val) {g_data[id]=val;}
		}
		
	} else if(op==CV7L_STAT_OPERATOR_MIN) {

		if(g_samples[id]==0) {
			g_data[id]=val;
			} else {
			if(g_data[id]>val) {g_data[id]=val;}  
			}
	} else if(op==CV7L_STAT_OPERATOR_GUST_SPEED) {
		//ToDo: boundary check
		g_wind_gust[g_samples[id]] = val;
	}


	g_samples[id]++;

}

static void CV7L_process_temperature(const char * const p)
{
	CV7L_NMEA_UpdateStats(CV7L_STAT_TEMPERATURE,p);
	CV7L_NMEA_UpdateStats(CV7L_STAT_TEMPERATURE_MAX,p);
	CV7L_NMEA_UpdateStats(CV7L_STAT_TEMPERATURE_MIN,p);
}

static void CV7L_process_WinDir(const char * const p)
{
	CV7L_NMEA_UpdateStats(CV7L_STAT_WINDIR,p);
	
}

static void CV7L_process_WindSpeed(const char * const p)
{
	CV7L_NMEA_UpdateStats(CV7L_STAT_WINSPEED,p);
	CV7L_NMEA_UpdateStats(CV7L_STAT_WINSPEED_GUST,p);
}

static void CV7L_NMEA_Handler_IIMWV(char * const psz)
{
	char * p = psz+7;
	char * q;
	uint8_t v;
	v = NMEALine_Tokenize(p,&q); //Wind Angle 000.0° to 359.0°
	if(v!=0) {CV7L_process_WinDir(p);}
		
	v = NMEALine_Tokenize(q,&p); //Relative reference.
	
	v = NMEALine_Tokenize(p,&q); //Wind speed.
	if(v!=0) {CV7L_process_WindSpeed(p);}
		
	v = NMEALine_Tokenize(q,&p); //Wind Speed unit N = knots, M = m/s, K = km/h.
	v = NMEALine_Tokenize(p,&q); //Status of CV3F A = available, V = alarm.
}

static void CV7L_NMEA_Handler_WIXDR(char * const psz)
{
	char * p = psz+7;
	char * q;
	uint8_t v;
	v = NMEALine_Tokenize(p,&q); //Unknown
	
	v = NMEALine_Tokenize(q,&p); //Temperature
	if(v!=0) {CV7L_process_temperature(q);} 
		
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

