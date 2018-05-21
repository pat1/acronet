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
#include "Acronet/Sensors/PulseAnemometer/pulse_anemometer.h"
#include "Acronet/drivers/StatusLED/status_led.h"



volatile ANEMOMETER_DATA anemometer_stats;

typedef struct {
	volatile uint32_t lastTipEpoch[2];
	volatile uint32_t lastTipMillis[2];
} INTERNAL_STATISTICS;

volatile INTERNAL_STATISTICS ins;

static uint8_t tick;


static void anemometer_tip(void)
{
	volatile ANEMOMETER_DATA * const ps = &anemometer_stats;

	const uint16_t millis = hal_rtc_get_millis();
	const uint32_t epoch = hal_rtc_get_time();
	
	uint32_t s = 0xFFFF;
	
	uint8_t t = tick;
	uint32_t lastTipEpoch = ins.lastTipEpoch;

	if(lastTipEpoch==0) {
		t = 1-t;
		lastTipEpoch = ins.lastTipEpoch[t];
	}
	
	
	if(lastTipEpoch!=0) {
		
		const uint32_t d = (epoch - lastTipEpoch);
		const uint32_t m = ins.lastTipMillis[t];

		s = (1000*d) + millis - m;
		
		if (s<100) //Time between two tips is too low
		{
			return;
		}
		
		if(s< ps->maxSlope) {
			ps->maxSlope = s;
		}

	}


	ins.lastTipEpoch[tick] = epoch;
	ins.lastTipMillis[tick] = millis;
	ps->tips++;
}


static void init_stats(void)
{
	const static __flash ANEMOMETER_DATA zd = { .maxSlope = 0xFFFF , .tips = 0 };
	anemometer_stats = zd;
	
	tick = 0;
	const static __flash INTERNAL_STATISTICS zs = { {0,0} , {0,0} };
	ins = zs;
}


void anemometer_reset_data(void)
{
	anemometer_stats.maxSlope = 0xFFFF;
	anemometer_stats.tips	 = 0;
	
	const uint8_t t = 1 - tick;
	
	ins.lastTipEpoch[t] = 0;
	ins.lastTipMillis[t] = 0;
	
	tick = t;
}


void anemometer_get_data(ANEMOMETER_DATA * const ps)
{
	ps->maxSlope = anemometer_stats.maxSlope;
	ps->tips     = anemometer_stats.tips;
}


RET_ERROR_CODE anemometer_init(void)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"pulse_anemometer init");
	PORT_ConfigureInterrupt0( &PORTR, PORT_INT0LVL_LO_gc, 0x03 );


	sleepmgr_lock_mode(SLEEPMGR_IDLE);

	init_stats();
	//anemometer_reset_data();
	return AC_ERROR_OK;
}


RET_ERROR_CODE anemometer_Data2String(const ANEMOMETER_DATA * const st,char * const sz, int16_t * len_sz)
{
	int16_t len = snprintf_P(sz,*len_sz,PSTR("&WP=%u&WS=%u"),st->tips,st->maxSlope);

	const RET_ERROR_CODE e = (len < *len_sz) ? AC_ERROR_OK : AC_BUFFER_OVERFLOW;
	*len_sz = len;
	return e;
}

#ifdef SETUP_AMEMOMETER

ISR(PORTR_INT1_vect)
{
	anemometer_tip(tc_read_count(&TCC1));
}

#endif
