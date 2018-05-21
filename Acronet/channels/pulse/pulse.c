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


#define SYNCFLAG_CH0_INTERRUPT  0b00000001
#define SYNCFLAG_CH0_DATAISDIRT 0b00000010
#define SYNCFLAG_CH0_DATAISLOCK 0b00000100

#define SYNCFLAG_CH1_INTERRUPT  0b00010000
#define SYNCFLAG_CH1_DATAISDIRT 0b00100000
#define SYNCFLAG_CH1_DATAISLOCK 0b01000000


#ifdef USES_PULSE_CHAN_0
static volatile PULSE_CHAN_STATISTICS s0 = {0};

void pulse_init_CH0(void)
{
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTR,0), IOPORT_TOTEM | IOPORT_DIR_INPUT | IOPORT_SRL_ENABLED | IOPORT_PULL_DOWN | IOPORT_FALLING );

	PORT_ConfigureInterrupt0( &PORTR, PORT_INT0LVL_LO_gc, 0x01 );
	sleepmgr_lock_mode(SLEEPMGR_IDLE);
}

bool pulse_is_empty_CH0(void)
{
	return 	!(sync & SYNCFLAG_CH0_DATAISDIRT);
}

void pulse_get_tstamp_CH0(PULSE_CHAN_STATISTICS * const ps)
{
	//while(sync & SYNCFLAG_CH0_DATAISLOCK) { barrier(); }
	sync |= SYNCFLAG_CH0_DATAISLOCK;
	*ps = s0;
	sync &= ~(SYNCFLAG_CH0_DATAISDIRT | SYNCFLAG_CH0_DATAISLOCK);
}

static void pulse_CH0_2(const uint32_t epoch, const uint16_t millis, const uint16_t tbounce)
{
	s0.lastPulseEpoch = epoch;
	s0.lastPulseMillis = millis;

	
	if (0 == (sync & SYNCFLAG_CH0_DATAISDIRT))
	{
		sync |= SYNCFLAG_CH0_DATAISDIRT;
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
	
}

static void pulse_CH0_1(void)
{
	
	const uint16_t millis = hal_rtc_get_millis();
	const uint32_t epoch = hal_rtc_get_time();

	uint32_t t1 = epoch - s0.lastPulseEpoch;
	uint16_t tbounce = 0xFFFF;
	if( 0 == (t1 & 0xFFFFFFC0) ) { //if it overflows leave tbounce as undefined
		tbounce = (t1 << 10) + (millis - s0.lastPulseMillis);
		if( 0 == (tbounce & 0xFF80) )
		{
			return;
		}
		
	}

	//while(sync & SYNCFLAG_CH0_DATAISLOCK) { barrier(); }
	sync |= SYNCFLAG_CH0_DATAISLOCK;

	pulse_CH0_2(epoch,millis,tbounce);
	sync &= ~SYNCFLAG_CH0_DATAISLOCK;

	usart_putchar(USART_DEBUG,'!');

}

static void pulse_CH0(void)
{
	//////////////////////////////////////////
	//TODO: This sync flag is not needed
	//and eventually should be implemented with 
	//an atomic memory access (in this implementation
	//i used a single byte to sync both interrupts
	//leaving a possible race condition bug)
	
	if (0 != (sync & SYNCFLAG_CH0_INTERRUPT)) 
	{
		return;
	}
	sync |= SYNCFLAG_CH0_INTERRUPT;

	usart_putchar(USART_DEBUG,'^');

	pulse_CH0_1();
	sync &= ~SYNCFLAG_CH0_INTERRUPT;
	
}


ISR(PORTR_INT0_vect)
{
	pulse_CH0();
}

#endif

#ifdef USES_PULSE_CHAN_1
static volatile PULSE_CHAN_STATISTICS s1 = {0};

void pulse_init_CH1(void)
{
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTR,1), IOPORT_DIR_INPUT | IOPORT_TOTEM | IOPORT_SRL_ENABLED | IOPORT_PULL_DOWN | IOPORT_FALLING );
	
	PORT_ConfigureInterrupt1( &PORTR, PORT_INT1LVL_LO_gc,  0x02 );
	sleepmgr_lock_mode(SLEEPMGR_IDLE);
}

bool pulse_is_empty_CH1(void)
{
	return 	!(sync & SYNCFLAG_CH1_DATAISDIRT);
}

void pulse_get_tstamp_CH1(PULSE_CHAN_STATISTICS * const ps)
{
	//while(sync & SYNCFLAG_CH1_DATAISLOCK) { barrier(); }
	sync |= SYNCFLAG_CH1_DATAISLOCK;
	*ps = s1;
	sync &= ~(SYNCFLAG_CH1_DATAISDIRT | SYNCFLAG_CH1_DATAISLOCK) ;
}

static void pulse_CH1_2(const uint32_t epoch, const uint16_t millis, const uint16_t tbounce)
{
	s1.lastPulseEpoch = epoch;
	s1.lastPulseMillis = millis;
	
	if (0 == (sync & SYNCFLAG_CH1_DATAISDIRT))
	{
		sync |= SYNCFLAG_CH1_DATAISDIRT;
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
	
}

static void pulse_CH1_1(void)
{

	const uint16_t millis = hal_rtc_get_millis();
	const uint32_t epoch = hal_rtc_get_time();

	uint32_t t1 = epoch - s1.lastPulseEpoch;
	uint16_t tbounce = 0xFFFF;
	if( 0 == (t1 & 0xFFFFFFC0) ) { //if it overflows leave tbounce as undefined
		tbounce = (t1 << 10) + (millis - s1.lastPulseMillis);
		if( 0 == (tbounce & 0xFF80) ) 
		{
			return;
		}
			
	}

	//while(sync & SYNCFLAG_CH1_DATAISLOCK) { barrier(); }
	sync |= SYNCFLAG_CH1_DATAISLOCK;
	pulse_CH1_2(epoch,millis,tbounce);
	sync &= ~SYNCFLAG_CH1_DATAISLOCK;
	usart_putchar(USART_DEBUG,'*');

}

static void pulse_CH1(void)
{
	//////////////////////////////////////////
	//TODO: This sync flag is not needed
	//and eventually should be implemented with
	//an atomic memory access (in this implementation
	//i used a single byte to sync both interrupts
	//leaving a possible race condition bug)

	if (0 != (sync & SYNCFLAG_CH1_INTERRUPT))
	{
		return;
	}
	sync |= SYNCFLAG_CH1_INTERRUPT;

	usart_putchar(USART_DEBUG,'.');
	pulse_CH1_1();
	sync &= ~SYNCFLAG_CH1_INTERRUPT;
	
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
