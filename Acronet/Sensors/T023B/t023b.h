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


#ifndef T023B_H_
#define T023B_H_

//#ifndef
//
//#endif

typedef struct
{
	int16_t			v;
	//int16_t			v_max;
	//int16_t			v_min; /* ToDo */
	uint8_t			samples;
} T023B_DATA;

RET_ERROR_CODE t023b_init(void);
void t023b_periodic(void);
void t023b_enable(void);
void t023b_disable(void);
RET_ERROR_CODE t023b_get_data(T023B_DATA * const);
RET_ERROR_CODE t023b_reset_data(void);
RET_ERROR_CODE t023b_Data2String(const T023B_DATA * const st,char * const sz, uint16_t * len_sz);

bool t023b_Yield( void );
void t023b_trigger_reading(void);

//bool vp61_Yield(void);
#ifdef RMAP_SERVICES
RET_ERROR_CODE t023b_Data2String_RMAP( uint8_t * const subModule ,const T023B_DATA * const st ,const uint32_t timeStamp ,const uint16_t timeWindow ,char * const szTopic ,int16_t * const len_szTopic ,char * const szMessage ,int16_t * const len_szMessage );
#endif


#endif /* T023B_H_ */