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


#ifndef HD3910_H_
#define HD3910_H_


typedef struct
{
	int16_t			wvc;
	int16_t			temp;
	uint8_t			samples;
} HD3910_DATA;

RET_ERROR_CODE hd3910_init(void);
void hd3910_periodic(void);
void hd3910_enable(void);
void hd3910_disable(void);
RET_ERROR_CODE hd3910_get_data(HD3910_DATA * const);
RET_ERROR_CODE hd3910_reset_data(void);
RET_ERROR_CODE hd3910_Data2String(const HD3910_DATA * const st,char * const sz, uint16_t * len_sz);

bool hd3910_Yield( void );

#ifdef RMAP_SERVICES
RET_ERROR_CODE hd3910_Data2String_RMAP( uint8_t * const subModule ,const HD3910_DATA * const st ,const uint32_t timeStamp ,const uint16_t timeWindow ,char * const szTopic ,int16_t * const len_szTopic ,char * const szMessage ,int16_t * const len_szMessage );
#endif


#endif /* HD3910_H_ */
