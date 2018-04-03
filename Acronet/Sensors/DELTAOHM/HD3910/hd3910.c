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
#include "hd3910.h"
#include "Acronet/drivers/SP336/SP336.h"
#include "Acronet/services/MODBUS_RTU/mb_crc.h"
#include "Acronet/services/MODBUS_RTU/master_rtu.h"



#if !defined(HD3910_MBUS_CH)
#pragma message "[!!! PROJECT WARNING !!!] in file " __FILE__\
" SYMBOL HD3910_MBUS_CH not defined, using default just to compile, your project may not work as aspected"
#define HD3910_MBUS_CH	0
#endif


#define HD3910_MEASURES_NUMBER	32
#define HD3910_DATABUFSIZE		17	// Raw measures buffer size. On this measures array statistics are done.
#define HD3910_MEASUREBUFMID		8   //

typedef struct  
{
	int16_t wvc;
	int16_t temp;
} DATAVAL;

static DATAVAL g_Data[HD3910_DATABUFSIZE];
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
	for(uint8_t idx=pos;idx<HD3910_DATABUFSIZE;++idx)
	{
		const DATAVAL a = g_Data[idx];
		const DATAVAL v1 = (a.wvc==0)?v0:a;
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
		const DATAVAL v1 = (a.wvc==0)?v0:a;
		g_Data[idx] = v0;
		v0 = v1;
	} while (idx-- != 0);
	
	return 0;
}

static uint8_t medianInsert(const DATAVAL val)
{
	uint8_t idx;
	const DATAVAL vm = g_Data[HD3910_MEASUREBUFMID];
	
	//if (vm==0) {
	//return medianInsert_right(val,0);
	//} else
	if (val.wvc<vm.wvc) {
		for(idx=HD3910_MEASUREBUFMID;idx>0;--idx) {
			const DATAVAL vr = g_Data[idx];
			const DATAVAL vl = g_Data[idx-1];
			if ((val.wvc>=vl.wvc) && (val.wvc<=vr.wvc)) {
				return medianInsert_right(val,idx);
			}
		}
		//New Minimum, it also manage the undef value
		return medianInsert_right(val,0);

		} else if(val.wvc>vm.wvc)	{
		for(idx=HD3910_MEASUREBUFMID;idx<HD3910_DATABUFSIZE-1;++idx) {
			const DATAVAL vl = g_Data[idx];
			const DATAVAL vr = g_Data[idx+1];
			if ((val.wvc>=vl.wvc) && (val.wvc<=vr.wvc)) {
				return medianInsert_left(val,idx);
			}
		}
		//New Maximum
		return medianInsert_left(val,HD3910_DATABUFSIZE-1);
		} else if (val.wvc==vm.wvc) {
		medianInsert_right(val,HD3910_MEASUREBUFMID);
		medianInsert_left(val,HD3910_MEASUREBUFMID);
	}

	return 0;
}



RET_ERROR_CODE hd3910_init(void)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"T023 Init");

	return AC_ERROR_OK;
}

void hd3910_enable(void)
{
	//usart_set_rx_interrupt_level(SP336_USART0,USART_INT_LVL_LO);
	//usart_rx_enable(SP336_USART0);

	/* ToDo */
}

void hd3910_disable(void)
{
	/* ToDo */
}

RET_ERROR_CODE hd3910_get_data(HD3910_DATA * const ps)
{
	ps->wvc  = g_Data[HD3910_MEASUREBUFMID].wvc;
	ps->temp = g_Data[HD3910_MEASUREBUFMID].temp;
	ps->samples = g_samples;
	
	
	return AC_ERROR_OK;
}

static int16_t interpret_pdu_ab_int16(uint8_t * const pData)
{
	typedef union {
		int16_t ival;
		uint8_t bval[2];
	} VV;
	
	const register VV v = {	.bval[1] = pData[0] ,
							.bval[0] = pData[1] };

	return v.ival;	
}

static void interpret_pdu(MBUS_PDU * const pPDU)
{
	if (g_samples > 254) {
		return;
	}

	DATAVAL dv;
							
	const float temp = interpret_pdu_ab_int16( &(pPDU->data.byte[6]) );
	const float wvc  = interpret_pdu_ab_int16( &(pPDU->data.byte[2]) );
	
	if(wvc==-9999.0F) {
		debug_string_1P(NORMAL,PSTR("[WARNING] Invalid value"));
		return;
	} else {
		dv.wvc  = wvc;
		dv.temp = temp;
	}
	
	
	char szBUF[64];
	sprintf_P(szBUF,PSTR(" (%d , %d)\r\n"),dv.wvc,dv.temp);
	debug_string(NORMAL,szBUF,RAM_STRING);

	medianInsert(dv);
	g_samples++;
}

bool hd3910_Yield( void )
{
	static MBUS_PDU g_mbp = {0};

	while(! MBUS_IS_EMPTY(HD3910_MBUS_CH) )
	{
		const uint8_t b = MBUS_GET_BYTE(HD3910_MBUS_CH);
		if ( MBUS_STATUS_END == MBUS_BUILD_DGRAM(HD3910_MBUS_CH,&g_mbp,b) )
		{
			usart_putchar(USART_DEBUG,'y');

			const uint16_t crcc = MBUS_GET_CRC(HD3910_MBUS_CH);
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
			MBUS_RELEASE(HD3910_MBUS_CH);
		}
//		usart_putchar(USART_DEBUG,'Y');
				
		return true;
	}
	return false;
}

void hd3910_periodic(void)
{
	static const __flash uint8_t cmd[] = {0x1B,0x04,0x00,0x00,0x00,0x04,0xF3,0xF3};

	if ( AC_ERROR_OK != MBUS_LOCK(HD3910_MBUS_CH) )
	{
		return;
	}

	usart_putchar(USART_DEBUG,'p');
	uint8_t buf[16];
	memcpy_P(buf,cmd,8);
	MBUS_ISSUE_CMD(HD3910_MBUS_CH,buf,8);
}

RET_ERROR_CODE hd3910_reset_data(void)
{
	g_samples = 0;
	return AC_ERROR_OK;
}

RET_ERROR_CODE hd3910_Data2String(const HD3910_DATA * const st,char * const sz, uint16_t * const len_sz)
{
	const uint16_t samples = st->samples;
	
	uint16_t len = snprintf_P(sz,*len_sz,PSTR("&wvc=%u&wvt=%u&nSmp=%u"),st->wvc,st->temp,samples);
	
	const RET_ERROR_CODE e = (len < *len_sz) ? AC_ERROR_OK : AC_BUFFER_OVERFLOW;
	*len_sz = len;
	return e;
//	return AC_ERROR_OK;
}


#ifdef RMAP_SERVICES

RET_ERROR_CODE hd3910_Data2String_RMAP(	 uint8_t * const subModule
										,const HD3910_DATA * const st
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

