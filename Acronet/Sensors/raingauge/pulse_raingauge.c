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

#define RAINGAUGE1 0
#define RAINGAUGE2 1

//#define MAXSLOPE_DATATYPE uint16_t
//#define MAXSLOPE_UNDEF_VALUE ((MAXSLOPE_DATATYPE)(-1))
//#define CENTS_UNDEF_VALUE MAXSLOPE_UNDEF_VALUE

typedef struct
{
	RAINGAUGE_DATA raingauge_stats;
	uint32_t lastTipEpoch[2];
	uint32_t lastTipMillis[2];
	uint8_t sig_data = 0;
	uint8_t tick;
} RAINGAUGE_PRIVATE_DATA;


typedef struct {
	uint32_t lastTipEpoch[2];
    uint32_t lastTipMillis[2];
} INTERNAL_STATISTICS;


#ifdef SETUP_RAINGAUGE_AUX
static volatile RAINGAUGE_DATA raingauge_stats[2];
static volatile INTERNAL_STATISTICS ins[2];
static volatile uint8_t sig_data1 = 0;
static volatile uint8_t sig_data2 = 0;
static uint8_t tick[2] ;
#else
static volatile RAINGAUGE_DATA raingauge_stats[1];
static volatile INTERNAL_STATISTICS ins[1];
static volatile uint8_t sig_data1 = 0;
static uint8_t tick[1] ;
#endif


typedef struct {
	uint8_t swapped;
} RAINGAUGE_SETUP;


static void raingauge_tip(const uint8_t id)
{
	volatile RAINGAUGE_DATA * const ps = &raingauge_stats[id];
	

	const uint32_t millis = hal_rtc_get_millis();
	const uint32_t epoch = hal_rtc_get_time();
	
	uint32_t s = 0xFFFF;
	
	uint8_t t = tick[id];
	uint32_t lastTipEpoch = ins[id].lastTipEpoch[t];

	if(lastTipEpoch==0) {
		t = 1-t;
		lastTipEpoch = ins[id].lastTipEpoch[t];
	}
	
	
	if(lastTipEpoch!=0) {
		
		const uint32_t d = (epoch - lastTipEpoch);
		const uint32_t m = ins[id].lastTipMillis[t];

		s = (1000*d) + millis - m; 
		
		if (s<200) //Time between two tips is too low
		{
			return;
		}
		
		if(s< ps->maxSlope) {
			ps->maxSlope = s;
		}

	}


	ins[id].lastTipEpoch[tick[id]] = epoch;
	ins[id].lastTipMillis[tick[id]] = millis;
	ps->tips++;

	//if (RAINGAUGE1==id)
	//{
			//usart_putchar(USART_DEBUG,'A');
	//} 
	//else
	//{
			//usart_putchar(USART_DEBUG,'B');
	//}
	//
	//status_led_toggle();	
}

static void internal_reset_data(const uint8_t id)
{
	raingauge_stats[id].maxSlope = 0xFFFF;
	raingauge_stats[id].tips	 = 0;
	
	const uint8_t t = 1 - tick[id];
	
	ins[id].lastTipEpoch[t] = 0;
	ins[id].lastTipMillis[t] = 0;
	
	tick[id] = t;
}

void raingauge_reset_data(void)
{
	simple_signal_wait(&sig_data1);
	
	SIGNAL_SET_AND_CLEAR_AUTOMATIC(sig_data1);
	
	internal_reset_data(RAINGAUGE1);
}


void raingauge_get_data(RAINGAUGE_DATA * const ps)
{
	simple_signal_wait(&sig_data1);
	
	SIGNAL_SET_AND_CLEAR_AUTOMATIC(sig_data1);

	volatile RAINGAUGE_DATA * const s = &raingauge_stats[RAINGAUGE1];

	//memcpy_ram2ram(ps,s,sizeof(RAINGAUGE_DATA));
	ps->tips = s->tips;
	ps->maxSlope = s->maxSlope;
}

#ifdef SETUP_RAINGAUGE_AUX

void raingauge_get_data_aux(RAINGAUGE_DATA * const ps)
{
	simple_signal_wait(&sig_data2);
	
	SIGNAL_SET_AND_CLEAR_AUTOMATIC(sig_data2);

	RAINGAUGE_DATA * const s = &raingauge_stats[RAINGAUGE2];

//	memcpy_ram2ram(ps,s,sizeof(RAINGAUGE_DATA));
	ps->tips = s->tips;
	ps->maxSlope = s->maxSlope;

}

void raingauge_reset_data_aux(void)
{
	simple_signal_wait(&sig_data2);
		
	SIGNAL_SET_AND_CLEAR_AUTOMATIC(sig_data2);

	internal_reset_data(RAINGAUGE2);
}



#endif

static void init_stats(uint8_t id)
{
	const static __flash RAINGAUGE_DATA zd = { .maxSlope = 0xFFFF , .tips = 0 };
	raingauge_stats[id] = zd;
	
	tick[id] = 0;
	const static __flash INTERNAL_STATISTICS zs = { {0,0} , {0,0} };
	ins[id] = zs;
}

RET_ERROR_CODE raingauge_init(void) 
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"RAINGAUGE INIT");
	RAINGAUGE_SETUP s;
	CFG_ITEM_ADDRESS f = 0;
	if( AC_ERROR_OK != cfg_find_item(CFG_TAG_RAINGAUGE_PORT,&f))
	{
		debug_string_1P(NORMAL,PSTR("[ERROR] Missing configuration file\r\n"));
		return AC_ERROR_GENERIC;
	}

	cfg_get_item_file(f,&s,sizeof(RAINGAUGE_SETUP));


	PORT_ConfigureInterrupt0( &PORTR, PORT_INT0LVL_LO_gc, (s.swapped==0) ? 0x02 : 0x01 ); 
//	raingauge_reset_data();
	init_stats(RAINGAUGE1);



#ifdef SETUP_RAINGAUGE_AUX
	PORT_ConfigureInterrupt1( &PORTR, PORT_INT1LVL_LO_gc, (s.swapped==0) ? 0x01 : 0x02 );
	//raingauge_reset_data_aux();
	init_stats(RAINGAUGE2);
#endif	

	sleepmgr_lock_mode(SLEEPMGR_IDLE);

	return AC_ERROR_OK;
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

	static const char __flash fmt[][32] = {"/1,0,%d/1,-,-,-/B13011","/1,0,%d/103,2001,-,-/B13011"};


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

static RET_ERROR_CODE internal_Data2String(const uint8_t id,const RAINGAUGE_DATA * const st,char * const sz, int16_t * len_sz)
{
	static const char __flash fmt[][18] = {"&P=%u&S=%u","&PAUX=%u&SAUX=%u"};
	int16_t len = snprintf_P(sz,*len_sz,fmt[(id==0)?0:1],st->tips,st->maxSlope);

	const RET_ERROR_CODE e = (len < *len_sz) ? AC_ERROR_OK : AC_BUFFER_OVERFLOW;
	*len_sz = len;
	return e;
}

RET_ERROR_CODE raingauge_Data2String_aux(const RAINGAUGE_DATA * const st,char * const sz, int16_t * len_sz)
{
	return internal_Data2String(1,st,sz, len_sz);
}

RET_ERROR_CODE raingauge_Data2String(const RAINGAUGE_DATA * const st,char * const sz, int16_t * len_sz)
{
	return internal_Data2String(0,st,sz, len_sz);
}

#ifdef SETUP_RAINGAUGE

ISR(PORTR_INT0_vect)
{
	simple_signal_wait(&sig_data1);
	
	SIGNAL_SET_AND_CLEAR_AUTOMATIC(sig_data1);
	
	raingauge_tip(RAINGAUGE1);
}

#ifdef SETUP_RAINGAUGE_AUX

ISR(PORTR_INT1_vect)
{
	simple_signal_wait(&sig_data2);
		
	SIGNAL_SET_AND_CLEAR_AUTOMATIC(sig_data2);

	raingauge_tip(RAINGAUGE2);
}

#endif
#endif
