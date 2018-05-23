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
#include "Acronet/Sensors/DELTAOHM/HD3910/hd3910.h"
#include "Acronet/drivers/SP336/SP336.h"
#include "Acronet/channels/MODBUS_RTU/mb_crc.h"
#include "Acronet/channels/MODBUS_RTU/master_rtu.h"


#define HD3910_MEASURES_NUMBER	32
#define HD3910_DATABUFSIZE		17	// Raw measures buffer size. On this measures array statistics are done.
#define HD3910_MEASUREBUFMID		8   //

typedef struct
{
	int16_t wvc;
	int16_t temp;
} HD3910_PRIVATE_SAMPLE;

typedef struct
{
	MBUS_PDU pdu;

	HD3910_PRIVATE_SAMPLE g_Data[HD3910_DATABUFSIZE];
	uint8_t g_samples;

	volatile uint8_t g_DataIsBusy;
} HD3910_PRIVATE_DATA;



static uint8_t medianInsert_right(HD3910_PRIVATE_DATA * const pSelf,HD3910_PRIVATE_SAMPLE val,uint8_t pos)
{
	HD3910_PRIVATE_SAMPLE v0 = val;
	for(uint8_t idx=pos;idx<HD3910_DATABUFSIZE;++idx)
	{
		const HD3910_PRIVATE_SAMPLE a = pSelf->g_Data[idx];
		const HD3910_PRIVATE_SAMPLE v1 = (a.wvc==0)?v0:a;
		pSelf->g_Data[idx] = v0;
		v0 = v1;
	}
	return 0;
}

static uint8_t medianInsert_left(HD3910_PRIVATE_DATA * const pSelf,HD3910_PRIVATE_SAMPLE val,uint8_t pos)
{
	HD3910_PRIVATE_SAMPLE v0 = val;
	uint8_t idx=pos;
	do 
	{
		const HD3910_PRIVATE_SAMPLE a = pSelf->g_Data[idx];
		const HD3910_PRIVATE_SAMPLE v1 = (a.wvc==0)?v0:a;
		pSelf->g_Data[idx] = v0;
		v0 = v1;
	} while (idx-- != 0);
	
	return 0;
}

static uint8_t medianInsert(HD3910_PRIVATE_DATA * const pSelf,const HD3910_PRIVATE_SAMPLE val)
{
	uint8_t idx;
	const HD3910_PRIVATE_SAMPLE vm = pSelf->g_Data[HD3910_MEASUREBUFMID];
	
	//if (vm==0) {
	//return medianInsert_right(val,0);
	//} else
	if (val.wvc<vm.wvc) {
		for(idx=HD3910_MEASUREBUFMID;idx>0;--idx) {
			const HD3910_PRIVATE_SAMPLE vr = pSelf->g_Data[idx];
			const HD3910_PRIVATE_SAMPLE vl = pSelf->g_Data[idx-1];
			if ((val.wvc>=vl.wvc) && (val.wvc<=vr.wvc)) {
				return medianInsert_right(pSelf,val,idx);
			}
		}
		//New Minimum, it also manage the undef value
		return medianInsert_right(pSelf,val,0);

		} else if(val.wvc>vm.wvc)	{
		for(idx=HD3910_MEASUREBUFMID;idx<HD3910_DATABUFSIZE-1;++idx) {
			const HD3910_PRIVATE_SAMPLE vl = pSelf->g_Data[idx];
			const HD3910_PRIVATE_SAMPLE vr = pSelf->g_Data[idx+1];
			if ((val.wvc>=vl.wvc) && (val.wvc<=vr.wvc)) {
				return medianInsert_left(pSelf,val,idx);
			}
		}
		//New Maximum
		return medianInsert_left(pSelf,val,HD3910_DATABUFSIZE-1);
		} else if (val.wvc==vm.wvc) {
		medianInsert_right(pSelf,val,HD3910_MEASUREBUFMID);
		medianInsert_left(pSelf,val,HD3910_MEASUREBUFMID);
	}

	return 0;
}



static RET_ERROR_CODE hd3910_init(HD3910_PRIVATE_DATA * const pSelf)
{
//	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"HD3910 Init");

	pSelf->g_samples = 0;
	MBUS_PDU_reset( &pSelf->pdu );

	return AC_ERROR_OK;
}

/*
static void hd3910_enable(void)
{
	//usart_set_rx_interrupt_level(SP336_USART0,USART_INT_LVL_LO);
	//usart_rx_enable(SP336_USART0);

}

static void hd3910_disable(void)
{
}
*/

static RET_ERROR_CODE hd3910_get_data(HD3910_PRIVATE_DATA * const pSelf,HD3910_DATA * const ps)
{
	ps->wvc  = pSelf->g_Data[HD3910_MEASUREBUFMID].wvc;
	ps->temp = pSelf->g_Data[HD3910_MEASUREBUFMID].temp;
	ps->samples = pSelf->g_samples;
	
	
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

static void interpret_pdu(HD3910_PRIVATE_DATA * const pSelf)
{
	if (pSelf->g_samples > 254) {
		return;
	}

	HD3910_PRIVATE_SAMPLE dv;
							
	const float temp = interpret_pdu_ab_int16( &(pSelf->pdu.data.byte[6]) );
	const float wvc  = interpret_pdu_ab_int16( &(pSelf->pdu.data.byte[2]) );
	
	if(wvc==-9999.0F) {
		debug_string_1P(NORMAL,PSTR("[WARNING] Invalid value"));
		return;
	} else {
		dv.wvc  = wvc;
		dv.temp = temp;
	}
	
	
	//char szBUF[64];
	//sprintf_P(szBUF,PSTR(" (%d , %d)\r\n"),dv.wvc,dv.temp);
	//debug_string(NORMAL,szBUF,RAM_STRING);

	medianInsert(pSelf,dv);
	pSelf->g_samples++;
}

static RET_ERROR_CODE hd3910_reset_data(HD3910_PRIVATE_DATA * const pSelf)
{
	pSelf->g_samples = 0;
	MBUS_PDU_reset(&(pSelf->pdu));
	
	return AC_ERROR_OK;
}

RET_ERROR_CODE hd3910_Data2String(const HD3910_DATA * const st,char * const sz, size_t * const len_sz)
{
	const uint16_t samples = st->samples;
	
	size_t len = snprintf_P(sz,*len_sz,PSTR("&wvc=%u&wvt=%u&nSmp=%u"),st->wvc,st->temp,samples);
	
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

#define MODULE_INTERFACE_PRIVATE_DATATYPE HD3910_PRIVATE_DATA


#define MODINST_PARAM_ID MOD_ID_HD3910_MODBUS
#include "Acronet/datalogger/modinst/module_interface_definition.h"
#undef MODINST_PARAM_ID