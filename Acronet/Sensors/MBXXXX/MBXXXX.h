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


#ifndef LEVELGAUGE_H_
#define LEVELGAUGE_H_

#define LG_ADC_PORT		PORTB
#define LG_ADC			ADCB
#define LG_ADC_CH		ADC_CH0
#define LG_ID			0


#ifdef MB_RESOLUTION_MM
#define MBXXXX_NORANGEFOUND		9999 //MB7563
#else
#define MBXXXX_NORANGEFOUND		765	//MB7040
#endif

typedef struct {
	int16_t val;
	int16_t maxVal;
	int16_t minVal;
	uint16_t samples;
	uint16_t noRangeFound;
#ifdef 	MBXXXX_MISS_STATS
	uint16_t misses;
#endif	
} MBXXXX_DATA;


RET_ERROR_CODE MBXXXX_init(void);
void MBXXXX_enable(void);
void MBXXXX_disable(void);
void MBXXXX_get_data(MBXXXX_DATA * const );
void MBXXXX_reset_data(void);
bool MBXXXX_Yield(void);

RET_ERROR_CODE MBXXXX_Data2String(const MBXXXX_DATA * const st,char * const sz, uint16_t * len_sz);

void MBXXXX_triggerReading(void);


#endif /* LEVELGAUGE_H_ */