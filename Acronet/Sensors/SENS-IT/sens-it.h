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

#ifndef SENS_IT_H_
#define SENS_IT_H_


enum {
	SENSIT_STAT_BEG=0,
	SENSIT_STAT_CO=SENSIT_STAT_BEG,
	SENSIT_STAT_NO2,
	SENSIT_STAT_O3,
	SENSIT_STAT_CH4,
	SENSIT_STAT_NOx,
	SENSIT_STAT_C6H6,
	SENSIT_STAT_END
	};

typedef struct {
		float		m_stat[SENSIT_STAT_END];
		uint16_t	m_samples[SENSIT_STAT_END];
		uint16_t	m_err[SENSIT_STAT_END];
} SENSIT_STATS;

RET_ERROR_CODE sens_it_init(void);
void sens_it_enable(void);
void sens_it_disable(void);
RET_ERROR_CODE sens_it_get_data(SENSIT_STATS * const);
RET_ERROR_CODE sens_it_reset_data(void);
RET_ERROR_CODE sens_it_Data2String(const SENSIT_STATS * const st,char * const sz, uint16_t * len_sz);

bool sens_it_Yield(void);

void sens_it_triggerReading(uint8_t address);

#endif /* SENS_IT_H_ */