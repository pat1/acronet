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
#include "conf_board.h"
#include "Acronet/globals.h"
#include "vp61.h"

#define VP61_MEASURES_NUMBER 32
#define VP61_DATABUFSIZE			17	// Raw measures buffer size. On this measures array statistics are done.
#define VP61_MEASUREBUFMID        8   //

static int16_t g_Data[VP61_DATABUFSIZE];
static uint8_t g_samples = 0;

static volatile uint8_t g_DataIsBusy = 0;

static uint8_t medianInsert_right(uint16_t val,uint8_t pos)
{
	uint16_t v0 = val;
	for(uint8_t idx=pos;idx<VP61_DATABUFSIZE;++idx)
	{
		const uint16_t a = g_Data[idx];
		const uint16_t v1 = (a==0)?v0:a;
		g_Data[idx] = v0;
		v0 = v1;
	}
	return 0;
}

static uint8_t medianInsert_left(uint16_t val,uint8_t pos)
{
	uint16_t v0 = val;
	uint8_t idx=pos;
	do 
	{
		const uint16_t a = g_Data[idx];
		const uint16_t v1 = (a==0)?v0:a;
		g_Data[idx] = v0;
		v0 = v1;
	} while (idx-- != 0);
	
	return 0;
}

static uint8_t medianInsert(const uint16_t val)
{
	uint8_t idx;
	const uint16_t vm = g_Data[VP61_MEASUREBUFMID];
	
	//if (vm==0) {
	//return medianInsert_right(val,0);
	//} else
	if (val<vm) {
		for(idx=VP61_MEASUREBUFMID;idx>0;--idx) {
			const uint16_t vr = g_Data[idx];
			const uint16_t vl = g_Data[idx-1];
			if ((val>=vl) && (val<=vr)) {
				return medianInsert_right(val,idx);
			}
		}
		//New Minimum, it also manage the undef value
		return medianInsert_right(val,0);

		} else if(val>vm)	{
		for(idx=VP61_MEASUREBUFMID;idx<VP61_DATABUFSIZE-1;++idx) {
			const uint16_t vl = g_Data[idx];
			const uint16_t vr = g_Data[idx+1];
			if ((val>=vl) && (val<=vr)) {
				return medianInsert_left(val,idx);
			}
		}
		//New Maximum
		return medianInsert_left(val,VP61_DATABUFSIZE-1);
		} else if (val==vm) {
		medianInsert_right(val,VP61_MEASUREBUFMID);
		medianInsert_left(val,VP61_MEASUREBUFMID);
	}

	return 0;
}

static uint8_t ReadCalibrationBytes( uint8_t index )
{
	uint8_t result;

	/* Load the NVM Command register to read the calibration row. */
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
	result = pgm_read_byte(index);

	/* Clean up NVM Command register. */
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;

	return( result );
}


RET_ERROR_CODE vp61_init(void)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"VP61 Init");
	ADCB.CALL = ReadCalibrationBytes( ADCBCAL0 );
	ADCB.CALH = ReadCalibrationBytes( ADCBCAL1 );
	return AC_ERROR_OK;
}

void vp61_enable(void)
{
	/* ToDo */
}

void vp61_disable(void)
{
	/* ToDo */
}

RET_ERROR_CODE vp61_get_data(VP61_DATA * const ps)
{
	
	const float v1 = (float)g_Data[VP61_MEASUREBUFMID];
	const float v2 = (float)g_Data[VP61_DATABUFSIZE - 1];
	const float v3 = (float)g_Data[0];
	const uint8_t s = g_samples;
	
	
	ps->v = (int16_t) (v1/2.048F*2.8F/1.6F);
	ps->v_max = (int16_t) (v2/2.048F*2.8F/1.6F);
	ps->v_min = (int16_t) (v3/2.048F*2.8F/1.6F);
	ps->samples = s;
	
	return AC_ERROR_OK;
}

void vp61_process_sample(void )
{
	struct adc_config adc_conf;
	struct adc_channel_config adcch_conf;
	
	int32_t result_average = 0;
	
	adc_read_configuration(&VP61_VOLTMETER, &adc_conf);
	adcch_read_configuration(&VP61_VOLTMETER, VP61_VOLTMETER_CH, &adcch_conf);
	
	adc_set_conversion_parameters(&adc_conf, ADC_SIGN_ON, ADC_RES_12, ADC_REF_VCC);
	adc_set_conversion_trigger(&adc_conf, ADC_TRIG_MANUAL, 1, 0);
	adc_set_clock_rate(&adc_conf, 64000UL);

	adcch_set_input(&adcch_conf, VP61_VOLTMETER_PIN_POS, VP61_VOLTMETER_PIN_NEG, 1);
	adcch_write_configuration(&VP61_VOLTMETER, VP61_VOLTMETER_CH, &adcch_conf);
	adc_write_configuration(&VP61_VOLTMETER, &adc_conf);

	vp61_enable();

	adc_enable(&VP61_VOLTMETER);
	adc_start_conversion(&VP61_VOLTMETER, VP61_VOLTMETER_CH);
	adc_wait_for_interrupt_flag(&VP61_VOLTMETER, VP61_VOLTMETER_CH);

	for (uint8_t i = 0; i < VP61_MEASURES_NUMBER; i++)
	{
		adc_start_conversion(&VP61_VOLTMETER, VP61_VOLTMETER_CH);
		adc_wait_for_interrupt_flag(&VP61_VOLTMETER, VP61_VOLTMETER_CH);
		result_average += adc_get_result(&VP61_VOLTMETER, VP61_VOLTMETER_CH) - 4; //4 offset measured when PIN+ and PIN- are both to GND.
	}
	
	adc_disable(&VP61_VOLTMETER);
	vp61_disable();

	result_average /= VP61_MEASURES_NUMBER;
	medianInsert ((int16_t)result_average);
	
	if (g_samples < 255) {
		g_samples++;
	}
	
	//char szBUF[64];
	//int16_t voltage = (int16_t) (float) (result_average)*1.75F/2.048F;///2048*2800/1.6F;
	//snprintf_P(szBUF,sizeof(szBUF),PSTR("ADC RAW = %d - Voltage = %d mV\r\n"),(int16_t) result_average, voltage);
	//debug_string(NORMAL,szBUF,RAM_STRING);
	
}

RET_ERROR_CODE vp61_reset_data(void)
{
	g_samples = 0;
	return AC_ERROR_OK;
}

RET_ERROR_CODE vp61_Data2String(const VP61_DATA * const st,char * const sz, uint16_t * const len_sz)
{
	const uint16_t samples = st->samples;
	
	uint16_t len = snprintf_P(sz,*len_sz,PSTR("&V=%u&Vmax=%u&Vmin=%u&nSmp=%u"),st->v,st->v_max,st->v_min,samples);
	
	const RET_ERROR_CODE e = (len < *len_sz) ? AC_ERROR_OK : AC_BUFFER_OVERFLOW;
	*len_sz = len;
	return e;
}

//bool vp61_Yield(void)
//{
	//vp61_process_sample();
	//return false;
//}

#ifdef RMAP_SERVICES


RET_ERROR_CODE vp61_Data2String_RMAP(	 uint8_t * const subModule
									,const VP61_DATA * const st
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

