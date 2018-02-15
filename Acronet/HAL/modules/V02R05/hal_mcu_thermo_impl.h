
#ifndef HAL_MCU_THERMO_IMPL_H_
#define HAL_MCU_THERMO_IMPL_H_



uint16_t hal_thermometer_getValue(void)
{
	struct adc_config adc_conf;
	struct adc_channel_config adcch_conf;

	adc_read_configuration(&ADCA, &adc_conf);
	adcch_read_configuration(&ADCA, ADC_CH0, &adcch_conf);

	adc_set_conversion_parameters(&adc_conf, ADC_SIGN_OFF, ADC_RES_12, ADC_REF_BANDGAP);
	adc_set_clock_rate(&adc_conf, 200000UL);
	adc_set_conversion_trigger(&adc_conf, ADC_TRIG_MANUAL, 1, 0);
	adc_enable_internal_input(&adc_conf,ADC_INT_TEMPSENSE);

	adc_write_configuration(&ADCA, &adc_conf);


	adcch_set_input(&adcch_conf, ADCCH_POS_TEMPSENSE, ADCCH_NEG_NONE, 1);
	adcch_write_configuration(&ADCA, ADC_CH0, &adcch_conf);

	// Get measurement for 85 degrees C (358 kelvin) from calibration data.
	uint16_t tempsense = adc_get_calibration_data(ADC_CAL_TEMPSENSE);

	delay_ms(2);

	adc_enable(&ADCA);
	adc_start_conversion(&ADCA, ADC_CH0);
	adc_wait_for_interrupt_flag(&ADCA, ADC_CH0);
	const int16_t result = adc_get_result(&ADCA, ADC_CH0);
	adc_disable(&ADCA);

	uint32_t temperature = (uint32_t)result * 358;
	temperature /= tempsense;

	
	//char szBUF[32];
	//snprintf_P(szBUF,sizeof(szBUF), PSTR("temperature: %d\r\n"),temperature);
	//debug_string(NORMAL,szBUF,RAM_STRING);
	
	return temperature;
}


#endif /* HAL_MCU_THERMO_IMPL_H_ */