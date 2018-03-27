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


#include <stdio.h>
#include <asf.h>
#include "Acronet/globals.h"
#include "Acronet/Sensors/raingauge/pulse_raingauge.h"
//Extern delarations
const char g_szCRLF[] PROGMEM = "\r\n";


#ifdef GIT_TAG
const char g_szGIT_TAG[] PROGMEM = xstr(GIT_TAG);
#else
const char g_szGIT_TAG[] PROGMEM = "UNKNOWN";
#endif

#ifdef PRJ_TAG
const char g_szPRJ_TAG[] PROGMEM = xstr(PRJ_TAG);
#else
const char g_szPRJ_TAG[] PROGMEM = "UNKNOWN";
#endif

uint8_t g_log_verbosity = VERY_VERBOSE;


//void debug_string_P(uint8_t level,const char * const sz)
void debug_string_2P(const uint8_t level,const char * const szWho,const char * const szWhat)
{
	if (NULL != szWho)
	{
		debug_string(level,PSTR("[\e[33m"),PGM_STRING);
		debug_string(level,szWho,PGM_STRING);
		debug_string(level,PSTR("\e[39m] : "),PGM_STRING);
	}
	debug_string(level,szWhat,PGM_STRING);
	debug_string(level,PSTR("\r\n"),PGM_STRING);
}

void debug_string_1P(const uint8_t level,const char * const szWhat)
{
	debug_string(level,szWhat,PGM_STRING);
	debug_string(level,PSTR("\r\n"),PGM_STRING);
}

void debug_string(uint8_t level,const char * const sz,const uint8_t isPGM)
{
	if(level>g_log_verbosity) return;

	const char * p = sz;

	if(isPGM) {
		nvm_wait_until_ready();
		while(1) {
			const uint8_t c = nvm_flash_read_byte(  (flash_addr_t) p++ );
			if(c==0) break;
			while (usart_data_register_is_empty(USART_DEBUG) == false) {}
			usart_put(USART_DEBUG, c);
		}
		
	} else {
		while(*p) {
			while (usart_data_register_is_empty(USART_DEBUG) == false) {}
			usart_put(USART_DEBUG, *p++);

		}
	}
}

static void freeRam (void) {
	extern int __heap_start, *__brkval;
	char sz[32];
	int v;
	sprintf_P(sz,PSTR("mem: %d\r\n"), ((int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval)));
	debug_string(NORMAL,sz,RAM_STRING);
}
void debug_function_out_name_print(const char * const fname[])
{
	debug_string(VERBOSE,PSTR("[\e[32mOUT\e[39m]\t/\\ \e[97m"),PGM_STRING);
	debug_string(VERBOSE,*fname,RAM_STRING);
	debug_string(VERBOSE,PSTR("\r\n"),PGM_STRING);
}

void debug_function_in_name_print(const uint8_t level,const char fname[])
{
	debug_string(level,PSTR("[\e[32mIN\e[39m]\t\\/ \e[97m"),PGM_STRING);\
	debug_string(level,fname,RAM_STRING);\
	debug_string(level,PSTR("\e[39m\r\n"),PGM_STRING);
	freeRam();
}

void debug_function_in_name_print_P(const uint8_t level,const char fname[])
{
	debug_string(level,PSTR("[\e[32mIN\e[39m]\t\\/ \e[97m"),PGM_STRING);
	debug_string(level,fname,PGM_STRING);
	debug_string(level,PSTR("\e[39m\r\n"),PGM_STRING);
	freeRam();
}

void debug_function_out_name_print_P(const char * const fname[])
{
	debug_string(VERBOSE,PSTR("[\e[32mOUT\e[39m]\t/\\ \e[97m"),PGM_STRING);
	debug_string(VERBOSE,*fname,PGM_STRING);
	debug_string(VERBOSE,PSTR("\e[39m\r\n"),PGM_STRING);
}
/*
void dump_rainstats_to_log(const uint8_t id)
{
	RAINGAUGE_STATS rs;
	raingauge_get_stats(id,&rs);

	char szBuf[256];
	snprintf_P(szBuf,sizeof(szBuf),PSTR("\tfirst tip cents: %u\r\n\tlast tip cents:%u\r\n\tmax slope cents: %u\r\n\ttips: %u\r\n\tmax slope %u\r\n")
	,rs.firstTip_cents
	,rs.lastTip_cents
	,rs.maxSlope_cents
	,rs.tips
	,rs.maxSlope);

	debug_string(NORMAL,szBuf,false);
}
*/
/*
void vbus_action(bool b_high)
{
	if (b_high) {
		// Attach USB Device
	} else {
		// VBUS not present
	}
}


*/
