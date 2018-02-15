
#ifndef HAL_VOLTMETER_IMPL_H_
#define HAL_VOLTMETER_IMPL_H_



// Voltmeter switch to GND
#define VOLTMETER_SWITCH_TURN_ON()		ioport_configure_pin(BATTERY_VOLTMETER_SWITCH, IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
// High impedance on voltemeter switch
#define VOLTMETER_SWITCH_TURN_OFF()		ioport_configure_pin(BATTERY_VOLTMETER_SWITCH, IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);

#define VOLTMETER_SCALE_FACTOR			0.427246094F

/**
 * \brief Initialize the ADCA by reading calibration data stored in the MCU
 *
 */
void hal_voltmeter_init(void)
{
	ADCA.CALL = adc_get_calibration_data( ADCACAL0 );
	ADCA.CALH = adc_get_calibration_data( ADCACAL1 );
}

/**
 * \brief Turn On the voltmeter
 *
 */
static void voltmeter_turn_on(void)
{
	VOLTMETER_SWITCH_TURN_ON()
	// Switch off pull-up resistor on ADC port
	PORTA.PIN1CTRL = PORT_OPC_TOTEM_gc;
}

/**
 * \brief Turn Off the voltmeter
 *
 */
static void voltmeter_turn_off(void)
{
	VOLTMETER_SWITCH_TURN_OFF()
	// Turn on pull-up resistor on ADC port
	PORTA.PIN1CTRL = PORT_OPC_PULLUP_gc;
}


/**
 * \brief Read the board input voltage 
 *
 * the return value is in cents of volt
 *
 */
uint16_t hal_voltmeter_getValue(void)
{
	
	struct adc_config adc_conf;
	struct adc_channel_config adcch_conf;
	
	adc_read_configuration(&BATTERY_VOLTMETER, &adc_conf);
	adcch_read_configuration(&BATTERY_VOLTMETER, BATTERY_VOLTMETER_CH, &adcch_conf);

	adc_set_conversion_parameters(&adc_conf, ADC_SIGN_OFF, ADC_RES_12, ADC_REF_VCC);
	adc_set_conversion_trigger(&adc_conf, ADC_TRIG_MANUAL, 1, 0);
	adc_set_clock_rate(&adc_conf, 64000UL);

	adcch_set_input(&adcch_conf, BATTERY_VOLTMETER_PIN, ADCCH_NEG_NONE, 1);
	adcch_write_configuration(&BATTERY_VOLTMETER, BATTERY_VOLTMETER_CH, &adcch_conf);
	adc_write_configuration(&BATTERY_VOLTMETER, &adc_conf);

	voltmeter_turn_on();
	delay_ms(2);
	
	adc_enable(&BATTERY_VOLTMETER);
	adc_start_conversion(&BATTERY_VOLTMETER, BATTERY_VOLTMETER_CH);
	adc_wait_for_interrupt_flag(&BATTERY_VOLTMETER, BATTERY_VOLTMETER_CH);
	adc_start_conversion(&BATTERY_VOLTMETER, BATTERY_VOLTMETER_CH);
	adc_wait_for_interrupt_flag(&BATTERY_VOLTMETER, BATTERY_VOLTMETER_CH);
	const uint16_t result = adc_get_result(&BATTERY_VOLTMETER, BATTERY_VOLTMETER_CH);
	adc_disable(&BATTERY_VOLTMETER);
	voltmeter_turn_off();
	
	//char szBUF[32];
	//snprintf_P(szBUF,sizeof(szBUF),PSTR("ADC: %d\r\n"),result);
	//debug_string(VERBOSE,szBUF,RAM_STRING);
	
	uint16_t batval = (float)((result-190)) * VOLTMETER_SCALE_FACTOR;
	return batval;
}

#endif /* HAL_VOLTMETER_IMPL_H_ */