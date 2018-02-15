

#ifndef HAL_BOARD_IMPL_H_
#define HAL_BOARD_IMPL_H_

void hal_board_init(void)
{
	PR.PRPA |= PR_AC_bm | PR_ADC_bm;
	PR.PRPB |= PR_AC_bm | PR_ADC_bm | PR_DAC_bm;

	PORTCFG.MPCMASK = 0xFF;
	PORTA.PIN0CTRL = PORT_OPC_PULLDOWN_gc;

	PORTCFG.MPCMASK = 0xFF;
	PORTB.PIN0CTRL = PORT_OPC_PULLDOWN_gc;

	
	//By default the PORTF UART has been used for debugging purposes
	//Since on the PCB uses the UART pin for GPRS command
	//we remap the PORTF UART
	//so we can have the USART0 on PIN6 and PIN7
	PORTF.REMAP |= 16;
	
	//Configure the UART on PortF
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTF, 7), IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTF, 6), IOPORT_DIR_INPUT);
	//ioport_configure_pin(IOPORT_CREATE_PIN(PORTF, 6), IOPORT_DIR_INPUT | IOPORT_INV_ENABLED );

	ioport_configure_pin(IOPORT_CREATE_PIN(PORTB, 7), IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);


	//Configure GPRS_UART
	ioport_configure_pin(GPRS_UART_TX, IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
	ioport_configure_pin(GPRS_UART_RX, IOPORT_DIR_INPUT);

	//Configure the status LED switch PIN
	ioport_configure_pin(STATUS_LED_PIN, IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);

	//Configure the GPRS Switch PIN
	ioport_configure_pin(GPRS_SWITCH, IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);

	//Configure the GPRS Status sense PIN
	ioport_configure_pin(GPRS_STATUS, IOPORT_DIR_INPUT);

	//Configure the Raingauge Switch PINs
	ioport_configure_pin(RAINGAUGE1_SWITCH, IOPORT_DIR_INPUT | IOPORT_TOTEM | IOPORT_RISING );
	ioport_configure_pin(RAINGAUGE2_SWITCH, IOPORT_DIR_INPUT | IOPORT_TOTEM | IOPORT_RISING );

	//Configure the USB VBus sensing PIN
	//	ioport_configure_pin(USB_PROBE_PIN, IOPORT_DIR_INPUT | IOPORT_TOTEM | IOPORT_BOTHEDGES | IOPORT_SRL_ENABLED );

	//Configure the battery voltmeter switch
	ioport_configure_pin(BATTERY_VOLTMETER_SWITCH, IOPORT_DIR_INPUT);

	ioport_configure_pin(IOPORT_CREATE_PIN(PORTD, 1), IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
	
	ioport_configure_pin(SP336_USART0_PIN_TX_SIGNAL, IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
	ioport_configure_pin(SP336_USART0_PIN_RX_SIGNAL, IOPORT_DIR_INPUT);

	ioport_configure_pin(SP336_USART1_PIN_TX_SIGNAL, IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
	ioport_configure_pin(SP336_USART1_PIN_RX_SIGNAL, IOPORT_DIR_INPUT);

}




#endif /* HAL_BOARD_IMPL_H_ */