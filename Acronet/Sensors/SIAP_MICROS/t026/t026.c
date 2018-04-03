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
#include "t026.h"
#include "Acronet/drivers/SP336/SP336.h"
#include "Acronet/services/MODBUS_RTU/mb_crc.h"
#include "Acronet/services/MODBUS_RTU/master_rtu.h"



#if !defined(T026_MBUS_CH)
#pragma message "[!!! PROJECT WARNING !!!] in file " __FILE__\
" SYMBOL T026_MBUS_CH not defined, using default just to compile, your project may not work as aspected"
#define T026_MBUS_CH	0
#endif


#define T026_MEASURES_NUMBER	32
#define T026_DATABUFSIZE		17	// Raw measures buffer size. On this measures array statistics are done.
#define T026_MEASUREBUFMID		8   //

typedef struct  
{
	int16_t temp;
	int16_t rh;
} DATAVAL;

static MBUS_PDU g_mbp = {0};

static DATAVAL g_Data[T026_DATABUFSIZE];
static uint8_t g_samples = 0;

static volatile uint8_t g_DataIsBusy = 0;



//static void t026_rx(const char c)
//{
////	usart_putchar(USART_DEBUG,c);
////	NMEALine_addChar(c);
//}


static uint8_t medianInsert_right(DATAVAL val,uint8_t pos)
{
	DATAVAL v0 = val;
	for(uint8_t idx=pos;idx<T026_DATABUFSIZE;++idx)
	{
		const DATAVAL a = g_Data[idx];
		const DATAVAL v1 = (a.temp==0)?v0:a;
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
		const DATAVAL v1 = (a.temp==0)?v0:a;
		g_Data[idx] = v0;
		v0 = v1;
	} while (idx-- != 0);
	
	return 0;
}

static uint8_t medianInsert(const DATAVAL val)
{
	uint8_t idx;
	const DATAVAL vm = g_Data[T026_MEASUREBUFMID];
	
	//if (vm==0) {
	//return medianInsert_right(val,0);
	//} else
	if (val.temp<vm.temp) {
		for(idx=T026_MEASUREBUFMID;idx>0;--idx) {
			const DATAVAL vr = g_Data[idx];
			const DATAVAL vl = g_Data[idx-1];
			if ((val.temp>=vl.temp) && (val.temp<=vr.temp)) {
				return medianInsert_right(val,idx);
			}
		}
		//New Minimum, it also manage the undef value
		return medianInsert_right(val,0);

		} else if(val.temp>vm.temp)	{
		for(idx=T026_MEASUREBUFMID;idx<T026_DATABUFSIZE-1;++idx) {
			const DATAVAL vl = g_Data[idx];
			const DATAVAL vr = g_Data[idx+1];
			if ((val.temp>=vl.temp) && (val.temp<=vr.temp)) {
				return medianInsert_left(val,idx);
			}
		}
		//New Maximum
		return medianInsert_left(val,T026_DATABUFSIZE-1);
		} else if (val.temp==vm.temp) {
		medianInsert_right(val,T026_MEASUREBUFMID);
		medianInsert_left(val,T026_MEASUREBUFMID);
	}

	return 0;
}



RET_ERROR_CODE t026_init(void)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"T026 Init");

	return AC_ERROR_OK;
}

void t026_enable(void)
{
	//usart_set_rx_interrupt_level(SP336_USART0,USART_INT_LVL_LO);
	//usart_rx_enable(SP336_USART0);

	/* ToDo */
}

void t026_disable(void)
{
	/* ToDo */
}

RET_ERROR_CODE t026_get_data(T026_DATA * const ps)
{
	ps->temp	= g_Data[T026_MEASUREBUFMID].temp;
	ps->rh		= g_Data[T026_MEASUREBUFMID].rh;
	ps->samples = g_samples;
	
	
	return AC_ERROR_OK;
}

static float interpret_pdu_cdab_float(uint8_t * const pData)
{
	typedef union {
		float fval;
		uint8_t bval[4];
	} VV;
	
	const register VV v = {	.bval[3] = pData[2] ,
							.bval[2] = pData[3] ,
							.bval[1] = pData[0] ,
							.bval[0] = pData[1] };

	return v.fval;	
}

static void interpret_pdu(MBUS_PDU * const pPDU)
{
	if (g_samples > 254) {
		return;
	}

	DATAVAL dv;
							
	const float temp = interpret_pdu_cdab_float( &(pPDU->data.byte[0]) );
	const float rh = interpret_pdu_cdab_float( &(pPDU->data.byte[4]) );
	
	if(temp==-9999.0F) {
		debug_string_1P(NORMAL,PSTR("[WARNING] Invalid value"));
		return;
	} else {
		dv.temp = (int16_t) (temp*10);
		dv.rh   = (int16_t) (rh*10);
	}
	
	
	char szBUF[64];
	sprintf_P(szBUF,PSTR(" (%d , %d)\r\n"),dv.temp,dv.rh);
	debug_string(NORMAL,szBUF,RAM_STRING);

	medianInsert(dv);
	g_samples++;
}


bool t026_Yield( void )
{
	while(! MBUS_IS_EMPTY(T026_MBUS_CH) )
	{
		const uint8_t b = MBUS_GET_BYTE(T026_MBUS_CH);
		if ( MBUS_STATUS_END == MBUS_BUILD_DGRAM(T026_MBUS_CH,&g_mbp,b) )
		{
			usart_putchar(USART_DEBUG,'y');

			const uint16_t crcc = MBUS_GET_CRC(T026_MBUS_CH);
			const uint16_t crcp =( (((uint16_t) g_mbp.crc_hi) << 8) | g_mbp.crc_lo );
			if (crcc == crcp)
			{
				usart_putchar(USART_DEBUG,'u');
				interpret_pdu(&g_mbp);
				} else {
				char szBUF[64];
				sprintf_P(szBUF,PSTR("%04X != %04X\r\n"),crcc,crcp);
				debug_string(NORMAL,szBUF,RAM_STRING);
			}
			MBUS_RELEASE(T026_MBUS_CH);
		}
		//		usart_putchar(USART_DEBUG,'Y');
		
		return true;
	}
	return false;
}

//bool t026_Yield( void )
//{
	//while(! MBUS_is_empty(T026_MBUS_CH))
	//{
		//
		//MBUS_build_dgram(&g_mbc,&g_mbp,MBUS_get_byte(T026_MBUS_CH));
		//if (MBUS_STATUS_END == g_mbc.status)
		//{
			//usart_putchar(USART_DEBUG,'y');
//
			//const uint16_t crcc = mb_crc_get(g_mbc.transmission_crc);
			//const uint16_t crcp =( (((uint16_t) g_mbp.crc_hi) << 8) | g_mbp.crc_lo );
			//if (crcc == crcp)
			//{
				//usart_putchar(USART_DEBUG,'u');
				//interpret_pdu(&g_mbp);
			//} else {
				//char szBUF[64];
				//sprintf_P(szBUF,PSTR("%04X != %04X\r\n"),crcc,crcp);
				//debug_string(NORMAL,szBUF,RAM_STRING);
			//}
			//g_mbc.status = MBUS_STATUS_BEGIN;
		//}
////		usart_putchar(USART_DEBUG,'Y');
				//
		//return true;
	//}
	//return false;
//}

void t026_periodic(void)
{
	static const __flash uint8_t cmd[] = {0x15,0x04,0x00,0x00,0x00,0x04,0xF2,0xDD};

	if ( AC_ERROR_OK != MBUS_LOCK(T026_MBUS_CH) )
	{
		return;
	}

	usart_putchar(USART_DEBUG,'p');
	uint8_t buf[16];
	memcpy_P(buf,cmd,8);
	MBUS_ISSUE_CMD(T026_MBUS_CH,buf,8);
}

RET_ERROR_CODE t026_reset_data(void)
{
	g_samples = 0;
	return AC_ERROR_OK;
}

RET_ERROR_CODE t026_Data2String(const T026_DATA * const st,char * const sz, uint16_t * const len_sz)
{
	const uint16_t samples = st->samples;
	
	uint16_t len = snprintf_P(sz,*len_sz,PSTR("&ll=%u&lt=%u&nSmp=%u"),st->temp,st->rh,samples);
	
	const RET_ERROR_CODE e = (len < *len_sz) ? AC_ERROR_OK : AC_BUFFER_OVERFLOW;
	*len_sz = len;
	return e;
//	return AC_ERROR_OK;
}


#ifdef RMAP_SERVICES

RET_ERROR_CODE t026_Data2String_RMAP(	 uint8_t * const subModule
									,const T026_DATA * const st
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

