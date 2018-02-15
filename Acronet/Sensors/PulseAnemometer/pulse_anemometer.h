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


#ifndef PULSE_ANEMOMETER_H_
#define PULSE_ANEMOMETER_H_

typedef struct
{
	uint16_t			maxSlope;
	uint16_t			tips;
} ANEMOMETER_DATA;


RET_ERROR_CODE anemometer_init(void);



void anemometer_get_data(ANEMOMETER_DATA * const ps);
void anemometer_reset_data(void);


RET_ERROR_CODE anemometer_Data2String(const ANEMOMETER_DATA * const st,char * const sz, int16_t * len_sz);




#endif /* PULSE_ANEMOMETER_H_ */