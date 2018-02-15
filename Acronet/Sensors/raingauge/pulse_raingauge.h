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


#ifndef PULSE_RAINGAUGE_H_
#define PULSE_RAINGAUGE_H_

//#define MAXSLOPE_DATATYPE uint16_t
//#define MAXSLOPE_UNDEF_VALUE ((MAXSLOPE_DATATYPE)(-1))
//#define CENTS_UNDEF_VALUE MAXSLOPE_UNDEF_VALUE


typedef struct
{
	uint16_t			maxSlope;
	uint16_t			tips;
} RAINGAUGE_DATA;


RET_ERROR_CODE raingauge_init(void);


void raingauge_get_data(RAINGAUGE_DATA * const ps);
void raingauge_reset_data(void);

#ifdef SETUP_RAINGAUGE_AUX

void raingauge_get_data_aux(RAINGAUGE_DATA * const ps);
void raingauge_reset_data_aux(void);

#endif

RET_ERROR_CODE raingauge_Data2String_aux(const RAINGAUGE_DATA * const st,char * const sz, int16_t * len_sz);
RET_ERROR_CODE raingauge_Data2String(const RAINGAUGE_DATA * const st,char * const sz, int16_t * len_sz);

#ifdef RMAP_SERVICES
RET_ERROR_CODE raingauge_Data2String_RMAP( uint8_t * const subModule,const RAINGAUGE_DATA * const st ,const uint32_t timeStamp ,const uint16_t timeWindow ,char * const szTopic ,int16_t * const len_szTopic ,char * const szMessage ,int16_t * const len_szMessage );
#ifdef SETUP_RAINGAUGE_AUX
RET_ERROR_CODE raingauge_Data2String_RMAP_aux( uint8_t * const subModule ,const RAINGAUGE_DATA * const st ,const uint32_t timeStamp ,const uint16_t timeWindow ,char * const szTopic ,int16_t * const len_szTopic ,char * const szMessage ,int16_t * const len_szMessage );
#endif
#endif



#endif /* PULSE_RAINGAUGE_H_ */