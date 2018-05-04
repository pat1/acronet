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
#include "t023.h"
#include "Acronet/drivers/SP336/SP336.h"
#include "Acronet/services/MODBUS_RTU/mb_crc.h"
#include "Acronet/services/MODBUS_RTU/master_rtu.h"



#if !defined(T023_MBUS_CH)
#pragma message "[!!! PROJECT WARNING !!!] in file " __FILE__\
" SYMBOL T023_MBUS_CH not defined, using default just to compile, your project may not work as aspected"
#define T023_MBUS_CH	0
#endif


#define T023_MEASURES_NUMBER	32
#define T023_DATABUFSIZE		17	// Raw measures buffer size. On this measures array statistics are done.
#define T023_MEASUREBUFMID		8   //

typedef struct  
{
	int16_t levl;
	int16_t temp;
} DATAVAL;

static MBUS_PDU g_mbp = {0};

static DATAVAL g_Data[T023_DATABUFSIZE];
static uint8_t g_samples = 0;

//static volatile uint8_t g_DataIsBusy = 0;



//static void t023_rx(const char c)
//{
////	usart_putchar(USART_DEBUG,c);
////	NMEALine_addChar(c);
//}


static uint8_t medianInsert_right(DATAVAL val,uint8_t pos)
{
	DATAVAL v0 = val;
	for(uint8_t idx=pos;idx<T023_DATABUFSIZE;++idx)
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
	const DATAVAL vm = g_Data[T023_MEASUREBUFMID];
	
	//if (vm==0) {
	//return medianInsert_right(val,0);
	//} else
	if (val.levl<vm.levl) {
		for(idx=T023_MEASUREBUFMID;idx>0;--idx) {
			const DATAVAL vr = g_Data[idx];
			const DATAVAL vl = g_Data[idx-1];
			if ((val.levl>=vl.levl) && (val.levl<=vr.levl)) {
				return medianInsert_right(val,idx);
			}
		}
		//New Minimum, it also manage the undef value
		return medianInsert_right(val,0);

		} else if(val.levl>vm.levl)	{
		for(idx=T023_MEASUREBUFMID;idx<T023_DATABUFSIZE-1;++idx) {
			const DATAVAL vl = g_Data[idx];
			const DATAVAL vr = g_Data[idx+1];
			if ((val.levl>=vl.levl) && (val.levl<=vr.levl)) {
				return medianInsert_left(val,idx);
			}
		}
		//New Maximum
		return medianInsert_left(val,T023_DATABUFSIZE-1);
		} else if (val.levl==vm.levl) {
		medianInsert_right(val,T023_MEASUREBUFMID);
		medianInsert_left(val,T023_MEASUREBUFMID);
	}

	return 0;
}



RET_ERROR_CODE t023_init(void)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"T023 Init");

	return AC_ERROR_OK;
}

void t023_enable(void)
{
	//usart_set_rx_interrupt_level(SP336_USART0,USART_INT_LVL_LO);
	//usart_rx_enable(SP336_USART0);

	/* ToDo */
}

void t023_disable(void)
{
	/* ToDo */
}

RET_ERROR_CODE t023_get_data(T023_DATA * const ps)
{
	ps->levl = g_Data[T023_MEASUREBUFMID].levl;
	ps->temp = g_Data[T023_MEASUREBUFMID].temp;
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
	const float levl = interpret_pdu_cdab_float( &(pPDU->data.byte[4]) );
	
	if(temp==-9999.0F) {
		debug_string_1P(NORMAL,PSTR("[WARNING] Invalid value"));
		return;
	} else {
		dv.temp = (int16_t) (temp*10);
		dv.levl = (int16_t) (levl*1000);
	}
	
	
	char szBUF[64];
	sprintf_P(szBUF,PSTR(" (%d , %d)\r\n"),dv.levl,dv.temp);
	debug_string(NORMAL,szBUF,RAM_STRING);

	medianInsert(dv);
	g_samples++;
}

RET_ERROR_CODE t023_reset_data(void)
{
	g_samples = 0;
	return AC_ERROR_OK;
}

RET_ERROR_CODE t023_Data2String(const T023_DATA * const st,char * const sz, uint16_t * const len_sz)
{
	const uint16_t samples = st->samples;
	
	uint16_t len = snprintf_P(sz,*len_sz,PSTR("&LL=%u&LT=%u&nSmp=%u"),st->levl,st->temp,samples);
	
	const RET_ERROR_CODE e = (len < *len_sz) ? AC_ERROR_OK : AC_BUFFER_OVERFLOW;
	*len_sz = len;
	return e;
//	return AC_ERROR_OK;
}


#ifdef RMAP_SERVICES

RET_ERROR_CODE t023_Data2String_RMAP(	 uint8_t * const subModule
									,const T023_DATA * const st
									,const uint32_t timeStamp
									,const uint16_t timeWindow
									,char * const szTopic
									,int16_t * const len_szTopic
									,char * const szMessage
									,int16_t * const len_szMessage	)
{
	return AC_ERROR_OK;
}

#endif //RMAP_SERVICES

#define MODULE_INTERFACE_PRIVATE_DATATYPE T023_PRIVATE_DATA

#define MODINST_PARAM_ID MOD_ID_T023_MODBUS
#include "Acronet/datalogger/modinst/single_module_setup.h"
#undef MODINST_PARAM_ID