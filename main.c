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



#include "asf.h"
#include <stdio.h>
#include "calendar.h"

#include <conf_usart_serial.h>
#include "config/conf_usart_serial.h"

#include "Acronet/globals.h"
#include "Acronet/datalogger/datalogger.h"

#include "Acronet/drivers/StatusLED/status_led.h"
#include "Acronet/drivers/PowerSwitch/powerswitch.h"
#include "Acronet/drivers/SIM/sim900.h"

#include "Acronet/services/taskman/taskman.h"
#include "Acronet/services/config/config.h"

#include "Acronet/drivers/SP336/SP336.h"

#include "Acronet/Sensors/raingauge/pulse_raingauge.h"
#include "Acronet/Sensors/LB150/LB150.h"
#include "Acronet/Sensors/MBXXXX/MBXXXX.h"

#include "Acronet/services/CAP/cap_common.h"


#include "Acronet/setup.h"
#include "Acronet/services/DB/DB.h"


static void	simple_logger_test(void);


int main (void)
{

	//Setting this variable we can decide the amount of info to send
	//at the Debug facility. Compiling the firmware in Release may
	//disable it
	g_log_verbosity = VERBOSE;


	/* Initialize the board.
	 * The board-specific conf_board.h file contains the configuration of
	 * the board initialization.
	 */
	sleepmgr_init();
	sysclk_init();

	board_init();
	//VBusMon_init();
	pmic_init(); 
	pmic_set_scheduling(PMIC_SCH_ROUND_ROBIN);
	pmic_set_vector_location(PMIC_VEC_APPLICATION);

	powerSwitch_init();

	//Enables the Event module
	sysclk_enable_module(SYSCLK_PORT_GEN, SYSCLK_EVSYS); 



	//After that the Debug facilty has been configured
	//report the Power On event on it
	debug_string_1P(NORMAL,PSTR ("\r\n\r\n****************** POWER ON ******************"));


	if(g_log_verbosity>=NORMAL) {
		debug_string(NORMAL,PSTR ("MCU Revision : "),PGM_STRING);
		usart_putchar(USART_DEBUG,'A'+(MCU.REVID & 0x0F));
		debug_string(NORMAL,g_szCRLF,true);
	}

	////Initialize the RTC 
	////// Workaround for known issue: Enable RTC32 sysclk
	//sysclk_enable_module(SYSCLK_PORT_GEN, SYSCLK_RTC);
	//while (RTC32.SYNCCTRL & RTC32_SYNCBUSY_bm) {
		////// Wait for RTC32 sysclk to become stable
	//}
	//
	////sysclk_rtcsrc_enable(SYSCLK_RTCSRC_TOSC32);
//
//
	//rtc_init();
	
	hal_rtc_init();


	debug_string(NORMAL,PSTR("Enabling oscillator for DFLL\r\n"),true);


 	irqflags_t flags = cpu_irq_save();
 	OSC.XOSCCTRL |= 0x02;
 	OSC.CTRL |= 0x08;
	do {} while (!(OSC.STATUS & 0x08));
 	cpu_irq_restore(flags);

 	osc_enable_autocalibration(OSC_ID_RC32MHZ,OSC_ID_XOSC);

	cpu_irq_enable();


	debug_string(NORMAL,PSTR("oscillator OK\r\n"),true);



//the code in the DEBUG definition block is
//here just for testing purposes
#ifdef DEBUG

	char buf[64];
	
	debug_string(NORMAL,PSTR("\r\nsizeof(void *) = "),true);
	itoa(sizeof(void *),buf,10);
	debug_string(NORMAL,buf,false);

	debug_string(NORMAL,PSTR("\r\nsizeof(flash_addr_t) = "),true);
	itoa(sizeof(flash_addr_t),buf,10);
	debug_string(NORMAL,buf,false);

//#ifndef SIM900_USART_POLLED
	//debug_string(NORMAL,PSTR ("\r\nSince you compiled in DEBUG we start the GPRS UART in main\r\n"),true);
	//usart_interruptdriver_initialize(&sim900_usart_data,USART_GPRS,USART_INT_LVL_LO);
	////usart_set_dre_interrupt_level(USART_SERIAL_GPRS,USART_INT_LVL_LO);
	//usart_set_rx_interrupt_level(USART_GPRS,USART_INT_LVL_LO);
	//usart_set_tx_interrupt_level(USART_GPRS,USART_INT_LVL_OFF);
//#endif

#endif
	hal_PCALXXXX_Init();

	simple_logger_test();
	

#if 1
	if(0==dl_init()) {

		dl_run();
	}
#else
	dl_dump_db2();

#endif

	while (1)
	{
		gpio_toggle_pin(STATUS_LED_PIN);
		delay_ms(500);
	}

	
}

static void	simple_logger_test(void)
{
	return;
	uint8_t buf[32];
	char sz[128];
	
	const uint8_t ibeg = '1';
	const uint8_t iend = ':';
	uint8_t i;
	
	for(i = ibeg;i<iend;i++) {
		buf[i-ibeg]=i;
	}
	buf[iend-ibeg] = 0;
	
	int l = iend-ibeg;
	uint32_t iv = 0xFFFFFFFF;
	crc_set_initial_value(iv);
	uint32_t v = crc_io_checksum(buf,l,CRC_32BIT);
	sprintf_P(sz,PSTR("%s -> %d -> %lX\r\n"),buf,l,v);
	debug_string(NORMAL,sz,RAM_STRING);

	while (1)
	{
		gpio_toggle_pin(STATUS_LED_PIN);
		delay_ms(500);
	}

	return;
}


//#define SWITCH_PIN					IOPORT_CREATE_PIN(PORTD, 1)
//
//#define UART_IRIDIUM_9602			&USARTD1
//#define UART_TX_IRIDIUM_9602		IOPORT_CREATE_PIN(PORTD,7)
//#define UART_RX_IRIDIUM_9602		IOPORT_CREATE_PIN(PORTD,6)
//
//static void	simple_logger_test(void)
//{
	//return;
	//
	//char szBUF[128];
	//cfg_get_gprs_apn("vodafone IT",szBUF,64);
	//debug_string(NORMAL,szBUF,RAM_STRING);
	//debug_string_P(NORMAL,g_szCRLF);
//
	//cfg_get_gprs_apn("I TIM",szBUF,64);
	//debug_string(NORMAL,szBUF,RAM_STRING);
	//debug_string_P(NORMAL,g_szCRLF);
//
	//cfg_get_gprs_apn("ok",szBUF,64);
	//debug_string(NORMAL,szBUF,RAM_STRING);
	//debug_string_P(NORMAL,g_szCRLF);
//
	//cfg_get_gprs_apn("vodafone IT",szBUF,64);
	//debug_string(NORMAL,szBUF,RAM_STRING);
	//debug_string_P(NORMAL,g_szCRLF);
//
//
//while(1) {
	//delay_ms(1500);
	//gpio_toggle_pin(SWITCH_PIN);//  ioport_configure_pin(SWITCH_PIN, IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
	//status_led_toggle();
//}
//
	//return;
	//
	//ioport_configure_pin(UART_TX_IRIDIUM_9602, IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
	//ioport_configure_pin(UART_RX_IRIDIUM_9602, IOPORT_DIR_INPUT);
//
	//// USART options.
	//const usart_rs232_options_t USART_SERIAL_OPTIONS = {
		//.baudrate = 19200,
		//.charlength = USART_CHAR_LENGTH,
		//.paritytype = USART_PARITY,
		//.stopbits = USART_STOP_BIT
	//};
//
//
	////Configure the Debug facilty
	//sysclk_enable_module(SYSCLK_PORT_D,PR_USART1_bm);
	//usart_init_rs232(UART_IRIDIUM_9602, &USART_SERIAL_OPTIONS);
//
	//usart_set_rx_interrupt_level(UART_IRIDIUM_9602,USART_INT_LVL_LO);
	//usart_set_tx_interrupt_level(UART_IRIDIUM_9602,USART_INT_LVL_OFF);
	//
	//usart_rx_enable(UART_IRIDIUM_9602);
	//usart_tx_enable(UART_IRIDIUM_9602);
//
//
//
//
	//delay_s(5);
//
	//while(1) {
		//usart_putchar(UART_IRIDIUM_9602,'A');
		//usart_putchar(UART_IRIDIUM_9602,'T');
		//usart_putchar(UART_IRIDIUM_9602,'\r');
		//usart_putchar(UART_IRIDIUM_9602,'\n');
		//delay_ms(2000);
	//}
	//
	//
 	//return;
	//ioport_configure_pin(SWITCH_PIN, IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
	//while(1) {
		//delay_ms(1500);
		//gpio_toggle_pin(SWITCH_PIN);//  ioport_configure_pin(SWITCH_PIN, IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
		//status_led_toggle();
	//}
//// 	
//// 	
//// 	return;
	//powerSwitch_init();
	//powerSwitch_toggle();
	//delay_ms(5000);
	//MBXXXX_init();
	//MBXXXX_enable();
	//while(1) {	MBXXXX_Yield();gpio_toggle_pin(STATUS_LED_PIN);delay_ms(100); }
//}
/*
ISR(USARTD1_RXC_vect)
{
	usart_putchar(USART_DEBUG,usart_getchar(&USARTD1));
}
*/
