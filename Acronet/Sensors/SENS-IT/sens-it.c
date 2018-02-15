/*
 * ACRONET Project
 * http://www.acronet.cc
 *
 * Copyright ( C ) 2014 Acrotec srl
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the EUPL v.1.1 license.  See http://ec.europa.eu/idabc/eupl.html for details.
 */ 

#include "Acronet/setup.h"

#include <asf.h>

#include "Acronet/globals.h"
#include <stdio.h>
#include <conf_board.h>
#include <conf_usart_serial.h>
#include "config/conf_usart_serial.h"


#include "Acronet/drivers/SP336/SP336.h"
#include "Acronet/drivers/UART_INT/cbuffer_usart.h"
#include "Acronet/drivers/StatusLED/status_led.h"

#include "Acronet/Sensors/SENS-IT/sens-it.h"

#define SENSIT_PUSART			(&SP336_USART0)
#define SENSIT_PIN_TX_ENABLE	SP336_USART0_PIN_TX_ENABLE
#define SENSIT_PIN_TX_SIGNAL	SP336_USART0_PIN_TX_SIGNAL
#define SENSIT_PIN_RX_SIGNAL	SP336_USART0_PIN_RX_SIGNAL


#define SENTENCE_MAX_LENGTH 64
//#define SENSIT_SENSOR_NUM 4

#define SENSIT_SENSOR_NUM 2

static void sens_it_rx(const char c);
static char SENSITLine_getChar(void);

static volatile char g_szSENSITLine[1024];
static volatile uint16_t g_idxBufferSENSITLine;
static volatile uint16_t g_idxProcessSENSITLine;

//static const char g_sensIDs[] ="CN";
static char *g_sensIDs;

static void SENSITLine_reset(void)
{
	g_szSENSITLine[0] = 0;
	g_idxBufferSENSITLine = 0;
	g_idxProcessSENSITLine = 0;
}

typedef struct {
	char m_fmt[10];
	uint16_t m_fct;
	uint8_t m_oper;
} SENSIT_STAT_FORMAT;

enum {SENSIT_STAT_OPERATOR_MEAN,SENSIT_STAT_OPERATOR_MAX,SENSIT_STAT_OPERATOR_MIN,SENSIT_STAT_OPERATOR_END};


static const SENSIT_STAT_FORMAT g_stat_fmt[SENSIT_STAT_END]  PROGMEM ={	
																		{"CO=%d", 100 ,SENSIT_STAT_OPERATOR_MAX},
																		{"NO2=%d", 100 ,SENSIT_STAT_OPERATOR_MAX},
																		{"O3=%d", 100 ,SENSIT_STAT_OPERATOR_MAX},
																		{"CH4=%d", 100 ,SENSIT_STAT_OPERATOR_MAX},
																		{"NOx=%d",  100 ,SENSIT_STAT_OPERATOR_MAX},
																		{"C6H6=%d",  100 ,SENSIT_STAT_OPERATOR_MAX},
																	};

static const char sz_CO[]		PROGMEM = "a";
static const char sz_NO2[]		PROGMEM = "b";
static const char sz_O3[]		PROGMEM = "c";
static const char sz_CH4[]		PROGMEM = "d";
static const char sz_NOx[]		PROGMEM = "e";
static const char sz_C6H6[]		PROGMEM = "f";



static const char * const tbl_SENSITin[] PROGMEM = {
													sz_CO,
													sz_NO2,
													sz_O3,
													sz_CH4,
													sz_NOx,
													sz_C6H6
												};


enum {	SENSIT_FIRST_ENTRY = 0,
		SENSIT_CO,	SENSIT_NO2,	SENSIT_O3,
		SENSIT_CH4,	SENSIT_NOx,	SENSIT_C6H6,
		SENSIT_LAST_ENTRY
};

/* Aggiunta del 03/03/2015 */
static volatile uint8_t g_errors[SENSIT_LAST_ENTRY];
/* Fine aggiunta del 03/03/2015 */

#define NUM_OF_SENSIT_in (sizeof(tbl_SENSITin)/sizeof(char *))

static void Handler_CO(char * const psz);
static void Handler_NO2(char * const psz);
static void Handler_O3(char * const psz);
static void Handler_CH4(char * const psz);
static void Handler_NOx(char * const psz);
static void Handler_C6H6(char * const psz);

typedef void (*SENSIT_FN_HANDLER)(char * const);

static const SENSIT_FN_HANDLER tbl_SENSITfn[] PROGMEM = {
														Handler_CO,
														Handler_NO2,
														Handler_O3,
														Handler_CH4,
														Handler_NOx,
														Handler_C6H6
													};

RET_ERROR_CODE sens_it_init(void)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"SENS-IT Init");
	sens_it_reset_data();
	
	//SENSIT sensor
	ioport_configure_pin(SENSIT_PIN_TX_ENABLE, IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
	ioport_configure_pin(SENSIT_PIN_TX_SIGNAL, IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
	ioport_configure_pin(SENSIT_PIN_RX_SIGNAL, IOPORT_DIR_INPUT);

	
	static usart_rs232_options_t usart_options = {
		.baudrate = 9600,
		.charlength = USART_CHAR_LENGTH,
		.paritytype = USART_PARITY,
		.stopbits = false
	};
	
	
	
	SP336_Config(SENSIT_PUSART,&usart_options);
	usart_tx_enable(SENSIT_PUSART);
	SP336_RegisterCallback(SENSIT_PUSART,sens_it_rx);

	//Initialize sensor IDs array
	//g_sensIDs[0] = 'C';
	//g_sensIDs[1] = 'N';
	//g_sensIDs[2] = 'I';
	//g_sensIDs[3] = 'T';
	//g_sensIDs[SENSIT_SENSOR_NUM] = '\0';
	
	//Initialize sensor IDs array
	g_sensIDs[0] = 'V';
	g_sensIDs[1] = 'N';
	g_sensIDs[2] = 'J';
	g_sensIDs[3] = 'X';
	g_sensIDs[SENSIT_SENSOR_NUM] = '\0';
	
	
	usart_rx_disable(SENSIT_PUSART);
	usart_tx_disable(SENSIT_PUSART);

	return AC_ERROR_OK;
}

static void SENSIT_process_Statement(char * const psz)
{
	const uint8_t le = SENSIT_LAST_ENTRY;
	
	//char szBuf[256];
	//sprintf_P(szBuf,PSTR("SENSIT_process_Statement on : %s\r\n"),psz); //<----------------------------
	//debug_string(NORMAL,szBuf,false); //<----------------------------

	for(uint8_t i = SENSIT_FIRST_ENTRY;i<le;++i)	{
		char * p = (char *) nvm_flash_read_word( (flash_addr_t) tbl_SENSITin+i );
		if (0==strncasecmp_P(psz+20,p,1))
		{
			//debug_string_P(NORMAL,PSTR("GAS FOUND\r\n")); //<----------------------------
			SENSIT_FN_HANDLER fn = nvm_flash_read_word( (flash_addr_t) tbl_SENSITfn+i );
			//char errBuf[8];
			//errBuf[0] = psz[45];
			//errBuf[1] = psz[46];
			//errBuf[2] = psz[47];
			//errBuf[3] = '\r';
			//errBuf[4] = '\n';
			//errBuf[5] = 0;
			
			uint8_t error = 0;
			error += psz[47] - 48;
			error += (psz[46] - 48) * 10 ;
			error += (psz[45] - 48) * 100 ;
			g_errors[i] |= error;
			//sprintf_P(szBuf,PSTR("Error : %d\r\n"),error); //<----------------------------
			//debug_string(NORMAL,szBuf,false); //<----------------------------
			//sprintf_P(szBuf,PSTR("Error[%d] : %d\r\n"),i,g_errors[i]); //<----------------------------
			//debug_string(NORMAL,szBuf,false); //<----------------------------
			fn(psz);
		}
	}
}

static float g_stats[SENSIT_STAT_END];
static uint8_t g_samples[SENSIT_STAT_END];

static float sens_it_compute_stats(const uint8_t id)
{
	if (g_samples[id]==0) { return -9999.0F; }
	

	const uint8_t op = nvm_flash_read_byte( (flash_addr_t) &g_stat_fmt[id].m_oper);
	
	if(op==SENSIT_STAT_OPERATOR_MEAN) {
		return g_stats[id] / g_samples[id];
	}
	
	return g_stats[id];
}

RET_ERROR_CODE sens_it_get_data(SENSIT_STATS * const ps)
{
	for(uint8_t ix = SENSIT_STAT_BEG;ix<SENSIT_STAT_END;++ix)
	{
		ps->m_stat[ix] = sens_it_compute_stats(ix);
		ps->m_samples[ix] = g_samples[ix];
		ps->m_err[ix] = g_errors[ix];
	}
	sens_it_reset_data();
	
	return AC_ERROR_OK;
}

RET_ERROR_CODE sens_it_reset_data(void)
{
	for(uint8_t ix = SENSIT_STAT_BEG;ix<SENSIT_STAT_END;++ix)
	{
		g_samples[ix] = 0;
		g_errors[ix] = 0;
		g_stats[ix] = 0;
	}


	return AC_ERROR_OK;
}

RET_ERROR_CODE sens_it_Data2String(const SENSIT_STATS * const st,char * const sz, uint16_t * len_sz)
{
	uint16_t len = 0;
	char sBuf[12] = "&";

	char * psz = sBuf;
	char * const p = sBuf+1;

	for (uint8_t ix=SENSIT_STAT_BEG;ix<SENSIT_STAT_END;++ix)
	{

		SENSIT_STAT_FORMAT af;

		nvm_flash_read_buffer((flash_addr_t)&g_stat_fmt[ix],&af,sizeof(SENSIT_STAT_FORMAT));
		strncpy(p,af.m_fmt,sizeof(af.m_fmt));

		
		float vf = st->m_stat[ix];
		if(-9999.0F!=vf) { vf*=af.m_fct; }
		const uint32_t vi = (uint32_t) vf;
		len += snprintf(sz+len,*len_sz-len,psz,vi);
		if(len>=*len_sz) {return AC_BUFFER_OVERFLOW;}
		//psz = sBuf;
	}
	

	
	len += snprintf_P(sz+len,*len_sz,PSTR("&COn=%d&NO2n=%d&O3n=%d&CH4n=%d&NOxn=%d&C6H6n=%d"),
	st->m_samples[0],
	st->m_samples[1],
	st->m_samples[2],
	st->m_samples[3],
	st->m_samples[4],
	st->m_samples[5]
	);
	
	/* Aggiunta del 03/03/2015 */
	len += snprintf_P(sz+len,*len_sz,PSTR("&COe=%d&NO2e=%d&O3e=%d&CH4e=%d&NOxe=%d&C6H6e=%d"),
	st->m_err[0],
	st->m_err[1],
	st->m_err[2],
	st->m_err[3],
	st->m_err[4],
	st->m_err[5]
	);
	/* Fine aggiunta del 03/03/2015 */
	
	const RET_ERROR_CODE e = (len < *len_sz) ? AC_ERROR_OK : AC_BUFFER_OVERFLOW;
	*len_sz = len;
	return e;
}

void sens_it_enable(void)
{
	usart_rx_enable(SENSIT_PUSART);
}

void sens_it_disable(void)
{
	usart_rx_disable(SENSIT_PUSART);
}

bool sens_it_Yield(void)
{
	static char szBuf[128];
	static uint8_t idx = 0;
	static uint8_t it = 0;
	char c;
	while((c = SENSITLine_getChar()) != 0)
	{
		if (c=='\r')	{
			szBuf[idx]=0;
			SENSIT_process_Statement(szBuf);
			idx=0;
		} else if (c=='#') {
			szBuf[0]='#';
			szBuf[1]=0;
			idx = 1;
		} else if(idx>0) {
			szBuf[idx++] = c;
		}

		if (idx==(sizeof(szBuf)-1))
		{
			debug_string_2P(NORMAL,PSTR("SENSIT_Yield") ,PSTR("Synchronization lost"));
			szBuf[0]=0;
			idx = 0;
		}
		
		return true;
	}
	
	sens_it_triggerReading(g_sensIDs[it++]);
	if(it>=SENSIT_SENSOR_NUM)	it=0;
	return false;
}

void sens_it_triggerReading(uint8_t address)
{
	usart_tx_enable(SENSIT_PUSART);
	gpio_set_pin_high(SENSIT_PIN_TX_ENABLE);
	delay_us(500);
	usart_putchar(SENSIT_PUSART,'&');
	delay_ms(1);
	usart_putchar(SENSIT_PUSART,address);
	delay_ms(1);
	gpio_set_pin_low(SENSIT_PIN_TX_ENABLE);
	delay_ms(500);
}

static char SENSITLine_getChar(void)
{
	
	//TODO: add critical section here ?
	
	if(g_idxBufferSENSITLine == 0)
		return 0;
	const char c = g_szSENSITLine[g_idxProcessSENSITLine++];
	if(g_idxProcessSENSITLine==g_idxBufferSENSITLine) {
		SENSITLine_reset();
	}

	return c;
}


static void SENSITLine_addChar(const char c)
{
	if(g_idxBufferSENSITLine<sizeof(g_szSENSITLine)) {
		g_szSENSITLine[g_idxBufferSENSITLine++]=c;
	}
}

static void sens_it_rx(const char c)
{
	SENSITLine_addChar(c);
	usart_putchar(USART_DEBUG,c);
}



static uint8_t SENSITLine_Tokenize(char * psz,char ** pNext)
{
	//char szBuf[256]; //<----------------------------
	//sprintf_P(szBuf,PSTR("SENSITLine_Tokenize on : %s\r\n"),psz); //<----------------------------
	//debug_string(NORMAL,szBuf,false); //<----------------------------
	uint8_t i = 0;
	char * const p = psz;
	
	do {
		const char c = p[i];
		//if(c=='.') {
			//p[i] = ',';
		//}
		if(c==';') {
			p[i] = 0;
			*pNext = p+(i+1);
			return i;
		}
		
		if(c==0) {
			*pNext = NULL;
			return i;
		}
	} while(++i<SENTENCE_MAX_LENGTH);
	
	*pNext = NULL;
	return 0;
}

static void SENSIT_UpdateStats(const uint8_t id,const char * const p)
{
	//char szBuf[256]; //<----------------------------
	//sprintf_P(szBuf,PSTR("SENSIT_UpdateStats input string : %s\r\n"),p); //<----------------------------
	//debug_string(NORMAL,szBuf,false); //<----------------------------
	
	const float val = atof(p);
	
	//sprintf_P(szBuf,PSTR("SENSIT_UpdateStats input string : %d\r\n"),(uint16_t)(val*100)); //<----------------------------
	//debug_string(NORMAL,szBuf,false); //<----------------------------
	
	uint8_t op;
	
	op = nvm_flash_read_byte( (flash_addr_t) &g_stat_fmt[id].m_oper);
	
	if(op==SENSIT_STAT_OPERATOR_MEAN) {
		
		if(g_samples[id]==0) {
			g_stats[id]=val;
			} else {
			g_stats[id]+=val;
		}
		
		} else if(op==SENSIT_STAT_OPERATOR_MAX) {
		
		if(g_samples[id]==0) {
			g_stats[id]=val;
			} else {
			if(g_stats[id]<val) {g_stats[id]=val;}
		}
		
		} else if(op==SENSIT_STAT_OPERATOR_MIN) {

		if(g_samples[id]==0) {
			g_stats[id]=val;
			} else {
			if(g_stats[id]>val) {g_stats[id]=val;}
		}

	}

	g_samples[id]++;



	//  	char szBuf[128];
	//  	sprintf_P(szBuf,PSTR("Update stat %d -> %s\r\n"),id,p);
	//  	debug_string(NORMAL,szBuf,RAM_STRING);

}

static void process_CO(const char * const p)
{
	SENSIT_UpdateStats(SENSIT_STAT_CO,p);
}

static void process_NO2(const char * const p)
{
	SENSIT_UpdateStats(SENSIT_STAT_NO2,p);
}

static void process_O3(const char * const p)
{
	SENSIT_UpdateStats(SENSIT_STAT_O3,p);
}

static void process_CH4(const char * const p)
{
	SENSIT_UpdateStats(SENSIT_STAT_CH4,p);
}

static void process_NOx(const char * const p)
{
	SENSIT_UpdateStats(SENSIT_STAT_NOx,p);
}

static void process_C6H6(const char * const p)
{
	SENSIT_UpdateStats(SENSIT_STAT_C6H6,p);
}

static void Handler_CO(char * const psz)
{

	//debug_string_P(NORMAL,PSTR("Handler_C6H6 IN\r\n")); //<--------------------
	char * p = psz+22;
	char * q;
	uint8_t v;

	v = SENSITLine_Tokenize(p,&q);
	//char szBuf[256]; //<----------------------------
	//sprintf_P(szBuf,PSTR("SENSITLine_Tokenize output : %d\r\n"),v); //<----------------------------
	//debug_string(NORMAL,szBuf,false); //<----------------------------
	if(v!=0) {process_CO(p);}
}

static void Handler_NO2(char * const psz)
{
	//debug_string_P(NORMAL,PSTR("Handler_C6H6 IN\r\n")); //<--------------------
	char * p = psz+22;
	char * q;
	uint8_t v;

	v = SENSITLine_Tokenize(p,&q);
	//char szBuf[256]; //<----------------------------
	//sprintf_P(szBuf,PSTR("SENSITLine_Tokenize output : %d\r\n"),v); //<----------------------------
	//debug_string(NORMAL,szBuf,false); //<----------------------------
	if(v!=0) {process_NO2(p);}
}

static void Handler_O3(char * const psz)
{

	//debug_string_P(NORMAL,PSTR("Handler_C6H6 IN\r\n")); //<--------------------
	char * p = psz+22;
	char * q;
	uint8_t v;

	v = SENSITLine_Tokenize(p,&q);
	//char szBuf[256]; //<----------------------------
	//sprintf_P(szBuf,PSTR("SENSITLine_Tokenize output : %d\r\n"),v); //<----------------------------
	//debug_string(NORMAL,szBuf,false); //<----------------------------
	if(v!=0) {process_O3(p);}
}

static void Handler_CH4(char * const psz)
{

	//debug_string_P(NORMAL,PSTR("Handler_C6H6 IN\r\n")); //<--------------------
	char * p = psz+22;
	char * q;
	uint8_t v;

	v = SENSITLine_Tokenize(p,&q);
	//char szBuf[256]; //<----------------------------
	//sprintf_P(szBuf,PSTR("SENSITLine_Tokenize output : %d\r\n"),v); //<----------------------------
	//debug_string(NORMAL,szBuf,false); //<----------------------------
	if(v!=0) {process_CH4(p);}
}

static void Handler_NOx(char * const psz)
{

	//debug_string_P(NORMAL,PSTR("Handler_C6H6 IN\r\n")); //<--------------------
	char * p = psz+22;
	char * q;
	uint8_t v;

	v = SENSITLine_Tokenize(p,&q);
	//char szBuf[256]; //<----------------------------
	//sprintf_P(szBuf,PSTR("SENSITLine_Tokenize output : %d\r\n"),v); //<----------------------------
	//debug_string(NORMAL,szBuf,false); //<----------------------------
	if(v!=0) {process_NOx(p);}
}

static void Handler_C6H6(char * const psz)
{
	//debug_string_P(NORMAL,PSTR("Handler_C6H6 IN\r\n")); //<--------------------
	char * p = psz+22;
	char * q;
	uint8_t v;

	v = SENSITLine_Tokenize(p,&q);
	//char szBuf[256]; //<----------------------------
	//sprintf_P(szBuf,PSTR("SENSITLine_Tokenize output : %d\r\n"),v); //<----------------------------
	//debug_string(NORMAL,szBuf,false); //<----------------------------
	if(v!=0) {process_C6H6(p);}
}
