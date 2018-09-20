

#ifndef HAL_BOARD_IMPL_H_
#define HAL_BOARD_IMPL_H_

#include "conf_board.h"
#include "Acronet/drivers/PCAL9535A/PCAL9535A.h"
#include "Acronet/drivers/MPL3115A2/MPL3115A2.h"



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

	//Configure the USB VBus sensing PIN
	//	ioport_configure_pin(USB_PROBE_PIN, IOPORT_DIR_INPUT | IOPORT_TOTEM | IOPORT_BOTHEDGES | IOPORT_SRL_ENABLED );

	//Configure the battery voltmeter switch
	ioport_configure_pin(BATTERY_VOLTMETER_SWITCH, IOPORT_DIR_INPUT);

	ioport_configure_pin(IOPORT_CREATE_PIN(PORTD, 1), IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
	
	ioport_configure_pin(SP336_USART0_PIN_TX_SIGNAL, IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
	ioport_configure_pin(SP336_USART0_PIN_RX_SIGNAL, IOPORT_DIR_INPUT);
	ioport_configure_pin(SP336_USART0_PIN_TX_ENABLE, IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);

	ioport_configure_pin(SP336_USART1_PIN_TX_SIGNAL, IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
	ioport_configure_pin(SP336_USART1_PIN_RX_SIGNAL, IOPORT_DIR_INPUT);
	ioport_configure_pin(SP336_USART1_PIN_TX_ENABLE, IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);

	ioport_configure_pin(SP336_USART2_PIN_TX_SIGNAL, IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
	ioport_configure_pin(SP336_USART2_PIN_RX_SIGNAL, IOPORT_DIR_INPUT);
	ioport_configure_pin(SP336_USART2_PIN_TX_ENABLE, IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);

	ioport_configure_pin(SP336_USART3_PIN_TX_SIGNAL, IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
	ioport_configure_pin(SP336_USART3_PIN_RX_SIGNAL, IOPORT_DIR_INPUT);
	ioport_configure_pin(SP336_USART3_PIN_TX_ENABLE, IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);

	// USART options.
	const usart_rs232_options_t USART_SERIAL_OPTIONS = {
		.baudrate = USART_DEBUG_BAUDRATE,
		.charlength = USART_CHAR_LENGTH,
		.paritytype = USART_PARITY,
		.stopbits = USART_STOP_BIT
	};


	//Configure the Debug facilty
	sysclk_enable_module(SYSCLK_PORT_F,PR_USART0_bm);
	usart_init_rs232(USART_DEBUG, &USART_SERIAL_OPTIONS);



}


RET_ERROR_CODE hal_board_get_stats(char * const pSZ, uint16_t * const len)
{
	char buf[128];

	hal_voltmeter_init();
	int val = hal_voltmeter_getValue();
	int i = snprintf_P(buf,sizeof(buf),PSTR("&VBAT=%d"),val);

	uint8_t b = 0;
	if(AC_ERROR_OK != PCAL9535A_Read(0,&b) )
	{
	}

	buf[i++] = '&';
	buf[i++] = 'P';
	buf[i++] = 'W';
	buf[i++] = 'M';
	buf[i++] = '=';
	buf[i++] = (b & 1) ? '1' : '0';
	buf[i++] = (b & 2) ? '1' : '0';
	buf[i++] = (b & 4) ? '1' : '0';

	if(AC_ERROR_OK != PCAL9535A_Read(1,&b) )
	{
	}

	buf[i++] = '&';
	buf[i++] = 'P';
	buf[i++] = 'W';
	buf[i++] = 'S';
	buf[i++] = '=';
	buf[i++] = (b & 128) ? '1' : '0';
	buf[i++] = (b &  64) ? '1' : '0';
	buf[i++] = (b &  32) ? '1' : '0';
	buf[i++] = (b &  16) ? '1' : '0';

	buf[i] = 0;

	val = hal_thermometer_getValue();
	i += snprintf_P(buf+i,sizeof(buf)-i,PSTR("&TCPU=%d"),val);
	

	if(AC_ERROR_OK != MPL3115A2_Write(0x26,0x00))
	{
	}

	if(AC_ERROR_OK != MPL3115A2_Write(0x13,0x07))
	{
	}

	if(AC_ERROR_OK != MPL3115A2_Write(0x26,0x3B))
	{
	}

	uint8_t ptb[8];
	b = 100;
	while(--b) {
		if(AC_ERROR_OK != MPL3115A2_Read(0,ptb, 1) )
		{
		}

		if ((ptb[0] & 8))
		{
			break;
		} else {
			delay_ms(10);
		}
	}

	uint32_t v32 = 0xFFFFFFFF;
	val = -9999;
	
	if(b!=0) {
		if(AC_ERROR_OK != MPL3115A2_Read(1,ptb+1, 5) )
		{
		}
		v32 = ( (((uint32_t)ptb[1]) << 10) | (((uint32_t)ptb[2]) << 2) | ((ptb[3]>>6)&0x02) );
		val = ( ( (((uint16_t)ptb[4])<<8) | ptb[5] ) * 5) >> 7;
	} else {
		if(AC_ERROR_OK != MPL3115A2_Write(0x26,0x00))
		{
		}

		if(AC_ERROR_OK != MPL3115A2_Write(0x26,0x04))
		{
		}
		debug_string(NORMAL,PSTR("MPL3115A2 Reset\r\n"),PGM_STRING);
	}

	i += snprintf_P(buf+i,sizeof(buf)-i,PSTR("&BAR=%lu"),v32);
	i += snprintf_P(buf+i,sizeof(buf)-i,PSTR("&TINT=%d"),val);

	buf[sizeof(buf)-1] = 0;
	
	if (!(i<*len))
	{
		return AC_BUFFER_TOO_SMALL;
	}
	
	strcpy(pSZ,buf);
	*len = i;
	
	return AC_ERROR_OK;
}

bool hal_sim_get_status(void)
{
	return gpio_pin_is_low(GPRS_STATUS);
}

#endif /* HAL_BOARD_IMPL_H_ */