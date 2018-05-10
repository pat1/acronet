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
#include "Acronet/HAL/hal_interface.h"
#include <asf.h>
#include <stdio.h>
#include "progmem.h"
#include <conf_board.h>
#include <conf_usart_serial.h>


#include "Acronet/globals.h"

#include "Acronet/channels/pulse/pulse.h"

static volatile uint8_t sync = 0;

#ifdef USES_PULSE_CHAN_0
static volatile PULSE_CHAN_STATISTICS s0;


bool pulse_is_empty_CH0(void)
{
	return 	!(sync & 0b00010000);
}

void pulse_get_tstamp_CH0(PULSE_CHAN_STATISTICS * const ps)
{
	while(sync & 0b00000001) { barrier(); }
	sync |= 0b00000001;
	*ps = s0;
	sync &= ~0b00010001;
}

static void pulse_CH0(void)
{
	while(sync & 0b00000001) { barrier(); }
	sync |= 0b00000001;

	const uint32_t millis = hal_rtc_get_millis();
	const uint32_t epoch = hal_rtc_get_time();

	uint32_t t1 = epoch - s0.lastPulseEpoch;
	uint32_t tbounce = 0xFFFFFFFF;
	if( 0 == (t1 & 0xFFC00000) ) { //if it overflows slope is undefined
		tbounce = (t1 << 10) - (millis - s0.lastPulseMillis);
		if( 0 == (tbounce & 0xFFFFFF80) ) 
		{
			return;
		}
		
	}

	s0.lastPulseEpoch = epoch;
	s0.lastPulseMillis = millis;

	
	if (0 == (sync & 0b00010000))
	{
		sync |= 0b00010000;
		s0.numOfPulses = 1;
		s0.firstPulseEpoch = epoch;
		s0.firstPulseMillis = millis;
		s0.minDT = tbounce;
	} else {
		if(tbounce < s0.minDT) {
			s0.minDT = tbounce;
		}

		++s0.numOfPulses;
	}
	
	sync &= ~0b00000001;
}

ISR(PORTR_INT0_vect)
{
	pulse_CH0();		
}

#endif

#ifdef USES_PULSE_CHAN_1
static volatile PULSE_CHAN_STATISTICS s1;


bool pulse_is_empty_CH1(void)
{
	return 	!(sync & 0b00100000);
}

void pulse_get_tstamp_CH1(PULSE_CHAN_STATISTICS * const ps)
{
	while(sync & 0b00000010) { barrier(); }
	sync |= 0b00000010;
	*ps = s1;
	sync &= ~0b00100010;
}

void pulse_CH1(void)
{
	while(sync & 0b00000010) { barrier(); }
	sync |= 0b00000010;

	const uint32_t millis = hal_rtc_get_millis();
	const uint32_t epoch = hal_rtc_get_time();

	uint32_t t1 = epoch - s1.lastPulseEpoch;
	uint32_t tbounce = 0xFFFFFFFF;
	if( 0 == (t1 & 0xFFC00000) ) { //if it overflows slope is undefined
		tbounce = (t1 << 10) - (millis - s1.lastPulseMillis);
		if( 0 == (tbounce & 0xFFFFFF80) ) {
			{
				return;
			}
			
	}

	s1.lastPulseEpoch = epoch;
	s1.lastPulseMillis = millis;
	
	if (0 == (sync & 0b00100000))
	{
		sync |= 0b00100000;
		s1.numOfPulses = 1;
		s1.firstPulseEpoch = epoch;
		s1.firstPulseMillis = millis;
		s1.minDT = tbounce;
	} else {
		if(tbounce < s1.minDT) {
			s1.minDT = tbounce;
		}

		++s1.numOfPulses;
	}
	
	sync &= ~0b00000010;
}

ISR(PORTR_INT1_vect)
{
	pulse_CH1();
}


#endif

#ifdef USES_PULSE_CHAN_2
#error "Pulse channel 2 is not implemented"
#endif


#ifdef USES_PULSE_CHAN_3
#error "Pulse channel 3 is not implemented"
#endif
