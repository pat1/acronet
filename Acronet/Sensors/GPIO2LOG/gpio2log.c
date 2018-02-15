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
#include <stdio.h>
#include "Acronet/globals.h"
#include "Acronet/Sensors/GPIO2LOG/gpio2log.h"
#include "Acronet/drivers/StatusLED/status_led.h"
#include "Acronet/services/config/config.h"
#include "Acronet/services/LOG/LOG.h"



RET_ERROR_CODE gpio2log_init(void)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"GPIO2LOG init");


	ioport_configure_port_pin(&PORTA,0b11111101, IOPORT_MODE_TOTEM | IOPORT_MODE_PULLUP | IOPORT_DIR_INPUT | IOPORT_BOTHEDGES );
	ioport_configure_port_pin(&PORTB,0b00000111, IOPORT_MODE_TOTEM | IOPORT_MODE_PULLUP | IOPORT_DIR_INPUT | IOPORT_BOTHEDGES );

	PORT_ConfigureInterrupt0( &PORTA, PORT_INT0LVL_LO_gc, 0b11111101 );
	PORT_ConfigureInterrupt0( &PORTB, PORT_INT0LVL_LO_gc, 0b00000111 );

	sleepmgr_lock_mode(SLEEPMGR_IDLE);

	return AC_ERROR_OK;

}

static volatile uint8_t pending_status_A;
static volatile uint8_t pending_status_B;

static volatile uint8_t consolidated_status_A;
static volatile uint8_t consolidated_status_B;

static void write2log(const uint8_t vals,const char szTemplate[])
{
	char buf[128];
	sprintf_P(buf,szTemplate,hal_rtc_get_time());
	char * const psz = buf + strlen(buf);
	for (uint8_t i=0;i<8;i++)
	{
		const uint8_t j = i*2;
		psz[ j ]= '0' + ((vals&(1U << i)) ? 1 : 0);
		psz[j+1]= ',';

	}
		
	psz[15] = ']';
	psz[16] = '}';
	psz[17] = 0;
	LOG_say(buf);
	LOG_process_buffer();
	psz[17] = '\r';
	psz[18] = '\n';
	psz[19] = 0;
	debug_string(NORMAL,buf,RAM_STRING);
}

bool gpio2log_Yield(void)
{
	
	if (consolidated_status_A != pending_status_A)
	{
		const uint8_t vals = pending_status_A;
		static const __flash char szTemplate[] = "{\"TIME\":%lu,\"PORT\":\"A\",\"BITFIELD\":[";
		write2log(vals,szTemplate);
		consolidated_status_A = vals;
	} 

	if (consolidated_status_B != pending_status_B)
	{
		const uint8_t vals = pending_status_B;
		static const __flash char szTemplate[] = "{\"TIME\":%lu,\"PORT\":\"B\",\"BITFIELD\":[";
		write2log(vals,szTemplate);
		consolidated_status_B = vals;
	}
	
	return false;
}

static void gpio2log_PA_tip(void)
{
	static uint32_t lastTip = 0;

	const uint32_t epoch = hal_rtc_get_time();
	if ((lastTip==0) || (epoch!=lastTip) )
	{
		pending_status_A = PORTA_IN;
	}
	lastTip = epoch;
}

static void gpio2log_PB_tip(void)
{
	static uint32_t lastTip = 0;

	const uint32_t epoch = hal_rtc_get_time();
	if ((lastTip==0) || (epoch!=lastTip) )
	{
		pending_status_B = PORTB_IN;
	}
	lastTip = epoch;

}

//static void gpio2log_PA_tip(const uint16_t cents)
//{
	//static volatile uint8_t lock = 0;
	//if (lock == 1)
	//{
		//return;
	//}
	//
	//lock = 1;
	//
	//uint8_t vals = PORTA_IN;
	//
	//char buf[16];
	//buf[0] = 'A';
//
	//for (uint8_t i=0;i<8;i++)
	//{
		//buf[i+1]= '0' + ((vals&(1U << i)) ? 1 : 0);
	//}
//
	//buf[ 9]='\r';
	//buf[10]='\n';
	//buf[11]=0;
//
	//debug_string(NORMAL,buf,RAM_STRING);
	//lock = 0;
//}

//static void gpio2log_PB_tip(const uint16_t cents)
//{
	//static volatile uint8_t lock = 0;
	//if (lock == 1)
	//{
		//return;
	//}
	//
	//lock = 1;
//
	//uint8_t vals = PORTB_IN;
//
	//char buf[16];
	//buf[0] = 'B';
//
	//for (uint8_t i=0;i<8;i++)
	//{
		//buf[i+1]= '0' + ((vals&(1U << i)) ? 1 : 0);
	//}
//
	//buf[ 9]='\r';
	//buf[10]='\n';
	//buf[11]=0;
//
	//debug_string(NORMAL,buf,RAM_STRING);
	//lock = 0;
//
//}

#ifdef SETUP_GPIO2LOG

ISR(PORTA_INT0_vect)
{
	gpio2log_PA_tip();
}

ISR(PORTB_INT0_vect)
{
	gpio2log_PB_tip();
}
#endif