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


#include "Acronet/setup.h"
#include "Acronet/globals.h"
#include <stdio.h>
#include <conf_board.h>
#include <conf_usart_serial.h>
#include "config/conf_usart_serial.h"
#include "Acronet/drivers/SIM/sim900.h"
#include "Acronet/services/config/config.h"
#include "Acronet/Sensors/raingauge/pulse_raingauge.h"
#include "Acronet/drivers/StatusLED/status_led.h"
#include "Acronet/drivers/UART_INT/cbuffer_usart.h"
#include "Acronet/drivers/PowerSwitch/powerswitch.h"
#include "Acronet/services/LOG/LOG.h"
#include "Acronet/services/taskman/taskman.h"

#define UNLOCK_SIM 0
#define MESSAGE_FORMAT 1
#	define PDU  0
#	define TEXT 1
#define MESSAGE_SEND 2

static void little_delay(void)
{
	delay_ms(200);
}

#define LITTLE_DELAY little_delay()

//IMEI
//#define IMEI_CODE_LEN 32
//char g_szIMEI[IMEI_CODE_LEN];


// string in flash
//static const char version_P[] PROGMEM = "v0.1 Caribbean";
//static const char name_P[] PROGMEM = "sim900 driver";

//USART buffer
static USART_data_t sim900_usart_data;
//volatile uint8_t usart_sync = false;


//GPRS AT command & return codes
static const char sz_AT[]		PROGMEM = "AT\r\n";
static const char sz_OK[]		PROGMEM = "OK";
static const char sz_ERROR[]	PROGMEM = "ERROR";

static const char * const tbl_returns[] PROGMEM = {	sz_OK,
													sz_ERROR };
													
#define NUM_OF_RETURNS (sizeof(tbl_returns)/sizeof(char *))


//GPRS AT+CPIN answer strings
static const char szCPIN_SIM_READY[]	PROGMEM = "READY";
static const char szCPIN_SIM_PIN[]		PROGMEM = "SIM PIN";
static const char szCPIN_SIM_PUK[]		PROGMEM = "SIM PUK";
static const char szCPIN_PHSIM_PIN[]	PROGMEM = "PH_SIM PIN";
static const char szCPIN_PHSIM_PUK[]	PROGMEM = "PH_SIM PUK";
static const char szCPIN_SIM_PIN2[]		PROGMEM = "SIM PIN2";
static const char szCPIN_SIM_PUK2[]		PROGMEM = "SIM PUK2";

static const char * const tbl_CPIN_rets[] PROGMEM = {	sz_OK,
														sz_ERROR,
														szCPIN_SIM_READY,
														szCPIN_SIM_PIN,
														szCPIN_SIM_PUK,
														szCPIN_PHSIM_PIN,
														szCPIN_PHSIM_PUK,
														szCPIN_SIM_PIN2,
														szCPIN_SIM_PUK2 };

#define NUM_OF_CPIN_RETURNS (sizeof(tbl_CPIN_rets)/sizeof(char *))

//GPRS AT+CREG answer strings

/////////////////////////////////////////////////////////////////

static int8_t g_gprs_ref = -1;

USART_data_t * SIM900_get_PUSART( void )
{
	return &sim900_usart_data;
}

static void sim900_power_toggle(void)
{
	gpio_set_pin_high(GPRS_SWITCH);
	delay_ms(1200);
	gpio_set_pin_low(GPRS_SWITCH);
	delay_ms(2500);
}


void sim900_power_off(void)
{
	const bool status = hal_sim_get_status();
	if(true==status) {
		debug_string_2P(NORMAL,PSTR("sim900_power_off") ,PSTR("module is ON so power cycling"));
		sim900_power_toggle();
	} else {
		debug_string_2P(NORMAL,PSTR("sim900_power_off") ,PSTR("module is already OFF"));
	}
}
													
void sim900_power_on(void)
{
	const bool status = hal_sim_get_status();
	if(true!=status) {
		debug_string_2P(NORMAL,PSTR("sim900_power_on") ,PSTR("module is OFF so power cycling"));
		sim900_power_toggle();
	} else {
		debug_string_2P(NORMAL,PSTR("sim900_power_on") ,PSTR("module is already ON"));
	}
}
													

static void sim900_put_data(const char * const pBuf,const uint16_t lenBuf,const uint8_t isPGM)
{
//	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"SIM900_PUT_DATA");

	const char * const p = pBuf;

	if(isPGM) {
		for(uint16_t l=0;l<lenBuf;++l) {
			const char c = nvm_flash_read_byte((flash_addr_t) (p+l) );
			while (usart_data_register_is_empty(USART_GPRS) == false) {}
			usart_put(USART_GPRS, c);
		}
	} else {
		for(uint16_t l=0;l<lenBuf;++l) {
			const char c = p[l];
			while (usart_data_register_is_empty(USART_GPRS) == false) {}
			usart_put(USART_GPRS, c);
		}
	}
}



static void sim900_put_string(const char * const sz,const uint8_t isPGM)
{
	
//	debug_string(NORMAL,PSTR("(sim900_put_string) IN\r\n"),true);

	const char * p = sz;


	if(isPGM) {
		while(1) {
			const char c = nvm_flash_read_byte((flash_addr_t)p++);
			if(0==c) break;
			while (usart_data_register_is_empty(USART_GPRS) == false) {}
			usart_put(USART_GPRS, c);
			if(g_log_verbosity>NORMAL) usart_putchar(USART_DEBUG,c);
		}
		} else {
		while(1) {
			const char c = *p++;
			if(0==c) break;
			while (usart_data_register_is_empty(USART_GPRS) == false) {}
			usart_put(USART_GPRS, c);
			if(g_log_verbosity>NORMAL) usart_putchar(USART_DEBUG,c);
		}
	}

//	debug_string(NORMAL,PSTR("(sim900_put_string) OUT\r\n"),true);
		
}

static uint8_t sim900_wait_data_on_usart(uint8_t seconds)
{
	const uint32_t dt1 = hal_rtc_get_time();

	uint16_t us=10000;

	while(1) {
		
#ifdef SIM900_USART_POLLED
		if(usart_rx_is_complete(USART_GPRS)) {
#else
		if(USART_RX_CBuffer_Data_Available(&sim900_usart_data)) {
#endif
			return 0xFF;
		}

		if(TM_DoYield()) {
			continue;
		}

		delay_us(100);

		if(--us!=0) {
			continue;
		}

		us=10000;
		const uint32_t dt2 = hal_rtc_get_time();
		if ( (dt2-dt1) > seconds )
		{
			//debug_string(VERY_VERBOSE,PSTR("(sim900_wait_data_on_usart) timed-out\r\n"),PGM_STRING);
			return 0;
		}
	}
	
	return 0;
}


//static uint8_t sim900_wait_data_on_usart(uint8_t seconds)
//{
//
	//for(uint8_t d1=seconds;d1>0;--d1)
	//{
		//for(uint16_t d2=10000;d2>0;--d2)
		//{
//#ifdef SIM900_USART_POLLED
			//if(usart_rx_is_complete(USART_GPRS)) {
//#else
			//if(USART_RX_CBuffer_Data_Available(&sim900_usart_data)) {
//#endif				
				//return 0xFF;
			//}
			//delay_us(100);
		//}
	//}
	//
	//debug_string(VERY_VERBOSE,PSTR("(sim900_wait_data_on_usart) timed-out\r\n"),PGM_STRING);
	//return 0;
//}

static RET_ERROR_CODE sim900_read_data(uint8_t * const szBuf,uint16_t * const lenBuf)
{
	const uint16_t ec = *lenBuf;

	uint16_t l = 0;
	RET_ERROR_CODE r = AC_ERROR_OK;



#ifndef SIM900_USART_POLLED
	//usart_buffer_flush(&sim900_usart_data);
#endif

	while(l<ec) {
		
		if(!sim900_wait_data_on_usart(20)) {
			debug_string_2P(NORMAL,PSTR("sim900_read_data") ,PSTR("got timeout waiting for a byte"));
			char sz[128];
			sprintf_P(sz,PSTR("Buffer = %u Index = %u\r\n"),ec,l);
			debug_string(NORMAL,sz,RAM_STRING);
			r=AC_SIM900_TIMEOUT;
			break;
		}
		
#ifdef SIM900_USART_POLLED
		const uint8_t c = usart_get(USART_GPRS);
#else
		const uint8_t c = USART_RX_CBuffer_GetByte(&sim900_usart_data);
#endif

		szBuf[l++] = c;
	}

	*lenBuf = l;
	return r;


}
static RET_ERROR_CODE sim900_read_string_token(char * const szBuf,uint16_t * const lenBuf,const char szToken[], const uint8_t lenTokenlist)
{
	//DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"sim900_read_string_token");

	const uint16_t ec = *lenBuf;

	char c;
	RET_ERROR_CODE r = AC_ERROR_OK;

	uint16_t l = 0;
	while(l<ec) {

		if(!sim900_wait_data_on_usart(20)) {
			debug_string_2P(NORMAL,PSTR("sim900_read_string"),PSTR("got timeout waiting for a character"));
			r=AC_SIM900_TIMEOUT;
			break;
		}
		
		c = USART_RX_CBuffer_GetByte(&sim900_usart_data);

		for(uint8_t j=0;j<lenTokenlist;j++)
		{
			if(c==szToken[j]) {
				szBuf[l++] = 0;
				goto token_exit;
			}
		}
		
		szBuf[l++] = c;

	}

token_exit:

	if(ec==l) {
		r = AC_BUFFER_TOO_SMALL;
		debug_string_2P(NORMAL,PSTR("sim900_read_string"),PSTR("Provided buffer is not big enough. Discarding chars"));
		l -= 1;
	}
	
	*lenBuf = l;
	return r;
}
/*
static RET_ERROR_CODE sim900_read_string(char * const szBuf,uint16_t * const lenBuf)
{

	RET_ERROR_CODE r = AC_ERROR_OK;
	char c;

#ifndef SIM900_USART_POLLED
	usart_buffer_flush(&sim900_usart_data);
#endif

	while(1) {
		
		if(!sim900_wait_data_on_usart(20)) {
			debug_string_2P(NORMAL,PSTR("sim900_read_string"),PSTR("got timeout waiting for a character"));
			r=AC_SIM900_TIMEOUT;
			break;
		}
		
#ifdef SIM900_USART_POLLED
		c = usart_get(USART_GPRS);
#else
		c = USART_RX_CBuffer_GetByte(&sim900_usart_data);
#endif

		if(c=='\r') {
			if(g_log_verbosity > NORMAL) usart_putchar(USART_DEBUG,'@');
		} else	if(c=='\n') {
			if(g_log_verbosity > NORMAL) usart_putchar(USART_DEBUG,'#');
		} else break;
	}

	if(r==AC_ERROR_OK) {
		szBuf[0] = c;
		(*lenBuf)++;
		const char szToken[4] = { '\r','\n', 0, 0 };
		r = sim900_read_string_token(&szBuf[1],lenBuf,szToken,2);
	}

	szBuf[*lenBuf] = 0;

	//debug_string(VERBOSE,szBuf,RAM_STRING);
	//debug_string(VERBOSE,szCRLF,PGM_STRING);

	//debug_string(VERBOSE,PSTR("(sim900_read_string) out\r\n"),true);

	return r;
}
*/

static RET_ERROR_CODE sim900_read_string(char * const szBuf,uint16_t * const lenBuf)
{
	const uint16_t ec = *lenBuf;

	uint16_t l = 0;
	RET_ERROR_CODE r = AC_ERROR_OK;
	char c;

#ifndef SIM900_USART_POLLED
	usart_buffer_flush(&sim900_usart_data);
#endif

	while(1) {
		
		if(!sim900_wait_data_on_usart(20)) {
			debug_string_2P(NORMAL,PSTR("sim900_read_string"),PSTR("got timeout waiting for a character"));
			r=AC_SIM900_TIMEOUT;
			break;
		}
		
#ifdef SIM900_USART_POLLED
		c = usart_get(USART_GPRS);
#else
		c = USART_RX_CBuffer_GetByte(&sim900_usart_data);
#endif

		if(c=='\r') {
			if(g_log_verbosity > NORMAL) usart_putchar(USART_DEBUG,'@');
		} else	if(c=='\n') {
			if(g_log_verbosity > NORMAL) usart_putchar(USART_DEBUG,'#');
		} else break;
	}

	if(r==AC_ERROR_OK)  {
		szBuf[l++] = c;

		while(l<ec) {
		

			if(!sim900_wait_data_on_usart(20)) {
				debug_string_2P(NORMAL,PSTR("sim900_read_string"),PSTR("got timeout waiting for a character"));
				r=AC_SIM900_TIMEOUT;
				break;
			}
		
	#ifdef SIM900_USART_POLLED
			c = usart_get(USART_GPRS);
	#else
			c = USART_RX_CBuffer_GetByte(&sim900_usart_data);
	#endif

			if(c=='\r') {
				//if(g_log_verbosity > NORMAL) usart_putchar(USART_DEBUG,'@');
				//break;
				continue;
			} else	if(c=='\n') {
				//if(g_log_verbosity > NORMAL) usart_putchar(USART_DEBUG,'#');
				break;
			}
			//if(g_log_verbosity > NORMAL) usart_putchar(USART_DEBUG,'.');
			szBuf[l++] = c;
		}
	}

	if(ec==l) {
		r = AC_BUFFER_TOO_SMALL;
		debug_string_2P(NORMAL,PSTR("sim900_read_string"),PSTR("Provided buffer is not big enough. Discarding chars"));
		l -= 1;
	}

	szBuf[l] = 0;

	//debug_string(VERBOSE,szBuf,RAM_STRING);
	//debug_string(VERBOSE,szCRLF,PGM_STRING);

	//debug_string(VERBOSE,PSTR("(sim900_read_string) out\r\n"),true);

	*lenBuf = l;
	return r;
}

static const char * sim900_wait4dictionary(const char * const dictionary[],uint8_t len)
{
	
	uint8_t num = len;
	const char * p[len];

#ifndef SIM900_USART_POLLED
	usart_buffer_flush(&sim900_usart_data);
#endif


	while(num--) {
		p[num] = nvm_flash_read_word( (flash_addr_t) (dictionary+num) );
	}

	while(1) {
		
		
		if(!sim900_wait_data_on_usart(20)) {
			debug_string_2P(NORMAL,PSTR("sim900_wait4dictionary") ,PSTR("timeout waiting for sim900 response"));
			return NULL;
		}
		
#ifdef SIM900_USART_POLLED
		const char c1 = usart_get(USART_GPRS);
#else
		const char c1=USART_RX_CBuffer_GetByte(&sim900_usart_data);
#endif

		for(uint8_t i=0;i<len;++i) {

			const char c2 = nvm_flash_read_byte( (flash_addr_t) p[i]);
			if(c1!=c2)
			{
				p[i]=(char *) nvm_flash_read_word( (flash_addr_t) (dictionary+i) );
				continue;
			}

			p[i]++;

			if( nvm_flash_read_byte( (flash_addr_t) p[i] ) == 0 )
			{
				const char * const r = (char *) nvm_flash_read_word( (flash_addr_t) (dictionary+i) );
				//debug_string(VERY_VERBOSE,PSTR("(sim900_wait4dictionary) got: "),true);
				//debug_string(VERY_VERBOSE,r,true);
				//debug_string(VERY_VERBOSE,szCRLF,true);
				return r;
			}
		}
	}
}

static const char * sim900_wait_string_P(const char * const sz,uint16_t lenSZ)
{

	uint16_t i=0;
	char c2 = 0;

	while( (i<lenSZ) && (c2=nvm_flash_read_byte( (flash_addr_t) (sz + i++) )) ) {

		if(!sim900_wait_data_on_usart(20)) {
			debug_string_2P(NORMAL,PSTR("sim900_wait_string_p") ,PSTR("timeout waiting for sim900 response"));
			return NULL;
		}

#ifdef SIM900_USART_POLLED
		const char c1=usart_get(USART_GPRS);
#else
		const char c1=USART_RX_CBuffer_GetByte(&sim900_usart_data);
#endif
		//if the sequence is not the same we start over again
		if(c1!=c2) {
			i = 0;
		}
	}
	return sz;
	
}


static const char * sim900_wait_string(const char * const sz, const uint16_t lenSZ,const uint8_t isPGM)
{
	
#ifndef SIM900_USART_POLLED
	usart_buffer_flush(&sim900_usart_data);
#endif

	if (isPGM)
	{
		return sim900_wait_string_P(sz,lenSZ);
	}
	
	uint16_t i=0;
	char c2 = 0;

	while( (i<lenSZ) && (c2=sz[i++]) ) {

		if(!sim900_wait_data_on_usart(20)) {
			debug_string_2P(NORMAL,PSTR("sim900_wait_string") ,PSTR("timeout waiting for sim900 response"));
			return NULL;
		}

#ifdef SIM900_USART_POLLED
		const char c1=usart_get(USART_GPRS);
#else
		const char c1=USART_RX_CBuffer_GetByte(&sim900_usart_data);
#endif
		//if the sequence is not the same we start over again
		if(c1!=c2) {
			i=0;
		}
	}
	return sz;

}



static const char * sim900_wait_retstring(void)
{
	return sim900_wait4dictionary(tbl_returns,NUM_OF_RETURNS);
}



static RET_ERROR_CODE sim900_cmd_with_read_string(const char * const szCMD,const uint8_t isCMDPGM,char * const szBuf, uint16_t * lenBuf)
{
	RET_ERROR_CODE r = AC_ERROR_OK;
	
	for(uint8_t i=0;i<2;++i) {
		sim900_put_string(szCMD,isCMDPGM);

		uint16_t l = *lenBuf;
		r = sim900_read_string(szBuf,&l);
		if ( (AC_ERROR_OK==r) || (AC_BUFFER_TOO_SMALL==r) )
		{
			*lenBuf = l;
			return r;
		}
		debug_string_2P(NORMAL,PSTR("sim900_cmd_with_read_string") ,PSTR("got timeout, retry"));
	}
	debug_string_2P(NORMAL,PSTR("sim900_cmd_with_read_string") ,PSTR("failed"));
	return r;
	
}

static const char * sim900_cmd_with_retstring(const char * const szCMD,const uint8_t isPGM)
{
	
	const char * szRet;
	for(uint8_t i=0;i<2;++i) {

		sim900_put_string(szCMD,isPGM);
		szRet=sim900_wait_retstring();

		if(NULL==szRet) {
			continue;
		} else {
			break;
		}

	}
	return szRet;
}


static RET_ERROR_CODE sim900_CMD_wait_URC(const char * const szCMD, const char * const szURC,char * const return_value, uint16_t * const rv_len)
{
//	uint16_t len_szBuf = 64;

	const uint16_t lenURC = strlen_P(szURC);

	LITTLE_DELAY;
	sim900_put_string(szCMD,PGM_STRING);


	if(NULL == sim900_wait_string(szURC, lenURC,PGM_STRING))
	{
		return AC_SIM900_COMM_ERROR;
	}

	RET_ERROR_CODE r = AC_ERROR_OK;

	//Reads the parameters of the URC message (e.g +HTTPREAD:<dimension of the buffer>)
	r = sim900_read_string(return_value,rv_len);
	
	if(r==AC_SIM900_TIMEOUT) {
		debug_string_2P(NORMAL,PSTR("sim900_CMD_wait_URC"),PSTR("#1 got timeout waiting for sim900 response"));
		return AC_SIM900_TIMEOUT;
	}
	
	if(r==AC_BUFFER_OVERFLOW) {
		debug_string_2P(NORMAL,PSTR("sim900_CMD_wait_URC"),PSTR("#1 provided buffer is not big enough"));
		return AC_BUFFER_OVERFLOW;
	}
	
	return r;
	
}

static RET_ERROR_CODE sim900_read_stream(char * const return_value, uint16_t * const rv_len, bool isBinary)
{
	RET_ERROR_CODE	r ;
	
	if(isBinary) {
		r = sim900_read_data(return_value,rv_len);
	} else {
		r = sim900_read_string(return_value,rv_len);
	}
	

	if(r==AC_SIM900_TIMEOUT) {
		debug_string_2P(NORMAL,PSTR("sim900_read_data"),PSTR("#2 got timeout waiting for sim900 response"));
		return AC_SIM900_TIMEOUT;
	}
	
	if(r==AC_BUFFER_OVERFLOW) {
		debug_string_2P(NORMAL,PSTR("sim900_read_data"),PSTR("#2 provided buffer is not big enough"));
		return AC_BUFFER_OVERFLOW;
	}

	return r;
}


//static uint8_t sim900_check_status(void)
//{
	//return 0;
//}



RET_ERROR_CODE sim900_GPRS_check_line( void )
{
	LITTLE_DELAY;
	if(NULL==sim900_cmd_with_retstring(sz_AT,PGM_STRING))
	{
		debug_string_2P(NORMAL,PSTR("sim900_GPRS_check_line") ,PSTR("SIM900 not responding"));
		return AC_SIM900_TIMEOUT;
	}

	LITTLE_DELAY;
	sim900_put_string(PSTR("AT+CPIN?\r\n"),PGM_STRING);
	const char * szRET = sim900_wait4dictionary(tbl_CPIN_rets,NUM_OF_CPIN_RETURNS);
	
	if(szRET!=szCPIN_SIM_READY ) {
		debug_string_2P(NORMAL,PSTR("sim900_GPRS_check_line") ,PSTR("SIM900 SIM IS PIN LOCKED"));
		return AC_SIM900_SIM_LOCKED;
	}
	
	char szBuf[32];

	uint16_t l = sizeof(szBuf);
	memset(szBuf,0,sizeof(szBuf));

	LITTLE_DELAY;
	sim900_put_string(PSTR("AT+CREG?\r\n"),PGM_STRING);

	sim900_read_string(szBuf,&l);
	if(strncasecmp_P(szBuf,PSTR("+CREG:"),5)!=0) {
		debug_string_2P(NORMAL,PSTR("sim900_GPRS_check_line") ,PSTR("device is not answering as it should at CREG command"));
		return AC_SIM900_COMM_ERROR;
	}
	if(szBuf[9]!='1') {
		debug_string_2P(NORMAL,PSTR("sim900_GPRS_check_line") ,PSTR("device not (yet) registered on the network"));
		return AC_SIM900_LINE_NOT_REGISTERED;
	}

	LITTLE_DELAY;
	if(NULL==sim900_cmd_with_retstring(sz_AT,PGM_STRING))
	{
		debug_string_2P(NORMAL,PSTR("sim900_GPRS_check_line") ,PSTR("SIM900 not responding"));
		return AC_SIM900_TIMEOUT;
	}


	l = sizeof(szBuf);
	memset(szBuf,0,sizeof(szBuf));

	LITTLE_DELAY;
	sim900_put_string(PSTR("AT+CGATT?\r\n"),PGM_STRING);

	sim900_read_string(szBuf,&l);
	if(strcasecmp_P(szBuf,PSTR("+CGATT: 1"))!=0) {
		debug_string_2P(NORMAL,PSTR("sim900_GPRS_check_line") ,PSTR("GPRS line not (yet) attached"));
		return AC_SIM900_GPRS_NOT_ATTACHED;
	}


	LITTLE_DELAY;
	if(NULL==sim900_cmd_with_retstring(sz_AT,PGM_STRING))
	{
		debug_string_2P(NORMAL,PSTR("sim900_GPRS_check_line") ,PSTR("SIM900 not responding"));
		return AC_SIM900_TIMEOUT;
	}

	l = sizeof(szBuf);
	memset(szBuf,0,sizeof(szBuf));

	LITTLE_DELAY;
	sim900_put_string(PSTR("AT+CSQ\r\n"),PGM_STRING);

	sim900_read_string(szBuf,&l);
	if(strncasecmp_P(szBuf,PSTR("+CSQ:"),4)!=0) {
		debug_string_2P(NORMAL,PSTR("sim900_GPRS_check_line") ,PSTR("device is not answering as it should at CSQ command"));
		return AC_SIM900_COMM_ERROR;
	}
	
	uint8_t i=4;
	uint8_t lq = 255;
	for(;i<l;++i ) {
		if(szBuf[i]==',') {
			szBuf[i]=0;
			lq = atoi(szBuf+5);
			break;
		}
	}

	sprintf_P(szBuf,PSTR("CSQ=%d"),lq);
	LOG_say(szBuf);
	//debug_string_P(NORMAL,PSTR("(sim900_GPRS_check_line) CSQ = "));
	//debug_string(NORMAL,szBuf,RAM_STRING);
	//debug_string_P(NORMAL,g_szCRLF);


	if(lq==99) {
		debug_string_2P(NORMAL,PSTR("sim900_GPRS_check_line") ,PSTR("[WARNING] line quality is undetectable"));
	} else if(lq<10) {
		debug_string_2P(NORMAL,PSTR("sim900_GPRS_check_line") ,PSTR("[WARNING] line quality is less than -93db"));
	}

	return AC_ERROR_OK;
}


RET_ERROR_CODE sim900_init( void )
{
	//DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"SIM900_INIT");
	static const __flash char funName[] = "SIM900_INIT";
	char szBuf[64];

	g_gprs_ref=0;
	
#ifdef SIM900_USART_POLLED
	
	debug_string_2P(NORMAL,funName,PSTR("code compiled with polled usart"));
#else
	usart_interruptdriver_initialize(&sim900_usart_data,USART_GPRS,USART_INT_LVL_LO);
	usart_set_rx_interrupt_level(USART_GPRS,USART_INT_LVL_LO);
	usart_set_tx_interrupt_level(USART_GPRS,USART_INT_LVL_OFF);

	debug_string_2P(NORMAL,funName,PSTR("code compiled with interrupt usart"));

#endif

	usart_rx_enable(USART_GPRS);

	sim900_power_off();
	
	const char * szRET = NULL;
	int t=2;
	while(t--) {
		sim900_power_toggle();
		uint8_t i=3;
		while(i--) {
			statusled_blink(1);
			sim900_put_string(sz_AT,PGM_STRING);
			szRET = sim900_wait_retstring();
			if(szRET!=sz_OK) {
				debug_string_2P(NORMAL,funName,PSTR("trying to set a serial line speed"));
			} else break;
		}
		if(szRET==sz_OK) { //If we get OK from the SIM900 we exit from this loop
			break;
		}
		//If we are still in this loop we try to toggle the SIM900 power signal again
		debug_string_2P(NORMAL,funName,PSTR("Not being able to connect to SIM900. Try to power it on again"));
	}

	//If the power toggles for too many times without having got the OK we should try to handle a severe HW problem
	if (szRET!=sz_OK)
	{
		debug_string_2P(NORMAL,funName,PSTR("failed to synchronize with GPRS UART. The process will end here"));
		powerSwitch_toggle();
		return AC_SIM900_COMM_ERROR;
	}


	//disabling echo back from the modem
	LITTLE_DELAY;
	szRET = sim900_cmd_with_retstring(PSTR("ATE0\r\n"),PGM_STRING);
	//sim900_wait_retstring();
	
	if(szRET!=sz_OK) {
		debug_string_2P(NORMAL,funName,PSTR("this sim900 doesn't support ATE0"));
	} else {
		//debug_string_2P(VERBOSE,funName,PSTR("correctly issued ATE0"));
	}
	

	LITTLE_DELAY;
	szRET = sim900_cmd_with_retstring(PSTR("AT+CREG=0\r\n"),PGM_STRING);

	if(szRET!=sz_OK) {
		debug_string_2P(NORMAL,funName,PSTR("this sim900 doesn't support AT+CREG"));
	} else {
		//debug_string_2P(VERBOSE,funName,PSTR("correctly issued AT+CREG=0"));
	}

	
	//get the IMEI code
	LITTLE_DELAY;
	szRET = sim900_cmd_with_retstring(PSTR("AT+GSN=?\r\n"),PGM_STRING);
	
	if(szRET!=sz_OK) {
		debug_string_2P(NORMAL,funName,PSTR("this sim900 doesn't support AT+GSN"));
	}

/*
	{
		uint16_t i=sizeof(szBuf)-10;
		memcpy_P(szBuf,PSTR("SIM MODEL="),10);
		LITTLE_DELAY;
		sim900_cmd_with_read_string(PSTR("AT+CGMM\r\n"),PGM_STRING,szBuf+10,&i);
		sim900_wait_retstring();

		debug_string(NORMAL,szBuf,RAM_STRING);
		debug_string(NORMAL,g_szCRLF,PGM_STRING);
	}
	
	{
		uint16_t i=sizeof(szBuf)-7;
		memcpy_P(szBuf,PSTR("FW REV="),7);
		LITTLE_DELAY;
		sim900_cmd_with_read_string(PSTR("AT+CGMR\r\n"),PGM_STRING,szBuf+7,&i);
		sim900_wait_retstring();

		debug_string(NORMAL,szBuf,RAM_STRING);
		debug_string(NORMAL,g_szCRLF,PGM_STRING);
	}
*/
	uint16_t i=sizeof(szBuf)-5;
	memcpy_P(szBuf,PSTR("IMEI="),5);
	LITTLE_DELAY;
	sim900_cmd_with_read_string(PSTR("AT+GSN\r\n"),PGM_STRING,szBuf+5,&i);
	sim900_wait_retstring();

	LOG_say(szBuf);

	szBuf[i+5] = '\r';
	szBuf[i+6] = '\n';
	szBuf[i+7] = 0;
	debug_string(NORMAL,szBuf,RAM_STRING);

	if(szRET!=sz_OK) {
		debug_string_2P(NORMAL,funName,PSTR("AT+GSN messed up"));
	} else {
		debug_string_2P(NORMAL,funName,PSTR("AT+GSN GOT OK"));
	}

	LITTLE_DELAY;
	sim900_put_string(PSTR("AT+CPIN=?\r\n"),PGM_STRING);
	szRET = sim900_wait_retstring();
	
	if(szRET!=sz_OK) {
		debug_string_2P(NORMAL,funName,PSTR("SIM is not present"));
		return AC_SIM900_SIM_NOT_PRESENT;
	}
	

	LITTLE_DELAY;
	sim900_put_string(PSTR("AT+CPIN?\r\n"),PGM_STRING);
	szRET = sim900_wait4dictionary(tbl_CPIN_rets,NUM_OF_CPIN_RETURNS);

	
	if(szRET==szCPIN_SIM_PIN) {
		debug_string_2P(NORMAL,funName,PSTR("SIM present and is PIN locked"));

		CFG_ITEM_ADDRESS f;
		if( AC_ERROR_OK != cfg_find_item(CFG_TAG_SIM_PIN_CODE,&f))
		{
			debug_string_2P(NORMAL,funName,PSTR("[ERROR] Missing configuration file"));
			return AC_ERROR_GENERIC;
		}
		szBuf[0] = 'P';
		szBuf[1] = 'I';
		szBuf[2] = 'N';
		szBuf[3] = ':';
		cfg_get_item_file(f,szBuf+4,8);

		if((szBuf[4]==0xFF) || (szBuf[4]==0x00)) {		
			debug_string_2P(NORMAL,funName,PSTR("PIN code is not present in memory SIM will remain locked"));
			return AC_SIM900_SIM_PIN_WRONG;
		} else {
			debug_string_2P(NORMAL,funName,PSTR("PIN code is present in memory trying to unlock"));
			debug_string(NORMAL,szBuf,RAM_STRING);
			debug_string_1P(NORMAL,g_szCRLF);
			LITTLE_DELAY;
			sim900_put_string(PSTR("AT+CPIN="),PGM_STRING);
			sim900_put_string(szBuf+4,RAM_STRING);
			sim900_put_string(g_szCRLF,PGM_STRING);
			const char * ret = sim900_wait_retstring();
			if(ret!=sz_OK) {
				debug_string_2P(NORMAL,funName,PSTR("[WARNING] PIN IS WRONG"));
				return AC_SIM900_SIM_PIN_WRONG;
			}
		}
	} else if (szRET==szCPIN_SIM_PUK)
	{
		debug_string_2P(NORMAL,funName,PSTR("SIM present and is PUK locked"));
		debug_string_2P(NORMAL,funName,PSTR("no PUK code, SIM init will fail here"));
		return AC_SIM900_SIM_PUK_LOCK;
	}
	
	LITTLE_DELAY;
	sim900_put_string(sz_AT,PGM_STRING);
	szRET = sim900_wait_retstring();
	if(szRET!=sz_OK) {
		debug_string_2P(NORMAL,funName,PSTR("UNABLE TO GET OK after AT\r\n"));
		return AC_SIM900_COMM_ERROR;
	} 

	LITTLE_DELAY;
	i=sizeof(szBuf)-5;
	memcpy_P(szBuf,PSTR("CCID="),5);
	sim900_cmd_with_read_string(PSTR("AT+CCID\r\n"),PGM_STRING,szBuf+5,&i);
	szRET = sim900_wait_retstring();

	LOG_say(szBuf);

	szBuf[i] = '\r';
	szBuf[i+1] = '\n';
	szBuf[i+2] = 0;

	//debug_string_P(NORMAL,PSTR("CCID code is : "));
	debug_string(NORMAL,szBuf,RAM_STRING);
	//debug_string_P(NORMAL,g_szCRLF);

	if(szRET!=sz_OK) {
		debug_string_2P(NORMAL,funName,PSTR("AT+CCID messed up"));
	} else {
		debug_string_2P(NORMAL,funName,PSTR("AT+CCID GOT OK"));
	}

	LITTLE_DELAY;
	sim900_put_string(PSTR("AT+CIPMUX=1\r\n"),PGM_STRING);
	szRET = sim900_wait_retstring();
	if(szRET!=sz_OK) {
		debug_string_2P(NORMAL,funName,PSTR("UNABLE TO GET OK after AT+CIPMUX=1\r\n"));
		return AC_SIM900_COMM_ERROR;
	}

	return AC_ERROR_OK;
}



uint8_t sim900_send_sms(char * const p_sms_buf,const uint8_t isBufPGM, char * const p_sms_recipient,const uint8_t isRecPGM)
{

	//DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"SIM900_SEND_SMS");

	usart_rx_enable(USART_GPRS);


	const char * szret = sim900_cmd_with_retstring(PSTR("AT+CMGF=1\r\n"),true);
	if(szret!=sz_OK) {
		debug_string_2P(NORMAL,PSTR ("sim900_send_sms") ,PSTR ("[ERROR] got an error from AT+CMGF=1\r\n(sim900_send_sms) [ERROR] Instead of OK got : "));
		debug_string_1P(NORMAL,(NULL==szret) ? PSTR("NULL"):szret); //szret points to a flash storage string
		debug_string_1P(NORMAL,g_szCRLF);
		return -1;
	}		
	
	LITTLE_DELAY;
	sim900_put_string(PSTR("AT+CMGS=\""),PGM_STRING);
	sim900_put_string(p_sms_recipient,isRecPGM);
	sim900_put_string(PSTR("\"\r\n"),PGM_STRING);
	
	if(!sim900_wait_string(PSTR(">"), 1,PGM_STRING)) {
		debug_string_2P(NORMAL,PSTR ("sim900_send_sms") ,PSTR ("[ERROR] got an error from AT+CMGS\r\n"));
		return -1;
	}
	

	LITTLE_DELAY;
	sim900_put_string(p_sms_buf,isBufPGM);
	sim900_put_string(PSTR("\x1A"),PGM_STRING);

	return 0;
}


uint8_t sim900_send_sms_to_phone_book(const char * sms_book[],const uint8_t isBOOKPGM,char * const msg,const uint8_t isMSGPGM)
{
	const size_t n = sizeof(sms_book)/sizeof(char *);
	uint8_t e = 0;
	for(size_t i=0;i<n;++i) {
		if(sim900_send_sms(msg,isMSGPGM,sms_book[i],isBOOKPGM)!=0) e=-1;
	}
	
	return e;
}

RET_ERROR_CODE sim900_bearer_simple_open( void )
{

	const RET_ERROR_CODE e = sim900_bearer_open();

	if(AC_ERROR_OK==e) {
		if(g_gprs_ref>0) {
			g_gprs_ref++;
		} else {
			g_gprs_ref=1;
		}
	}

	return e;
}

RET_ERROR_CODE sim900_bearer_simple_close(void)
{
//	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"SIM900_GPRS_SIMPLE_CLOSE");

	if(g_gprs_ref==0) {
		g_gprs_ref = -1;
		return sim900_bearer_close();
	} 

	return AC_ERROR_OK;
}

RET_ERROR_CODE sim900_bearer_simple_release(void)
{
//	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"sim900_bearer_simple_release");
	if(g_gprs_ref>0) {
		g_gprs_ref--;
	}

	return AC_ERROR_OK;
}



RET_ERROR_CODE sim900_get_APN_by_operator( char * const szAPN , uint16_t szAPNLen )
{
	
	char szBuf[512];
	uint16_t len = 48;//sizeof(szBuf);

	static const __flash char funName[] = "sim900_get_APN_by_operator";

	///////////////////////////////////////////////////////////////////////////////

	//CHECK THE CODE OF THE OPERATOR

	LITTLE_DELAY;
	if(sz_OK!=sim900_cmd_with_retstring(PSTR("AT+CENG=1\r\n"),PGM_STRING))
	{
		debug_string_2P(NORMAL,funName,PSTR("the AT+CENG=1 query went wrong, procedure aborted"));
		return AC_SIM900_COMM_ERROR;
	}

	LITTLE_DELAY;


	RET_ERROR_CODE err = sim900_CMD_wait_URC(PSTR("AT+CENG?\r\n"),PSTR("+CENG:"), szBuf,&len);

	if (err!=AC_ERROR_OK)
	{
		return err;
	}

	len = 48;
	sim900_read_stream(szBuf,&len,true);

	uint16_t iy=0;
	uint16_t ix=0;
	uint8_t coms = 4;

	while(ix<len-1)
	{
		const char c = szBuf[ix++]; 
		if (c==',')
		{
			coms--;
		}
	
		if(coms==0) {
			break;
		}
	}

	while(ix<len-1)
	{
		const char c = szBuf[ix++];
		if (c==',')
		{
			szBuf[iy++] = '_';
			break;
		}

		szBuf[iy++] = c;
	}

	while(ix<len-1)
	{
		const char c = szBuf[ix++]; 
		if (c==',')
		{
			break;
		}
		szBuf[iy++] = c;
	}

	szBuf[iy] = 0;
	
	///////////////////////////////////////////////////////////////////////////////

	debug_string_2P(NORMAL,funName,PSTR("OPERATOR is : "));
	debug_string(  NORMAL,szBuf,RAM_STRING);
	debug_string_1P(NORMAL,g_szCRLF);
	
	CFG_ITEM_ADDRESS fd;
	
	cfg_find_item(CFG_TAG_SIM_APN_LIST_ADDRESS,&fd);

	if(AC_ERROR_OK != cfg_get_item_dictionary(fd,szBuf,szAPN,szAPNLen))
	{
		szAPN[0] = 0;
		strcat_P(szAPN,PSTR("internet"));
	}
	//cfg_new_get_file(fd,szAPN,sizeof(szAPN));
	//	cfg_get_gprs_apn(szBuf,szAPN,sizeof(szAPN));

	//debug_string_1P(VERBOSE,PSTR("Using APN "));
	//debug_string(VERBOSE,szAPN,RAM_STRING);
	//debug_string_1P(VERBOSE,g_szCRLF);
	
	return AC_ERROR_OK;
	
}

RET_ERROR_CODE sim900_bearer_open( void )
{

	const char * szRet = NULL;
	char szBuf[128];
	uint16_t len = sizeof(szBuf);
	uint8_t i;

	//DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"SIM900_BEARER_INIT");

	static const __flash char funName[] = "SIM900_BEARER_INIT";


	//CHECK THE STATUS OF THE BEARER
	LITTLE_DELAY;
	RET_ERROR_CODE err = sim900_cmd_with_read_string(PSTR("AT+SAPBR=2,1\r\n"),PGM_STRING,szBuf,&len);
	if(AC_ERROR_OK!=err)
	{
		debug_string_2P(NORMAL,funName,PSTR("the SAPBR #1 query went wrong, procedure aborted"));
		return err;
	}
	
	szRet = sim900_wait_retstring();


	//debug_string_1P(VERBOSE,PSTR("Bearer is : "));
	//debug_string(  VERBOSE,szBuf,RAM_STRING);
	//debug_string_1P(VERBOSE,g_szCRLF);

	
	//check if the OK was at the end of the answer and
	//the status of the bearer
	
	if(sz_OK!=szRet) { //Something went wrong with the sapbr query, we exit from the init function
		debug_string_2P(NORMAL,funName,PSTR("the SAPBR #2 query went wrong, procedure aborted"));
		return AC_SIM900_COMM_ERROR;
	}
	

	//check if the bearer is not closed 
	if (strncasecmp_P(szBuf,PSTR("+SAPBR: 1,3,\"0.0.0.0\""),11)!=0)
	{

		const char r = szBuf[10];
		if(r=='2') { //this bearer is closing
			debug_string_2P(NORMAL,funName,PSTR("(WARNING) bearer is in closing status"));
			return AC_SIM900_RESOURCE_UNAVAILABLE;
		} else {
			debug_string_2P(NORMAL,funName,PSTR("(WARNING) bearer is already open"));
			return AC_ERROR_OK;
		}
	}

	//configure the bearer to be used
	LITTLE_DELAY;
	if(NULL==sim900_cmd_with_retstring(PSTR("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n"),true))
	{
		//Something went wrong with the sapbr query, we exit from the init function
		debug_string_2P(NORMAL,funName,PSTR("the SAPBR=3,1,\"Contype\",\"GPRS\""  \
											  " query went wrong, procedure aborted"));
		return AC_SIM900_COMM_ERROR;
		
	}
	

	sim900_get_APN_by_operator(szBuf,sizeof(szBuf));
	
	char * pStr[3];
	pStr[0]=szBuf;
	uint8_t cp=0;

	for(i=1;i<sizeof(szBuf)-1;i++)
	{
		const char c = szBuf[i];
		if(c==0) break;
		if(c==',') {
			pStr[++cp] = szBuf + i;
			if(cp == 2) break;
		}
	}


	static const __flash char szAPN[] = "APN";
	static const __flash char szUsr[] = "USER";
	static const __flash char szPwd[] = "PWD";

	static const __flash char * const idxSz = {szAPN,szUsr,szPwd};

	for(uint8_t j=0;j<cp;++j)
	{
		
		//Set the APN
		for (i=0;i<2;i++)
		{
			LITTLE_DELAY;
			sim900_put_string(PSTR("AT+SAPBR=3,1,\""),PGM_STRING);
			sim900_put_string(idxSz[j],PGM_STRING);
			sim900_put_string(PSTR("\",\""),PGM_STRING);
			sim900_put_string(pStr[j],RAM_STRING);
			sim900_put_string(PSTR("\"\r\n"),PGM_STRING);
			szRet=sim900_wait_retstring();

			if (sz_OK==szRet)
			{
				break;
			}
			else if(NULL==szRet)
			{
				continue;
			}

			//Something went wrong with the sapbr query, we exit from the init function
			debug_string_2P(NORMAL,funName,PSTR("the SAPBR=3,1,\""));
			debug_string_2P(NORMAL,funName,idxSz[j]);
			debug_string_2P(NORMAL,funName,PSTR("\" query went wrong, procedure aborted\r\n"));
			return AC_SIM900_COMM_ERROR;
		}
	}


	//Finally open the bearer
	LITTLE_DELAY;
	szRet = sim900_cmd_with_retstring(PSTR("AT+SAPBR=1,1\r\n"),PGM_STRING);
	if(sz_OK!=szRet)
	{
		//Something went wrong with the sapbr query, we exit from the init function
		debug_string_2P(NORMAL,funName,PSTR("the SAPBR=1,1"  \
					" query went wrong, procedure aborted\r\n"));
		return AC_SIM900_COMM_ERROR;
	}



	//Check if the bearer is correctly open
	for(i=0;i<10;++i) {

		len = sizeof(szBuf);		
		LITTLE_DELAY;
		err = sim900_cmd_with_read_string(PSTR("AT+SAPBR=2,1\r\n"),PGM_STRING,szBuf,&len);
		if(AC_ERROR_OK!=err)
		{
			debug_string_2P(NORMAL,funName,PSTR("the SAPBR query went wrong, procedure aborted\r\n"));
			return err;
		}

		
		szRet = sim900_wait_retstring();

		//debug_string_1P(VERBOSE,PSTR("Bearer is : "));
		//debug_string(VERBOSE,szBuf,RAM_STRING);
		//debug_string_1P(VERBOSE,g_szCRLF);

	
		//check if the OK was at the end of the answer and
		//the status of the bearer
	
		if(sz_OK!=szRet) { //Something went wrong with the sapbr query, we exit from the init function
			debug_string_2P(NORMAL,funName,PSTR("the SAPBR query went wrong, procedure aborted"));
			return AC_SIM900_COMM_ERROR;
		}

		const char r = szBuf[10];
			
		//check bearer status 0=connecting 1=connected 2=closing 3=closed
		if (r=='0')
		{
			debug_string_2P(NORMAL,funName,PSTR("the bearer is in 'connecting' status. Check again in 2 seconds"));
			delay_ms(2000);
			continue;
		} else if (r=='1')
		{
			debug_string_2P(VERBOSE,funName,PSTR("the bearer is open"));
			break;
		} 
		else if((r=='2') || (r=='3'))
		{
			debug_string_2P(NORMAL,funName,PSTR("(ERROR) the bearer is closed or in closing status"));
			return AC_SIM900_RESOURCE_UNAVAILABLE;
		}
	}

	return AC_ERROR_OK;
}

RET_ERROR_CODE sim900_bearer_close(void)
{
	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"SIM900_BEARER_CLOSE");
	LITTLE_DELAY;

	if(sz_OK!=sim900_cmd_with_retstring(PSTR("AT+SAPBR=0,1\r\n"),PGM_STRING))
	{
		return AC_SIM900_COMM_ERROR;
	}

	return AC_ERROR_OK;
}



RET_ERROR_CODE sim900_http_read(char * const return_value, uint16_t * const rv_len, bool isBinary)
{
	char szLen[16];
	int len = sizeof(szLen);
	RET_ERROR_CODE r = sim900_CMD_wait_URC(PSTR("AT+HTTPREAD\r\n"),PSTR("+HTTPREAD:"), szLen, &len);
	if (AC_ERROR_OK!=r)
	{
		return r;
	}
	szLen[sizeof(szLen)-1]=0;
	len = atoi(szLen);
	
	return sim900_read_stream(return_value,rv_len,isBinary);
}

RET_ERROR_CODE sim900_tcp_shutdown( void )
{
	LITTLE_DELAY;


	if(NULL==sim900_cmd_with_retstring(PSTR("AT+CIPSHUT\r\n"),PGM_STRING))
	{
		debug_string_2P(NORMAL,PSTR("SIM900_TCP_SHUTDOWN"),PSTR("SIM900 not responding"));
		return AC_SIM900_TIMEOUT;
	}

	return AC_ERROR_OK;
}

RET_ERROR_CODE sim900_tcp_close( const uint8_t cid )
{

	LITTLE_DELAY;

	char szBuf[32];
	uint16_t lenBuf = sizeof(szBuf);
	
	int v = cid;
	sprintf_P(szBuf,PSTR("AT+CIPCLOSE=%d\r\n"),v);

	RET_ERROR_CODE err = sim900_cmd_with_read_string(szBuf,RAM_STRING,szBuf,&lenBuf);
	if(AC_ERROR_OK!=err)
	{
		debug_string_2P(NORMAL,PSTR("SIM900_TCP_CLOSE"),PSTR("Error closing TCP channel, forcing shutdown"));
		return sim900_tcp_shutdown();
	}

	
	return AC_ERROR_OK;
}

RET_ERROR_CODE sim900_wait_closing_tcp_peer( void )
{
	if(NULL==sim900_wait_string(PSTR("CLOSED"), 6,PGM_STRING))
	{
		return AC_SIM900_TIMEOUT;
	}
		
	return AC_ERROR_OK;
}

//RET_ERROR_CODE sim900_tcp_get_buffer_len( const uint8_t cid, uint16_t * const pLen )
//{
	//LITTLE_DELAY;
	//if(NULL==sim900_cmd_with_retstring(sz_AT,PGM_STRING))
	//{
		//debug_string_2P(NORMAL,PSTR("sim900_tcp_get_buffer_len") ,PSTR("SIM900 not responding"));
		//return AC_SIM900_TIMEOUT;
	//}
	//
	//char szBuf[32] = {0};
	//uint16_t len_szBuf = sizeof(szBuf);
	//
	////g_log_verbosity = VERY_VERBOSE;
	//LITTLE_DELAY;
	//sim900_put_string(PSTR("AT+CIPSEND?\r\n"),PGM_STRING);
//
	//const RET_ERROR_CODE r = sim900_read_string(szBuf,&len_szBuf);
//
	//if(len_szBuf<9) {
		//return AC_SIM900_RESOURCE_UNAVAILABLE;
	//}
//
	//szBuf[sizeof(szBuf)-8] = 0;
//
	//uint8_t i = 0;
	//while( i < (sizeof(szBuf)-8) )
	//{
		//if(szBuf[i++]==',') break;
	//}
//
//
	//*pLen = atoi(szBuf+i);
	//return r;
//
//}

RET_ERROR_CODE sim900_tcp_get_buffer_len( const uint8_t cid, uint16_t * const pLen )
{
	LITTLE_DELAY;
	if(NULL==sim900_cmd_with_retstring(sz_AT,PGM_STRING))
	{
		debug_string_2P(NORMAL,PSTR("sim900_tcp_get_buffer_len") ,PSTR("SIM900 not responding"));
		return AC_SIM900_TIMEOUT;
	}
	
	char szBuf[32] = {0};
	
	//g_log_verbosity = VERY_VERBOSE;
	LITTLE_DELAY;
	sim900_put_string(PSTR("AT+CIPSEND?\r\n"),PGM_STRING);

	uint8_t limit = 6;
	while(limit--) {
		if(NULL==sim900_wait_string(PSTR("+CIPSEND:"),9,PGM_STRING)) {	
			return AC_SIM900_RESOURCE_UNAVAILABLE;	
		}
		const char token[] = {',',0x0D};
		uint16_t len_szBuf = sizeof(szBuf);
		if(AC_ERROR_OK!=sim900_read_string_token(szBuf,&len_szBuf,token,1))	{
			return AC_SIM900_RESOURCE_UNAVAILABLE;	
		}
		const uint8_t ch = atoi(szBuf);
		len_szBuf = sizeof(szBuf);
		if(AC_ERROR_OK!=sim900_read_string_token(szBuf,&len_szBuf,token+1,1)) {
			return AC_SIM900_RESOURCE_UNAVAILABLE;	
		}
		if(ch==cid) {
			break;
		}
	}
	

	if ( (sz_OK!=sim900_wait_retstring()) || (limit==0) )
	{
		return AC_SIM900_RESOURCE_UNAVAILABLE;
	}
	
	*pLen = atoi(szBuf);
	return AC_ERROR_OK;

}


static RET_ERROR_CODE sim900_tcp_init_2( const uint8_t cid, const char * const service_APN, const uint8_t isSZPGM )
{
	
	LITTLE_DELAY;
	if(NULL==sim900_cmd_with_retstring(PSTR("AT+CIPSTATUS\r\n"),PGM_STRING))
	{
		debug_string_2P(NORMAL,PSTR("(sim900_tcp_init)"),PSTR("SIM900 not responding"));
		return AC_SIM900_TIMEOUT;
	}
	
	char szBuf[32];
	uint16_t len = sizeof(szBuf);
	sim900_read_string(szBuf,&len);
	
	//debug_string(NORMAL,PSTR("IP STATUS : "),PGM_STRING);
	//debug_string(NORMAL,szBuf,RAM_STRING);
	//debug_string(NORMAL,g_szCRLF,PGM_STRING);
	
	RET_ERROR_CODE err = AC_ERROR_OK;

	if (0==strncasecmp_P(szBuf,PSTR("STATE: IP STATUS"),16))
	{
		debug_string_1P(NORMAL,PSTR("STATE: IP STATUS"));
		return AC_ERROR_OK;
	} else 	if (0==strncasecmp_P(szBuf,PSTR("STATE: IP PROCESSING"),16))
	{
		debug_string_1P(NORMAL,PSTR("STATE: IP PROCESSING"));
		return AC_ERROR_OK;
	} else if (0==strncasecmp_P(szBuf,PSTR("STATE: PDP DEACT"),16))
	{
		debug_string_1P(NORMAL,PSTR("STATE: PDP DEACT"));
		if(NULL==sim900_cmd_with_retstring(PSTR("AT+CIPSHUT\r\n"),PGM_STRING))
		{
			debug_string_2P(NORMAL,PSTR("sim900_tcp_init"),PSTR("SIM900 not responding"));
			return AC_SIM900_TIMEOUT;
		}
	} else {
		debug_string(NORMAL,PSTR("IP STATUS : "),PGM_STRING);
		debug_string(NORMAL,szBuf,RAM_STRING);
		debug_string(NORMAL,g_szCRLF,PGM_STRING);
	}

	
	//if (strncasecmp_P(szBuf,PSTR("STATE: IP INITIAL"),16))
	//{
		//debug_string_1P(NORMAL,PSTR("IP STATE IS NOT AS IT SHOULD: TCP SHUTDOWN FORCED"));
		//err = sim900_tcp_close(cid);
		//if (AC_ERROR_OK != err)
		//{
			//return err;
		//}
		//return AC_SIM900_RESOURCE_UNAVAILABLE;
	//}
	
	for (uint8_t i=0;i<2;i++)
	{
		LITTLE_DELAY;
		sim900_put_string(PSTR("AT+CSTT=\""),PGM_STRING);
		sim900_put_string(service_APN,isSZPGM);
		sim900_put_string(PSTR("\"\r\n"),PGM_STRING);
		const char * szRet=sim900_wait_retstring();
		if(szRet == NULL) {
			debug_string_2P(NORMAL,PSTR("sim900_tcp_init"),PSTR("timeout waiting for AT+CSTT command, retry"));
			continue;
		}
		if(szRet!=sz_OK) {
			debug_string_2P(NORMAL,PSTR("sim900_tcp_init"),PSTR("AT+CSTT command not OK"));
			return AC_SIM900_COMM_ERROR;
		}
		break;
	}

	LITTLE_DELAY;
	//if(NULL==sim900_cmd_with_retstring(sz_AT,PGM_STRING))
	//{
		//debug_string_2P(NORMAL,PSTR("sim900_tcp_connect") ,PSTR("SIM900 not responding"));
		//return AC_SIM900_TIMEOUT;
	//}
	
	RET_ERROR_CODE e = sim900_GPRS_check_line();
	if(AC_ERROR_OK!=e) {
		debug_string_2P(NORMAL,PSTR("sim900_tcp_connect") ,PSTR("GPRS line not OK"));
		return e;
	}

	LITTLE_DELAY;
	if(NULL==sim900_cmd_with_retstring(PSTR("AT+CIICR\r\n"),PGM_STRING))
	{
		debug_string_2P(NORMAL,PSTR("sim900_tcp_connect") ,PSTR("AT+CIICR not able to connect"));
		return AC_SIM900_COMM_ERROR;
	}
	

	for(uint8_t i = 0;i<2;++i) {

		uint16_t l = sizeof(szBuf);
		memset(szBuf,0,sizeof(szBuf));

		LITTLE_DELAY;
		sim900_put_string(PSTR("AT+CIFSR\r\n"),PGM_STRING);

		RET_ERROR_CODE e = sim900_read_string(szBuf,&l);
		if(AC_ERROR_OK==e) {
			if ( strcasecmp_P(szBuf,PSTR("0.0.0.0"))==0 ) {
				debug_string_2P(NORMAL,PSTR("sim900_tcp_connect") ,PSTR("device has no valid IP address"));
				return AC_SIM900_COMM_ERROR;
				} else {
				debug_string(NORMAL,PSTR("Device IP address : "),PGM_STRING);
				debug_string(NORMAL,szBuf,RAM_STRING);
				debug_string(NORMAL,PSTR("\r\n"),PGM_STRING);
				break;
			}
		}
	}

	return AC_ERROR_OK;
	
}

RET_ERROR_CODE sim900_tcp_init( const uint8_t cid, const char * const service_APN, const uint8_t isSZPGM )
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"SIM900_TCP_INIT");

	LITTLE_DELAY;
	if(NULL==sim900_cmd_with_retstring(sz_AT,PGM_STRING))
	{
		debug_string_2P(NORMAL,PSTR("sim900_tcp_init"),PSTR("SIM900 not responding"));
		return AC_SIM900_TIMEOUT;
	}

	const RET_ERROR_CODE e = sim900_tcp_init_2( cid, service_APN, isSZPGM );
	
	LITTLE_DELAY;
	if(NULL==sim900_cmd_with_retstring(sz_AT,PGM_STRING))
	{
		debug_string_2P(NORMAL,PSTR("sim900_tcp_init") ,PSTR("SIM900 not responding"));
		return AC_SIM900_TIMEOUT;
	}

	return e;	
}

RET_ERROR_CODE sim900_tcp_connect( const uint8_t cid, const char * const service, const uint8_t isSZServicePGM, const char * port, const uint8_t isSZPortPGM)
{

	char szBuf[32];

	LITTLE_DELAY;
	if(NULL==sim900_cmd_with_retstring(sz_AT,PGM_STRING))
	{
		debug_string_2P(NORMAL,PSTR("sim900_tcp_connect") ,PSTR("SIM900 not responding"));
		return AC_SIM900_TIMEOUT;
	}

	//LITTLE_DELAY;
	//if(NULL==sim900_cmd_with_retstring(PSTR("AT+CIPHEAD=1\r\n"),PGM_STRING))
	//{
		//debug_string_2P(NORMAL,PSTR("sim900_tcp_connect") ,PSTR("SIM900 not responding"));
		//return AC_SIM900_TIMEOUT;
	//}

	LITTLE_DELAY;
	//const int icid = cid;
	//snprintf_P(szBuf,sizeof(szBuf),PSTR("AT+CIPSTART=%d,\"TCP\",\""),icid);
	szBuf[0] = '0' + cid;
	szBuf[1] = 0;
	sim900_put_string(PSTR("AT+CIPSTART="),PGM_STRING);
	sim900_put_string(szBuf,RAM_STRING);
	sim900_put_string(PSTR(",\"TCP\",\""),PGM_STRING);
	sim900_put_string(service,isSZServicePGM);
	sim900_put_string(PSTR("\",\""),PGM_STRING);
	sim900_put_string(port,isSZPortPGM);
	sim900_put_string(PSTR("\"\r\n"),PGM_STRING);
	
	if(NULL==sim900_wait_retstring())
	{
		debug_string_2P(NORMAL,PSTR("sim900_tcp_connect") ,PSTR("AT+CIPSTART not able to connect"));
		return AC_SIM900_RESOURCE_UNAVAILABLE;
	}
	
	if(NULL==sim900_wait_string(PSTR("CONNECT OK"), 10,PGM_STRING))
	{
		debug_string_2P(NORMAL,PSTR("sim900_tcp_connect") ,PSTR("AT+CIPSTART not able to connect -2-"));
		return AC_SIM900_RESOURCE_UNAVAILABLE;
	}

	return AC_ERROR_OK;
	
}

RET_ERROR_CODE sim900_tcp_read(	char * const pAnswer,
								uint16_t * const pdimBufAnsw)
{
	char szBuf[64];
	uint16_t ix = 0;

	while(ix < *pdimBufAnsw ) {
		

		if(NULL==sim900_wait_string(PSTR("+RECEIVE,"), 9,PGM_STRING))
		{
			debug_string_2P(NORMAL,PSTR("sim900_tcp_read"),PSTR("Unable to sync with +RECEIVE\r\n"));
			return AC_SIM900_TIMEOUT;
		}
	
		//const char szToken[] = {','};

		uint16_t lb = sizeof(szBuf);

		RET_ERROR_CODE e = sim900_read_string(szBuf,&lb);
		if(AC_ERROR_OK != e) {
			debug_string_2P(NORMAL,PSTR("sim900_tcp_read") ,PSTR("got timeout waiting for TCP channel"));
			return AC_SIM900_NOT_READY;
		}

		if (lb==0)
		{
			debug_string_2P(NORMAL,PSTR("sim900_tcp_read"),PSTR("Unable get chunk size from +RECEIVE\r\n"));
			return AC_SIM900_NOT_READY;
		}

		//debug_string(NORMAL,szBuf,RAM_STRING);
		
		szBuf[sizeof(szBuf)-1] = 0;
		szBuf[lb-1] = 0;
		uint16_t rc = atoi(szBuf);
		uint16_t ls = lb;
		lb = atoi(szBuf+2);
		
		snprintf_P(szBuf,sizeof(szBuf),PSTR("TCP lb:%d chan:%d len:%d\r\n"),ls,rc,lb);
		debug_string(NORMAL,szBuf,RAM_STRING);

		
		if( lb > *pdimBufAnsw ) {
			debug_string_2P(NORMAL,PSTR("sim900_tcp_read") ,PSTR("Provided Buffer is not big enough, trimming"));
			lb = *pdimBufAnsw;
		}
		
		const RET_ERROR_CODE err = sim900_read_data(pAnswer+ix,&lb);
		if (err!=AC_ERROR_OK)
		{
			*pdimBufAnsw = ix;
			return err;
		}
		ix += lb;
	}
	*pdimBufAnsw = ix;
	return AC_ERROR_OK;

}


RET_ERROR_CODE sim900_tcp_send(	const uint8_t cid, 
								const char * const pBuf, 
								const uint8_t isBufPGM, 
								const uint16_t dimBufSend, 
								char * const pAnswer, 
								uint16_t * const pdimBufAnsw)
{
	LITTLE_DELAY;
	if(NULL==sim900_cmd_with_retstring(sz_AT,PGM_STRING))
	{
		debug_string_2P(NORMAL,PSTR("sim900_tcp_send") ,PSTR("SIM900 not responding"));
		return AC_SIM900_TIMEOUT;
	}

	char szBuf[32];
	const int icid = cid;
	snprintf_P(szBuf,sizeof(szBuf),PSTR("AT+CIPSEND=%d,%d\r\n"),icid,dimBufSend);


	LITTLE_DELAY;
	sim900_put_string(szBuf,RAM_STRING);
	if(NULL==sim900_wait_string(PSTR(">"), 1,PGM_STRING)) 
	{
		debug_string_2P(NORMAL,PSTR("sim900_tcp_send") ,PSTR("error with CIPSEND"));
		return AC_SIM900_COMM_ERROR;
	}

	LITTLE_DELAY;
	sim900_put_data(pBuf,dimBufSend,isBufPGM);
	
	if(NULL==pAnswer)
	{
		if(NULL==sim900_wait_string(PSTR("SEND OK\r\n"), 9,PGM_STRING))
		{
			return AC_SIM900_TIMEOUT;
		}

		return AC_ERROR_OK;
	}


	//if(NULL==sim900_wait_string(PSTR("+RECEIVE,"), 8,PGM_STRING))
	//{
		//return AC_SIM900_TIMEOUT;
	//}

	//uint16_t len = sizeof(szBuf);
	//sim900_read_string(szBuf,&len);

	
	return sim900_tcp_read(pAnswer,pdimBufAnsw);
	//return sim900_read_data(pAnswer,pdimBufAnsw);

}

//RET_ERROR_CODE sim900_set_DNS(void)
//{
	//LITTLE_DELAY;
	//if(NULL==sim900_cmd_with_retstring(sz_AT,PGM_STRING))
	//{
		//debug_string_2P(NORMAL,PSTR("sim900_set_DNS") ,PSTR("SIM900 not responding"));
		//return AC_SIM900_NOT_READY;
	//}
//
	//LITTLE_DELAY;
	//const char * const err = sim900_cmd_with_retstring(PSTR("AT+CDNSCFG=193.70.152.15\r\n"),PGM_STRING);
	//if(sz_ERROR==err)
	//{
		//debug_string_2P(NORMAL,PSTR("sim900_set_DNS") ,PSTR("AT+CDNSCFG not OK"));
		//return AC_SIM900_COMM_ERROR;
	//}
//
	//return AC_ERROR_OK;
//}

RET_ERROR_CODE sim900_http_get_prepare(	const char * const get_URL,
									const uint8_t isURLPGM, uint16_t * const answerSize )
{
	LITTLE_DELAY;
	if(NULL==sim900_cmd_with_retstring(sz_AT,PGM_STRING))
	{
		debug_string_2P(NORMAL,PSTR("sim900_http_get_prepare") ,PSTR("SIM900 not responding"));
		return AC_SIM900_NOT_READY;
	}


	LITTLE_DELAY;
	if(sz_OK!=sim900_cmd_with_retstring(PSTR("AT+HTTPINIT\r\n"),PGM_STRING))
	{
		debug_string_2P(NORMAL,PSTR("sim900_http_get_prepare") ,PSTR("AT+HTTPINIT not OK"));
		return AC_SIM900_COMM_ERROR;
	}

	LITTLE_DELAY;
	if(sz_OK!=sim900_cmd_with_retstring(PSTR("AT+HTTPPARA=\"CID\",1\r\n"),PGM_STRING))
	{
		debug_string_2P(NORMAL,PSTR("sim900_http_get_prepare") ,PSTR("AT+HTTPPARA=\"CID\" not OK"));
		return AC_SIM900_COMM_ERROR;
	}


	LITTLE_DELAY;
	sim900_put_string(PSTR("AT+HTTPPARA=\"URL\",\""),PGM_STRING);
	sim900_put_string(get_URL,isURLPGM);
	sim900_put_string(PSTR("\"\r\n"),PGM_STRING);
	sim900_wait_retstring();

	LITTLE_DELAY;
	if(sz_OK!=sim900_cmd_with_retstring(PSTR("AT+HTTPACTION=0\r\n"),PGM_STRING))
	{
		debug_string_2P(NORMAL,PSTR("sim900_http_get_prepare") ,PSTR("HTTPACTION didn't return OK"));
		return AC_SIM900_COMM_ERROR;
	}
	
	if(NULL == sim900_wait_string(PSTR("+HTTPACTION:"), 12,PGM_STRING))
	{
		debug_string_2P(NORMAL,PSTR("sim900_http_get_prepare") ,PSTR("didn't got +httpaction aborting"));
		return AC_SIM900_COMM_ERROR;
	}
	
	//HTTPACTION result code in the form of <method>,<status>,<datalen>
	char szBuf[32] = {0};
	static const uint8_t len_szBuf = sizeof(szBuf);
	uint16_t lb = len_szBuf;
	RET_ERROR_CODE e = sim900_read_string(szBuf,&lb);
	if(AC_ERROR_OK != e) {
		debug_string_2P(NORMAL,PSTR("sim900_http_get_prepare") ,PSTR("got timeout waiting for httpaction response"));
		return AC_SIM900_NOT_READY;
	}

	debug_string_1P(NORMAL,PSTR("httpaction returned: "));
	debug_string(NORMAL,szBuf,RAM_STRING);
	debug_string_1P(NORMAL,g_szCRLF);
	
	uint8_t i = 0;
	while( i < (sizeof(szBuf)-8) )
	{
		if(szBuf[i++]==',') break;
	}
	
	if(szBuf[i]!='2' || szBuf[i+1]!='0' || szBuf[i+2]!='0') {
		//HTTPACTION didn't return OK
		debug_string_2P(NORMAL,PSTR("sim900_http_get_prepare") ,PSTR("httpget response was not OK"));
		//if(szBuf[i]=='6' && szBuf[i+1]=='0' && szBuf[i+2]=='3') {
			//debug_string_1P(NORMAL,PSTR("Trying to force a new DNS"));
			//sim900_set_DNS();
		//}
		return AC_SIM900_COMM_ERROR;
	}
	
	if(NULL!=answerSize)
	{
		*answerSize = atoi(&szBuf[i+4]);
	}

	return AC_ERROR_OK;								
}


RET_ERROR_CODE sim900_http_get(	const char * const get_URL, 
							const uint8_t isURLPGM, 
							char * const return_value, 
							uint16_t * const rv_len, bool isBinary )
{
	
	uint16_t anSize;

	const RET_ERROR_CODE r = sim900_http_get_prepare(get_URL,isURLPGM, &anSize);
	if(r!=AC_ERROR_OK)
	{
		return r;
	}
	
	if (isBinary)
	{
		if(anSize>*rv_len)
		{
			return AC_BUFFER_OVERFLOW;
		}
	
		*rv_len = anSize;

	}

	return sim900_http_read(return_value,rv_len, isBinary);
}


RET_ERROR_CODE sim900_http_post(	const char * const post_URL, 
							const uint8_t isURLPGM, 
							const char * const post_data, 
							const uint16_t len_post_data,
							const uint8_t isPostPGM )
{

	LITTLE_DELAY;
	if(NULL==sim900_cmd_with_retstring(sz_AT,PGM_STRING))
	{
		debug_string_2P(NORMAL,PSTR("sim900_http_post") ,PSTR("SIM900 not responding"));
		return AC_SIM900_NOT_READY;
	}


	LITTLE_DELAY;
	if(sz_OK!=sim900_cmd_with_retstring(PSTR("AT+HTTPINIT\r\n"),PGM_STRING))
	{
		return AC_SIM900_COMM_ERROR;
	}


	LITTLE_DELAY;
	if(sz_OK!=sim900_cmd_with_retstring(PSTR("AT+HTTPPARA=\"CID\",1\r\n"),PGM_STRING))
	{
		return AC_SIM900_COMM_ERROR;
	}

	
	LITTLE_DELAY;
	sim900_put_string(PSTR("AT+HTTPPARA=\"URL\",\""),PGM_STRING);
	sim900_put_string(post_URL,isURLPGM);
	sim900_put_string(PSTR("\"\r\n"),PGM_STRING);
	sim900_wait_retstring();


	LITTLE_DELAY;
	const uint8_t SZBUF_LEN = 64;
	char szBuf[SZBUF_LEN];
	snprintf_P(szBuf,sizeof(szBuf),PSTR("AT+HTTPDATA=%d,%d\r\n"),len_post_data,30000);
	sim900_put_string(szBuf,RAM_STRING);

	if(NULL==sim900_wait_string(PSTR("DOWNLOAD"), 8,PGM_STRING))
	{
		return AC_SIM900_TIMEOUT;
	}
	
	LITTLE_DELAY;
	sim900_put_string(post_data,isPostPGM);
	sim900_wait_retstring();
	

	LITTLE_DELAY;
	if(sz_OK!=sim900_cmd_with_retstring(PSTR("AT+HTTPACTION=1\r\n"),PGM_STRING))
	{
		debug_string_2P( NORMAL,PSTR("sim900_http_post") ,PSTR("HTTPACTION didn't return OK") );
		return AC_SIM900_TIMEOUT;
	}

	
	if(NULL == sim900_wait_string(PSTR("+HTTPACTION:"), 12,PGM_STRING))
	{
		debug_string_2P(NORMAL,PSTR("sim900_http_post") ,PSTR("+httpaction ...waiting..."));
		if(NULL == sim900_wait_string(PSTR("+HTTPACTION:"), 12,PGM_STRING))
		{
			debug_string_2P(NORMAL,PSTR("sim900_http_post") ,PSTR("+httpaction ...still waiting..."));
			if(NULL == sim900_wait_string(PSTR("+HTTPACTION:"), 12,PGM_STRING))
			{
				debug_string_2P(NORMAL,PSTR("sim900_http_post") ,PSTR("+httpaction aborting") );
				return AC_SIM900_TIMEOUT;
			}
		}
	}

/////////////
// We need to know the return code of the post
		
	uint16_t len_szBuf = SZBUF_LEN;
	RET_ERROR_CODE e = sim900_read_string(szBuf,&len_szBuf);


	debug_string(NORMAL,PSTR("httpaction returned: "),PGM_STRING);
	debug_string(NORMAL,szBuf,RAM_STRING);
	debug_string(NORMAL,g_szCRLF,PGM_STRING);
	
	uint8_t i = 0;
	while( i < (sizeof(szBuf)-8) )
	{
		if(szBuf[i++]==',') break;
	}
		
	if(szBuf[i]!='2' || szBuf[i+1]!='0' || szBuf[i+2]!='0') {
		//HTTPACTION didn't return OK
		debug_string_2P(NORMAL,PSTR("sim900_http_post") ,PSTR("httppost response was not OK"));
		return AC_SIM900_COMM_ERROR;
	}

	
	return AC_ERROR_OK;
}

RET_ERROR_CODE sim900_http_close( void )
{
	LITTLE_DELAY;
	LITTLE_DELAY;
	if(sz_OK!=sim900_cmd_with_retstring(PSTR("AT+HTTPTERM\r\n"),PGM_STRING))
	{
		return AC_SIM900_COMM_ERROR;
	}
	
	return AC_ERROR_OK;
}

uint8_t sim900_ftp_open(	const char * const ftp_server, 
							const uint8_t isSZServerPGM,
							const char * const ftp_username, 
							const uint8_t isSZUserNamePGM,
							const char * const ftp_pwd,
							const uint8_t isSZPWDPGM)
{
	sim900_put_string(g_szCRLF,PGM_STRING);
	sim900_put_string(PSTR("AT+FTPCID=1"),PGM_STRING);
	sim900_put_string(g_szCRLF,PGM_STRING);
	
	sim900_put_string(PSTR("AT+FTPSERV=\""),PGM_STRING);
	sim900_put_string(ftp_server,isSZServerPGM);
	sim900_put_string(PSTR("\""),PGM_STRING);
	sim900_put_string(g_szCRLF,PGM_STRING);
	
	sim900_put_string(PSTR("AT+FTPUN=\""),PGM_STRING);
	sim900_put_string(ftp_username,isSZUserNamePGM);
	sim900_put_string(PSTR("\""),PGM_STRING);
	sim900_put_string(g_szCRLF,PGM_STRING);
	
	sim900_put_string(PSTR("AT+FTPW=\""),PGM_STRING);
	sim900_put_string(ftp_pwd,isSZPWDPGM);
	sim900_put_string(PSTR("\""),PGM_STRING);
	sim900_put_string(g_szCRLF,PGM_STRING);
	
	return 0;
}
uint8_t sim900_ftp_put(const char * const file_name,
						const uint8_t isSZFNamePGM,
						const char * const file_path,
						const uint8_t isSZFilePathPGM,
						const char * const file_data,
						const uint8_t isFileDataPGM)
{
	char buf[64];
	LITTLE_DELAY;
	sim900_put_string(PSTR("\""),PGM_STRING);
	sim900_put_string(PSTR("AT+FTPPUTNAME=\""),PGM_STRING);
	sim900_put_string(file_name,isSZFNamePGM);
	sim900_put_string(PSTR("\""),PGM_STRING);
	sim900_put_string(g_szCRLF,PGM_STRING);
	LITTLE_DELAY;
	
	sim900_put_string(g_szCRLF,PGM_STRING);
	sim900_put_string(PSTR("AT+FTPPUTPATH=\""),PGM_STRING);
	sim900_put_string(file_path,isSZFilePathPGM);
	sim900_put_string(PSTR("\""),PGM_STRING);
	sim900_put_string(g_szCRLF,PGM_STRING);
	LITTLE_DELAY;
	
	sim900_put_string(PSTR("AT+FTPPUT=1"),PGM_STRING);
	sim900_put_string(g_szCRLF,PGM_STRING);
	LITTLE_DELAY;

	sim900_put_string(PSTR("AT+FTPPUT=2,"),PGM_STRING);
	itoa(strlen(file_data),buf,10);
	sim900_put_string(buf,RAM_STRING);
	sim900_put_string(g_szCRLF,PGM_STRING);
	LITTLE_DELAY;
	
	sim900_put_string(file_data,isFileDataPGM);
	sim900_put_string(PSTR("AT+FTPPUT=2,0"),PGM_STRING);
	sim900_put_string(g_szCRLF,PGM_STRING);
	LITTLE_DELAY;
	
	return 0;
}

#ifndef SIM900_USART_POLLED
ISR(USART_GPRS_RX_Vect)
{
	USART_RX_CBuffer_Complete(&sim900_usart_data);
}

//ISR(USART_GPRS_TX_Vect)
//{
	//
//}

//ISR(USART_GPRS_DRE_Vect)
//{
	//USART_DataRegEmpty(&usart_data);
//}

//ISR(USART_GPRS_TX_Vect)
//{
	//udi_cdc_putc((uint8_t)((USART_SERIAL_GPRS)->DATA));
//}

#endif