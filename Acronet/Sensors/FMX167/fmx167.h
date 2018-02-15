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


#ifndef FMX167_H_
#define FMX167_H_

//#ifndef
//
//#endif

typedef struct
{
	int16_t			v;
	int16_t			v_max;
	int16_t			v_min;
	uint16_t		level;
} FMX167_DATA;

RET_ERROR_CODE fmx167_init(void);
void fmx167_enable(void);
void fmx167_disable(void);
RET_ERROR_CODE fmx167_get_data(FMX167_DATA * const);
RET_ERROR_CODE fmx167_reset_data(void);
RET_ERROR_CODE fmx167_Data2String(const FMX167_DATA * const st,char * const sz, uint16_t * len_sz);

bool fmx167_Yield(void);

#define FMX167_VOLTMETER			ADCB
#define FMX167_VOLTMETER_CH			ADC_CH0
#define FMX167_VOLTMETER_PIN_POS	ADCCH_POS_PIN5
#define FMX167_VOLTMETER_PIN_NEG	ADCCH_POS_PIN2

#endif /* FMX167_H_ */