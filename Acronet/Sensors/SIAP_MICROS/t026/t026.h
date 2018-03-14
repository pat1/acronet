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


#ifndef T026_H_
#define T026_H_


typedef struct
{
	int16_t			temp;
	int16_t			rh;
	uint8_t			samples;
} T026_DATA;

RET_ERROR_CODE t026_init(void);
void t026_periodic(void);
void t026_enable(void);
void t026_disable(void);
RET_ERROR_CODE t026_get_data(T026_DATA * const);
RET_ERROR_CODE t026_reset_data(void);
RET_ERROR_CODE t026_Data2String(const T026_DATA * const st,char * const sz, uint16_t * len_sz);

bool t026_Yield( void );

#ifdef RMAP_SERVICES
RET_ERROR_CODE t026_Data2String_RMAP( uint8_t * const subModule ,const T026_DATA * const st ,const uint32_t timeStamp ,const uint16_t timeWindow ,char * const szTopic ,int16_t * const len_szTopic ,char * const szMessage ,int16_t * const len_szMessage );
#endif


#endif /* T023_H_ */
