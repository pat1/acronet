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


#ifndef T023_H_
#define T023_H_


typedef struct
{
	int16_t			levl;
	int16_t			temp;
	uint8_t			samples;
} T023_DATA;

RET_ERROR_CODE t023_init(void);
void t023_periodic(void);
void t023_enable(void);
void t023_disable(void);
RET_ERROR_CODE t023_get_data(T023_DATA * const);
RET_ERROR_CODE t023_reset_data(void);
RET_ERROR_CODE t023_Data2String(const T023_DATA * const st,char * const sz, uint16_t * len_sz);

bool t023_Yield( void );

#ifdef RMAP_SERVICES
RET_ERROR_CODE t023_Data2String_RMAP( uint8_t * const subModule ,const T023_DATA * const st ,const uint32_t timeStamp ,const uint16_t timeWindow ,char * const szTopic ,int16_t * const len_szTopic ,char * const szMessage ,int16_t * const len_szMessage );
#endif


#endif /* T023_H_ */
