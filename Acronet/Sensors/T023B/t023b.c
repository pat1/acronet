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
#include "Acronet/HAL/hal_interface.h"
#include "Acronet/globals.h"
#include "t023b.h"
#include "Acronet/drivers/SP336/SP336.h"
#include "Acronet/services/MODBUS_RTU/master_rtu.h"



#if !defined(T023B_MBUS_CH)
#pragma message "[!!! PROJECT WARNING !!!] in file " __FILE__\
" SYMBOL T023B_MBUS_CH not defined, using default just to compile, your project may not work as aspected"
#define T023B_MBUS_CH	0
#endif


#define T023B_MEASURES_NUMBER	32
#define T023B_DATABUFSIZE		17	// Raw measures buffer size. On this measures array statistics are done.
#define T023B_MEASUREBUFMID		8   //

typedef struct  
{
	int16_t levl;
	int16_t temp;
} DATAVAL;

static DATAVAL g_Data[T023B_DATABUFSIZE];
static uint8_t g_samples = 0;

static volatile uint8_t g_DataIsBusy = 0;

static void t023b_rx(const char c)
{
//	usart_putchar(USART_DEBUG,c);
//	NMEALine_addChar(c);
}


static uint8_t medianInsert_right(DATAVAL val,uint8_t pos)
{
	DATAVAL v0 = val;
	for(uint8_t idx=pos;idx<T023B_DATABUFSIZE;++idx)
	{
		const DATAVAL a = g_Data[idx];
		const DATAVAL v1 = (a.levl==0)?v0:a;
		g_Data[idx] = v0;
		v0 = v1;
	}
	return 0;
}

static uint8_t medianInsert_left(DATAVAL val,uint8_t pos)
{
	DATAVAL v0 = val;
	uint8_t idx=pos;
	do 
	{
		const DATAVAL a = g_Data[idx];
		const DATAVAL v1 = (a.levl==0)?v0:a;
		g_Data[idx] = v0;
		v0 = v1;
	} while (idx-- != 0);
	
	return 0;
}

static uint8_t medianInsert(const DATAVAL val)
{
	uint8_t idx;
	const DATAVAL vm = g_Data[T023B_MEASUREBUFMID];
	
	//if (vm==0) {
	//return medianInsert_right(val,0);
	//} else
	if (val.levl<vm.levl) {
		for(idx=T023B_MEASUREBUFMID;idx>0;--idx) {
			const DATAVAL vr = g_Data[idx];
			const DATAVAL vl = g_Data[idx-1];
			if ((val.levl>=vl.levl) && (val.levl<=vr.levl)) {
				return medianInsert_right(val,idx);
			}
		}
		//New Minimum, it also manage the undef value
		return medianInsert_right(val,0);

		} else if(val.levl>vm.levl)	{
		for(idx=T023B_MEASUREBUFMID;idx<T023B_DATABUFSIZE-1;++idx) {
			const DATAVAL vl = g_Data[idx];
			const DATAVAL vr = g_Data[idx+1];
			if ((val.levl>=vl.levl) && (val.levl<=vr.levl)) {
				return medianInsert_left(val,idx);
			}
		}
		//New Maximum
		return medianInsert_left(val,T023B_DATABUFSIZE-1);
		} else if (val.levl==vm.levl) {
		medianInsert_right(val,T023B_MEASUREBUFMID);
		medianInsert_left(val,T023B_MEASUREBUFMID);
	}

	return 0;
}



RET_ERROR_CODE t023b_init(void)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"T023B Init");

	return AC_ERROR_OK;
}

void t023b_enable(void)
{
	//usart_set_rx_interrupt_level(SP336_USART0,USART_INT_LVL_LO);
	//usart_rx_enable(SP336_USART0);

	/* ToDo */
}

void t023b_disable(void)
{
	/* ToDo */
}

RET_ERROR_CODE t023b_get_data(T023B_DATA * const ps)
{
	
	
	return AC_ERROR_OK;
}

bool t023b_Yield( void )
{
	static char szBuf[128];
	static uint8_t idx = 0;
	while(! MBUS_is_empty(T023B_MBUS_CH))
	{
				
		return true;
	}
	return false;
}

void t023b_periodic(void)
{
	static const __flash uint8_t cmd[] = {0x15,0x04,0x00,0x00,0x00,0x04,0xF2,0xDD};
	uint8_t buf[16];
	memcpy_P(buf,cmd,8);
	MBUS_issue_cmd(0,buf,8);
}

RET_ERROR_CODE t023b_reset_data(void)
{
	g_samples = 0;
	return AC_ERROR_OK;
}

RET_ERROR_CODE t023b_Data2String(const T023B_DATA * const st,char * const sz, uint16_t * const len_sz)
{
	const uint16_t samples = st->samples;
	
//	uint16_t len = snprintf_P(sz,*len_sz,PSTR("&V=%u&Vmax=%u&Vmin=%u&nSmp=%u"),st->v,st->v_max,st->v_min,samples);
	
//	const RET_ERROR_CODE e = (len < *len_sz) ? AC_ERROR_OK : AC_BUFFER_OVERFLOW;
//	*len_sz = len;
//	return e;
	return AC_ERROR_OK;
}


#ifdef RMAP_SERVICES

RET_ERROR_CODE t023b_Data2String_RMAP(	 uint8_t * const subModule
									,const T023B_DATA * const st
									,const uint32_t timeStamp
									,const uint16_t timeWindow
									,char * const szTopic
									,int16_t * const len_szTopic
									,char * const szMessage
									,int16_t * const len_szMessage	)
{
	*subModule = 255;
	struct VP61_COEFF {float a;float b;} coeff;
	struct calendar_date dt;
	calendar_timestamp_to_date(timeStamp,&dt);

	int16_t len = snprintf_P(	szTopic,
								*len_szTopic,
								PSTR("/254,0,%d/1,-,-,-/B13215"),
								0	);


	if(len >= *len_szTopic)
	{
		return AC_BUFFER_OVERFLOW;
	}

	*len_szTopic = len;

	const int32_t vf = 337.4F - ((((float)st->v)*0.385410621F) - 125.2959954F);

	len = snprintf_P(	szMessage,
						*len_szMessage,
						PSTR("{\"v\":%ld , \"t\":\"%d-%02d-%02dT%02d:%02d:%02d\"}"),
						vf,dt.year,dt.month+1,dt.date+1,dt.hour,dt.minute,0);//dt.second-1 );

	if(len >= *len_szMessage)
	{
		return AC_BUFFER_OVERFLOW;
	}

	*len_szMessage = len;

	return AC_ERROR_OK;
}

#endif //RMAP_SERVICES

