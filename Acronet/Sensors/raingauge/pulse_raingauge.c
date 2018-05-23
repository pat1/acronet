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

#include "Acronet/setup.h"


#include <asf.h>
#include <stdio.h>
#include "Acronet/globals.h"
#include "Acronet/Sensors/raingauge/pulse_raingauge.h"
#include "Acronet/drivers/StatusLED/status_led.h"
//#include "services/module_registry/module_registry.h"
#include "Acronet/services/config/config.h"
#include "Acronet/channels/pulse/pulse.h"



static void raingauge_tip(RAINGAUGE_DATA * const pSelf,const PULSE_CHAN_STATISTICS * const pStat )
{
	pSelf->tips += pStat->numOfPulses;
	const uint16_t s = pStat->minDT;
	if( s < pSelf->maxSlope ) {
		pSelf->maxSlope = s;
	}
	
	char buf[64];
	unsigned int a = pStat->numOfPulses ,b = pStat->minDT ,c = pSelf->tips ,d = pSelf->maxSlope;
	sprintf_P(buf,PSTR("%u %u - %u %u\r\n") ,a
											,b
											,c
											,d);
	debug_string(NORMAL,buf,RAM_STRING);

}

#ifdef RMAP_SERVICES

static RET_ERROR_CODE internal_D2S_RMAP(	const uint8_t id
											,uint8_t * const subModule
											,const RAINGAUGE_DATA * const st
											,const uint32_t timeStamp
											,const uint16_t timeWindow
											,char * const szTopic
											,int16_t * const len_szTopic
											,char * const szMessage
											,int16_t * const len_szMessage	)
{

	*subModule = 255;

	struct calendar_date dt;
	calendar_timestamp_to_date(timeStamp,&dt);

	static const __flash char fmt[][32] = {"/1,0,%d/1,-,-,-/B13011","/1,0,%d/103,2001,-,-/B13011"};


	int16_t len = snprintf_P(	szTopic,
								*len_szTopic,
								fmt[(id==0)?0:1],
								timeWindow	);


	if(len >= *len_szTopic)
	{
		return AC_BUFFER_OVERFLOW;
	}

	*len_szTopic = len;

	const int vf = st->tips;

	len = snprintf_P(	szMessage,
						*len_szMessage,
						PSTR("{\"v\":%d, \"t\":\"%d-%02d-%02dT%02d:%02d:%02d\"}"),
						vf,dt.year,dt.month+1,dt.date+1,dt.hour,dt.minute,0);//dt.second-1 );

	if(len >= *len_szMessage)
	{
		return AC_BUFFER_OVERFLOW;
	}

	*len_szMessage = len;

	return AC_ERROR_OK;
}

RET_ERROR_CODE raingauge_Data2String_RMAP(	uint8_t * const subModule
											,const RAINGAUGE_DATA * const st
											,const uint32_t timeStamp
											,const uint16_t timeWindow
											,char * const szTopic
											,int16_t * const len_szTopic
											,char * const szMessage
											,int16_t * const len_szMessage	)
{
	return internal_D2S_RMAP(0,subModule,st,timeStamp,timeWindow,szTopic,len_szTopic,szMessage,len_szMessage);
}

RET_ERROR_CODE raingauge_Data2String_RMAP_aux(	uint8_t * const subModule
												,const RAINGAUGE_DATA * const st
												,const uint32_t timeStamp
												,const uint16_t timeWindow
												,char * const szTopic
												,int16_t * const len_szTopic
												,char * const szMessage
												,int16_t * const len_szMessage	)
{
	return internal_D2S_RMAP(1,subModule,st,timeStamp,timeWindow,szTopic,len_szTopic,szMessage,len_szMessage);
}

#endif

static RET_ERROR_CODE internal_Data2String(const RAINGAUGE_DATA * const st,char * const sz, uint16_t * len_sz)
{
	static const __flash char fmt[] = "&P=%u&S=%u";
	int16_t len = snprintf_P(sz,*len_sz,fmt,st->tips,st->maxSlope);

	const RET_ERROR_CODE e = (len < *len_sz) ? AC_ERROR_OK : AC_BUFFER_OVERFLOW;
	*len_sz = len;
	return e;
}


RET_ERROR_CODE raingauge_Data2String(const RAINGAUGE_DATA * const st,char * const sz, uint16_t * len_sz)
{
	return internal_Data2String(st,sz, len_sz);
}

#define MODULE_INTERFACE_PRIVATE_DATATYPE RAINGAUGE_DATA


#define MODINST_PARAM_ID MOD_ID_RAINGAUGE
#include "Acronet/datalogger/modinst/module_interface_definition.h"
#undef MODINST_PARAM_ID