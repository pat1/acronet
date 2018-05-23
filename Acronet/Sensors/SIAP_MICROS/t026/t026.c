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
#include "Acronet/channels/MODBUS_RTU/mb_crc.h"
#include "Acronet/channels/MODBUS_RTU/master_rtu.h"




#define T026_DATABUFSIZE		17	// Raw measures buffer size. On this measures array statistics are done.
#define T026_MEASUREBUFMID		8   //

typedef struct
{
	int16_t temp;
	int16_t rh;
} T026_SAMPLE;



typedef struct
{
	MBUS_PDU pdu;

	T026_SAMPLE sample[T026_DATABUFSIZE];
	uint8_t numSamples;

//	volatile uint8_t g_DataIsBusy;
} T026_PRIVATE_DATA;


static uint8_t medianInsert_right(T026_PRIVATE_DATA * const pSelf,T026_SAMPLE val,uint8_t pos)
{
	T026_SAMPLE v0 = val;
	for(uint8_t idx=pos;idx<T026_DATABUFSIZE;++idx)
	{
		const T026_SAMPLE a = pSelf->sample[idx];
		const T026_SAMPLE v1 = (a.temp==0)?v0:a;
		pSelf->sample[idx] = v0;
		v0 = v1;
	}
	return 0;
}

static uint8_t medianInsert_left(T026_PRIVATE_DATA * const pSelf,T026_SAMPLE val,uint8_t pos)
{
	T026_SAMPLE v0 = val;
	uint8_t idx=pos;
	do 
	{
		const T026_SAMPLE a = pSelf->sample[idx];
		const T026_SAMPLE v1 = (a.temp==0)?v0:a;
		pSelf->sample[idx] = v0;
		v0 = v1;
	} while (idx-- != 0);
	
	return 0;
}

static uint8_t medianInsert(T026_PRIVATE_DATA * const pSelf, const T026_SAMPLE val)
{
	uint8_t idx;
	const T026_SAMPLE vm = pSelf->sample[T026_MEASUREBUFMID];
	
	//if (vm==0) {
	//return medianInsert_right(val,0);
	//} else
	if (val.temp<vm.temp) {
		for(idx=T026_MEASUREBUFMID;idx>0;--idx) {
			const T026_SAMPLE vr = pSelf->sample[idx];
			const T026_SAMPLE vl = pSelf->sample[idx-1];
			if ((val.temp>=vl.temp) && (val.temp<=vr.temp)) {
				return medianInsert_right(pSelf,val,idx);
			}
		}
		//New Minimum, it also manage the undef value
		return medianInsert_right(pSelf,val,0);

		} else if(val.temp>vm.temp)	{
		for(idx=T026_MEASUREBUFMID;idx<T026_DATABUFSIZE-1;++idx) {
			const T026_SAMPLE vl = pSelf->sample[idx];
			const T026_SAMPLE vr = pSelf->sample[idx+1];
			if ((val.temp>=vl.temp) && (val.temp<=vr.temp)) {
				return medianInsert_left(pSelf,val,idx);
			}
		}
		//New Maximum
		return medianInsert_left(pSelf,val,T026_DATABUFSIZE-1);
		} else if (val.temp==vm.temp) {
		medianInsert_right(pSelf,val,T026_MEASUREBUFMID);
		medianInsert_left(pSelf,val,T026_MEASUREBUFMID);
	}

	return 0;
}



static RET_ERROR_CODE t026_init( T026_PRIVATE_DATA * const pSelf )
{
//	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"T026 Init");
	
	pSelf->numSamples = 0;
	MBUS_PDU_reset(&(pSelf->pdu));

	return AC_ERROR_OK;
}


/*
void t026_enable(void)
{
	//usart_set_rx_interrupt_level(SP336_USART0,USART_INT_LVL_LO);
	//usart_rx_enable(SP336_USART0);

}

void t026_disable(void)
{
}
*/

static RET_ERROR_CODE t026_get_data(T026_PRIVATE_DATA * const pSelf,T026_DATA * const ps)
{
	ps->temp	= pSelf->sample[T026_MEASUREBUFMID].temp;
	ps->rh		= pSelf->sample[T026_MEASUREBUFMID].rh;
	ps->samples = pSelf->numSamples;
	
	
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

static void interpret_pdu(T026_PRIVATE_DATA * const pSelf)
{
	if (pSelf->numSamples > 254) {
		return;
	}

	T026_SAMPLE dv;
							
	const float temp = interpret_pdu_cdab_float( &(pSelf->pdu.data.byte[0]) );
	const float rh = interpret_pdu_cdab_float( &(pSelf->pdu.data.byte[4]) );
	
	if(temp==-9999.0F) {
		debug_string_1P(NORMAL,PSTR("[WARNING] Invalid value"));
		return;
	} else {
		dv.temp = (int16_t) (temp*10);
		dv.rh   = (int16_t) (rh*10);
	}
	
	
	//char szBUF[64];
	//sprintf_P(szBUF,PSTR(" (%d , %d)\r\n"),dv.temp,dv.rh);
	//debug_string(NORMAL,szBUF,RAM_STRING);

	medianInsert(pSelf,dv);
	pSelf->numSamples++;
}


/*
RET_ERROR_CODE t026_reset_data(T026_PRIVATE_DATA * const pSelf)
{
	pSelf->numSamples = 0;
	return AC_ERROR_OK;
}
*/
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

#define MODULE_INTERFACE_PRIVATE_DATATYPE T026_PRIVATE_DATA

#define MODINST_PARAM_ID MOD_ID_T026_MODBUS
#include "Acronet/datalogger/modinst/module_interface_definition.h"
#undef MODINST_PARAM_ID
