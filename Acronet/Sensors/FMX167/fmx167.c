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
#include "fmx167.h"

#define FMX167_MEASURES_NUMBER 32
#define FMX167_DATABUFSIZE			17	// Raw measures buffer size. On this measures array statistics are done.
#define FMX167_MEASUREBUFMID        8   //

static int16_t g_Data[FMX167_DATABUFSIZE];
static uint16_t g_samples = 0;

static uint8_t medianInsert_right(uint16_t val,uint8_t pos)
{
	uint16_t v0 = val;
	for(uint8_t idx=pos;idx<FMX167_DATABUFSIZE;++idx)
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
	const uint16_t vm = g_Data[FMX167_MEASUREBUFMID];
	
	//if (vm==0) {
	//return medianInsert_right(val,0);
	//} else
	if (val<vm) {
		for(idx=FMX167_MEASUREBUFMID;idx>0;--idx) {
			const uint16_t vr = g_Data[idx];
			const uint16_t vl = g_Data[idx-1];
			if ((val>=vl) && (val<=vr)) {
				return medianInsert_right(val,idx);
			}
		}
		//New Minimum, it also manage the undef value
		return medianInsert_right(val,0);

		} else if(val>vm)	{
		for(idx=FMX167_MEASUREBUFMID;idx<FMX167_DATABUFSIZE-1;++idx) {
			const uint16_t vl = g_Data[idx];
			const uint16_t vr = g_Data[idx+1];
			if ((val>=vl) && (val<=vr)) {
				return medianInsert_left(val,idx);
			}
		}
		//New Maximum
		return medianInsert_left(val,FMX167_DATABUFSIZE-1);
		} else if (val==vm) {
		medianInsert_right(val,FMX167_MEASUREBUFMID);
		medianInsert_left(val,FMX167_MEASUREBUFMID);
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


RET_ERROR_CODE fmx167_init(void)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"FMX167 Init");
	ADCB.CALL = ReadCalibrationBytes( ADCBCAL0 );
	ADCB.CALH = ReadCalibrationBytes( ADCBCAL1 );
	return AC_ERROR_OK;
}

void fmx167_enable(void)
{
	/* ToDo */
}

void fmx167_disable(void)
{
	/* ToDo */
}

RET_ERROR_CODE fmx167_get_data(FMX167_DATA * const ps)
{
	FMX167_DATA result;
	
	int16_t voltage = (int16_t) (float)g_Data[FMX167_MEASUREBUFMID]/2.048F;
	int16_t voltage_max = (int16_t) (float)g_Data[FMX167_DATABUFSIZE - 1]/2.048F;
	int16_t voltage_min = (int16_t) (float)g_Data[0]/2.048F;
	result.v = voltage;
	result.v_max = voltage_max;
	result.v_min = voltage_min;
	
	result.level = (uint16_t) (float)voltage * .348984009F - 124; // Insert here calibration line
	
	memcpy_ram2ram(ps,&result,sizeof(FMX167_DATA));
	fmx167_reset_data();
	
	return AC_ERROR_OK;
}

static void fmx167_process_sample(void )
{
	struct adc_config adc_conf;
	struct adc_channel_config adcch_conf;
	
	int32_t result_average = 0;
	
	adc_read_configuration(&FMX167_VOLTMETER, &adc_conf);
	adcch_read_configuration(&FMX167_VOLTMETER, FMX167_VOLTMETER_CH, &adcch_conf);
	
	adc_set_conversion_parameters(&adc_conf, ADC_SIGN_ON, ADC_RES_12, ADC_REF_BANDGAP);
	adc_set_conversion_trigger(&adc_conf, ADC_TRIG_MANUAL, 1, 0);
	adc_set_clock_rate(&adc_conf, 64000UL);

	adcch_set_input(&adcch_conf, FMX167_VOLTMETER_PIN_POS, FMX167_VOLTMETER_PIN_NEG, 1);
	adcch_write_configuration(&FMX167_VOLTMETER, FMX167_VOLTMETER_CH, &adcch_conf);
	adc_write_configuration(&FMX167_VOLTMETER, &adc_conf);

	fmx167_enable();
	delay_ms(2);
	
	adc_enable(&FMX167_VOLTMETER);
	adc_start_conversion(&FMX167_VOLTMETER, FMX167_VOLTMETER_CH);
	adc_wait_for_interrupt_flag(&FMX167_VOLTMETER, FMX167_VOLTMETER_CH);
	delay_ms(2);
	for (uint8_t i = 0; i < FMX167_MEASURES_NUMBER; i++)
	{
		adc_start_conversion(&FMX167_VOLTMETER, FMX167_VOLTMETER_CH);
		adc_wait_for_interrupt_flag(&FMX167_VOLTMETER, FMX167_VOLTMETER_CH);
		result_average += adc_get_result(&FMX167_VOLTMETER, FMX167_VOLTMETER_CH) - 4; //4 offset measured when PIN+ and PIN- are both to GND.
	}
	
	adc_disable(&FMX167_VOLTMETER);
	fmx167_disable();

	result_average /= FMX167_MEASURES_NUMBER;
	medianInsert ((int16_t)result_average);
	char szBUF[32];
	snprintf_P(szBUF,sizeof(szBUF),PSTR("ADC RAW = %d\r\n"),result_average);
	debug_string(NORMAL,szBUF,RAM_STRING);
	
}

RET_ERROR_CODE fmx167_reset_data(void)
{
	g_samples = 0;
	return AC_ERROR_OK;
}

RET_ERROR_CODE fmx167_Data2String(const FMX167_DATA * const st,char * const sz, uint16_t * const len_sz)
{
	uint16_t len = snprintf_P(sz,*len_sz,PSTR("&L=%u&V=%u&Vmax=%u&Vmin=%u"),st->level,st->v,st->v_max,st->v_min);
	
	const RET_ERROR_CODE e = (len < *len_sz) ? AC_ERROR_OK : AC_BUFFER_OVERFLOW;
	*len_sz = len;
	return e;
}

bool fmx167_Yield(void)
{
	fmx167_process_sample();
	return false;
}
