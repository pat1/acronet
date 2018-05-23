/*
 * ACRONET Project
 * http://www.acronet.cc
 *
 * Copyright ( C ) 2014 Acrotec srl
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the EUPL v.1.1 license.  See http://ec.europa.eu/idabc/eupl.html for details.
 *
 * 
 * 
 */


#include "Acronet/setup.h"


#include <asf.h>


#include "Acronet/globals.h"
#include <stdio.h>
#include <conf_board.h>
#include <conf_usart_serial.h>
#include "config/conf_usart_serial.h"
#include "Acronet/services/config/config.h"
#include "Acronet/drivers/SP336/SP336.h"
#include "Acronet/drivers/StatusLED/status_led.h"
#include "Acronet/drivers/UART_INT/cbuffer_usart.h"


RET_ERROR_CODE SP336_Init( void )
{
#ifdef SP336_USART0_BPS
	{
		const usart_rs232_options_t uo = {	.baudrate = SP336_USART0_BPS,
											.charlength = USART_CHSIZE_8BIT_gc,
											.paritytype = USART_PMODE_DISABLED_gc,
											.stopbits = false };
		SP336_Config(&SP336_USART0,&uo);
	}
#endif

#ifdef SP336_USART1_BPS
	{
		const usart_rs232_options_t uo = {	.baudrate = SP336_USART1_BPS,
											.charlength = USART_CHSIZE_8BIT_gc,
											.paritytype = USART_PMODE_DISABLED_gc,
											.stopbits = false };
		SP336_Config(&SP336_USART1,&uo);
	}
#endif

#ifdef SP336_USART2_BPS
	{
		const usart_rs232_options_t uo = {	.baudrate = SP336_USART2_BPS,
											.charlength = USART_CHSIZE_8BIT_gc,
											.paritytype = USART_PMODE_DISABLED_gc,
											.stopbits = false };
		SP336_Config(&SP336_USART2,&uo);
	}
#endif

#ifdef SP336_USART3_BPS
	{
		const usart_rs232_options_t uo = {	.baudrate = SP336_USART3_BPS,
											.charlength = USART_CHSIZE_8BIT_gc,
											.paritytype = USART_PMODE_DISABLED_gc,
											.stopbits = false };
		SP336_Config(&SP336_USART3,&uo);
	}
#endif
	
	return AC_ERROR_OK;
}

RET_ERROR_CODE SP336_Config(USART_t * const pUsart,const usart_rs232_options_t * const pConf)
{

	usart_serial_init(pUsart, pConf);

	usart_set_rx_interrupt_level(pUsart,USART_INT_LVL_LO);
//	usart_set_tx_interrupt_level(pUsart,USART_INT_LVL_LO);

	return AC_ERROR_OK;
}


void SP336_RX_Disable(USART_t * const pUsart)
{	
	usart_rx_disable(pUsart);
}

void SP336_RX_Enable(USART_t * const pUsart)
{
	usart_rx_enable(pUsart);
}

static SP336_CALLBACK cb_usart0 = NULL;
static SP336_CALLBACK cb_usart1 = NULL;

RET_ERROR_CODE SP336_RegisterCallback(USART_t * const id,SP336_CALLBACK fn)
{
	SP336_CALLBACK * cb = id==(&SP336_USART0)?&cb_usart0:&cb_usart1;
	
	*cb = fn;
	
	return AC_ERROR_OK;
}


static void __inline__ internal_HalfDuplex_0_IN(void)
{
	gpio_set_pin_high(SP336_USART0_PIN_TX_ENABLE);
	SP336_USART0.STATUS |= 0b01000000;
}

static void __inline__ internal_HalfDuplex_1_IN(void)
{
	gpio_set_pin_high(SP336_USART1_PIN_TX_ENABLE);
	SP336_USART1.STATUS |= 0b01000000;
}

#ifdef SP336_USART2

static void __inline__ internal_HalfDuplex_2_IN(void)
{
	gpio_set_pin_high(SP336_USART2_PIN_TX_ENABLE);
	SP336_USART2.STATUS |= 0b01000000;
}
#endif //SP336_USART2

#ifdef SP336_USART3
static void __inline__ internal_HalfDuplex_3_IN(void)
{
	gpio_set_pin_high(SP336_USART3_PIN_TX_ENABLE);
	SP336_USART3.STATUS |= 0b01000000;
}
#endif //SP336_USART3


static void __inline__ internal_HalfDuplex_0_OUT(void)
{
	while (!(SP336_USART0.STATUS & 0b01000000)) { barrier(); };
	gpio_set_pin_low(SP336_USART0_PIN_TX_ENABLE);
}

static void __inline__ internal_HalfDuplex_1_OUT(void)
{
	while (!(SP336_USART1.STATUS & 0b01000000)) { barrier(); };
	gpio_set_pin_low(SP336_USART1_PIN_TX_ENABLE);
}

#ifdef SP336_USART2
static void __inline__ internal_HalfDuplex_2_OUT(void)
{
	while (!(SP336_USART2.STATUS & 0b01000000)) { barrier(); };
	gpio_set_pin_low(SP336_USART2_PIN_TX_ENABLE);
}
#endif

#ifdef SP336_USART3
static void __inline__ internal_HalfDuplex_3_OUT(void)
{
	while (!(SP336_USART3.STATUS & 0b01000000)) { barrier(); };
	gpio_set_pin_low(SP336_USART3_PIN_TX_ENABLE);
}
#endif


static RET_ERROR_CODE SP336_0_PutString2(const char * const psz,const uint16_t len)
{
	uint16_t i = 0;

#if (SP336_MODE==SP336_MODE_RS485_HALFDUP)
	internal_HalfDuplex_0_IN();
#endif
	
	while(i<len) {
		const uint8_t c = psz[i++];
		if (c==0)
		{
			return AC_ERROR_OK;
		}
		usart_putchar(&SP336_USART0,c);
	}

	return AC_BUFFER_OVERFLOW;
}

RET_ERROR_CODE SP336_0_PutString(const char * const ps,const uint16_t len)
{
	const RET_ERROR_CODE e = SP336_0_PutString2(ps,len);

#if (SP336_MODE==SP336_MODE_RS485_HALFDUP)
	internal_HalfDuplex_0_OUT();
#endif
	
	return e;
}

RET_ERROR_CODE SP336_0_PutBuffer(const uint8_t * const pBuf,const uint16_t len)
{
	uint16_t i = 0;
	//usart_putchar(USART_DEBUG,'s');
#if (SP336_MODE==SP336_MODE_RS485_HALFDUP)
	internal_HalfDuplex_0_IN();
#endif
	
	
	while(i<len) {
		const uint8_t c = pBuf[i++];
		usart_putchar(&SP336_USART0,c);
	}

#if (SP336_MODE==SP336_MODE_RS485_HALFDUP)
	internal_HalfDuplex_0_OUT();
#endif

	return AC_ERROR_OK;
}


static RET_ERROR_CODE SP336_1_PutString2(const char * const psz,const uint16_t len)
{
	uint16_t i = 0;

	#if (SP336_MODE==SP336_MODE_RS485_HALFDUP)
	internal_HalfDuplex_1_IN();
	#endif
	
	while(i<len) {
		const uint8_t c = psz[i++];
		if (c==0)
		{
			return AC_ERROR_OK;
		}
		usart_putchar(&SP336_USART1,c);
	}

	return AC_BUFFER_OVERFLOW;
}

RET_ERROR_CODE SP336_1_PutString(const char * const ps,const uint16_t len)
{
	const RET_ERROR_CODE e = SP336_1_PutString2(ps,len);

	#if (SP336_MODE==SP336_MODE_RS485_HALFDUP)
	internal_HalfDuplex_1_OUT();
	#endif
	
	return e;
}


RET_ERROR_CODE SP336_1_PutBuffer(const uint8_t * const pBuf,const uint16_t len)
{
	uint16_t i = 0;
	
#if (SP336_MODE==SP336_MODE_RS485_HALFDUP) || (SP336_MODE_MIXED_HALFDUP)
	internal_HalfDuplex_1_IN();
#endif
	
	
	while(i<len) {
		const uint8_t c = pBuf[i++];
		usart_putchar(&SP336_USART1,c);
	}

#if (SP336_MODE==SP336_MODE_RS485_HALFDUP) || (SP336_MODE_MIXED_HALFDUP)
	internal_HalfDuplex_1_OUT();
#endif

	return AC_ERROR_OK;
}

#ifdef SP336_USART2

static RET_ERROR_CODE SP336_2_PutString2(const char * const psz,const uint16_t len)
{
	uint16_t i = 0;

#if (SP336_2_MODE==SP336_2_MODE_RS485_HALFDUP)
	internal_HalfDuplex_2_IN();
#endif
	
	while(i<len) {
		const uint8_t c = psz[i++];
		if (c==0)
		{
			return AC_ERROR_OK;
		}
		usart_putchar(&SP336_USART2,c);
	}

	return AC_BUFFER_OVERFLOW;
}

RET_ERROR_CODE SP336_2_PutString(const char * const ps,const uint16_t len)
{
	const RET_ERROR_CODE e = SP336_2_PutString2(ps,len);

#if (SP336_2_MODE==SP336_2_MODE_RS485_HALFDUP)
	internal_HalfDuplex_2_OUT();
#endif
	
	return e;
}


RET_ERROR_CODE SP336_2_PutBuffer(const uint8_t * const pBuf,const uint16_t len)
{
	uint16_t i = 0;
	
#if (SP336_2_MODE==SP336_2_MODE_RS485_HALFDUP)
	internal_HalfDuplex_2_IN();
#endif
	
//	usart_putchar(USART_DEBUG,'2');
	
	while(i<len) {
		const uint8_t c = pBuf[i++];
		usart_putchar(&SP336_USART2,c);
	}

#if (SP336_2_MODE==SP336_2_MODE_RS485_HALFDUP)
	internal_HalfDuplex_2_OUT();
#endif

	return AC_ERROR_OK;
}

#endif //SP336_USART2


#ifdef SP336_USART3

static RET_ERROR_CODE SP336_3_PutString2(const char * const psz,const uint16_t len)
{
	uint16_t i = 0;

#if (SP336_2_MODE==SP336_2_MODE_RS485_HALFDUP) || (SP336_2_MODE_MIXED_HALFDUP)
	internal_HalfDuplex_3_IN();
#endif
	
	while(i<len) {
		const uint8_t c = psz[i++];
		if (c==0)
		{
			return AC_ERROR_OK;
		}
		usart_putchar(&SP336_USART3,c);
	}

	return AC_BUFFER_OVERFLOW;
}

RET_ERROR_CODE SP336_3_PutString(const char * const ps,const uint16_t len)
{
	const RET_ERROR_CODE e = SP336_3_PutString2(ps,len);

#if (SP336_2_MODE==SP336_2_MODE_RS485_HALFDUP) || (SP336_2_MODE_MIXED_HALFDUP)
	internal_HalfDuplex_3_OUT();
#endif
	
	return e;
}


RET_ERROR_CODE SP336_3_PutBuffer(const uint8_t * const pBuf,const uint16_t len)
{
	uint16_t i = 0;
	
#if (SP336_2_MODE==SP336_2_MODE_RS485_HALFDUP) || (SP336_2_MODE_MIXED_HALFDUP)
	internal_HalfDuplex_3_IN();
#endif
	
//	usart_putchar(USART_DEBUG,'3');
	while(i<len) {
		const uint8_t c = pBuf[i++];
		usart_putchar(&SP336_USART3,c);
	}

	//{
		//uint8_t buf[8];
		//uint8_t i = 0;
		//for(;i<len;i++) {
			//uint8_t v = pBuf[i];
			//sprintf_P(buf, PSTR("%02X "),v);
			//debug_string(NORMAL,buf,RAM_STRING);
		//}
//
		//debug_string_1P(NORMAL,PSTR("\r\n"));
	//}


#if (SP336_2_MODE==SP336_2_MODE_RS485_HALFDUP) || (SP336_2_MODE_MIXED_HALFDUP)
	internal_HalfDuplex_3_OUT();
#endif

	return AC_ERROR_OK;
}

#endif //SP336_USART2

