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


#ifndef VEGAPULS61_H_
#define VEGAPULS61_H_

//#ifndef
//
//#endif

typedef struct
{
	int16_t			v;
	int16_t			v_max;
	int16_t			v_min; /* ToDo */
	uint8_t			samples;
} VP61_DATA;

RET_ERROR_CODE vp61_init(void);
void vp61_enable(void);
void vp61_disable(void);
RET_ERROR_CODE vp61_get_data(VP61_DATA * const);
RET_ERROR_CODE vp61_reset_data(void);
RET_ERROR_CODE vp61_Data2String(const VP61_DATA * const st,char * const sz, uint16_t * len_sz);

void  vp61_periodic(void);

//bool vp61_Yield(void);
#ifdef RMAP_SERVICES
RET_ERROR_CODE vp61_Data2String_RMAP( uint8_t * const subModule ,const VP61_DATA * const st ,const uint32_t timeStamp ,const uint16_t timeWindow ,char * const szTopic ,int16_t * const len_szTopic ,char * const szMessage ,int16_t * const len_szMessage );
#endif

//#define VP61_VOLTMETER			ADCA
//#define VP61_VOLTMETER_CH		ADC_CH0
//#define VP61_VOLTMETER_PIN_POS	ADCCH_POS_PIN4
//#define VP61_VOLTMETER_PIN_NEG	ADCCH_POS_PIN0


//#define RECURRENT_TASK_TABLE (RECURRENT_TASK_TABLE {vp61_process_sample();})

#endif /* VEGAPULS61_H_ */