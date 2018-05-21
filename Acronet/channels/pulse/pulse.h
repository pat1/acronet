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




#ifndef PULSE_CHANNEL_H_
#define PULSE_CHANNEL_H_

typedef struct {
	uint32_t firstPulseEpoch;
	uint16_t firstPulseMillis;
	uint32_t lastPulseEpoch;
	uint16_t lastPulseMillis;
	uint16_t minDT;
	uint8_t  numOfPulses;
} PULSE_CHAN_STATISTICS;


#define PULSE_PREP(p1,p2,...)			p1##p2(__VA_ARGS__)

#define PULSE_IS_EMPTY(ch)		PULSE_PREP( pulse_is_empty_CH , ch )
#define PULSE_GET_TSTAMP(ch,a1)	PULSE_PREP( pulse_get_tstamp_CH , ch , a1 )
#define PULSE_INIT(ch)			PULSE_PREP( pulse_init_CH , ch )

void pulse_init_CH0(void);
void pulse_init_CH1(void);
void pulse_init_CH2(void);
void pulse_init_CH3(void);

bool pulse_is_empty_CH0(void);
bool pulse_is_empty_CH1(void);
bool pulse_is_empty_CH2(void);
bool pulse_is_empty_CH3(void);

void pulse_get_tstamp_CH0(PULSE_CHAN_STATISTICS * const);
void pulse_get_tstamp_CH1(PULSE_CHAN_STATISTICS * const);
void pulse_get_tstamp_CH2(PULSE_CHAN_STATISTICS * const);
void pulse_get_tstamp_CH3(PULSE_CHAN_STATISTICS * const);

#endif /* PULSE_CHANNEL_H_ */