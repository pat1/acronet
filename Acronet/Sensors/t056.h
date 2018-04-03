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


#ifndef T056_H_
#define T056_H_


typedef struct
{
	int16_t			levl;
	int16_t			temp;
	uint8_t			samples;
} T056_DATA;

RET_ERROR_CODE t056_init(void);
void t056_periodic(void);
void t056_enable(void);
void t056_disable(void);
RET_ERROR_CODE t056_get_data(T056_DATA * const);
RET_ERROR_CODE t056_reset_data(void);
RET_ERROR_CODE t056_Data2String(const T056_DATA * const st,char * const sz, uint16_t * len_sz);

bool t056_Yield( void );

#ifdef RMAP_SERVICES
RET_ERROR_CODE t056_Data2String_RMAP( uint8_t * const subModule ,const T056_DATA * const st ,const uint32_t timeStamp ,const uint16_t timeWindow ,char * const szTopic ,int16_t * const len_szTopic ,char * const szMessage ,int16_t * const len_szMessage );
#endif


#endif /* T056_H_ */
