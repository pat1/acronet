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

static volatile	uint8_t g_spin0 = 0;
static volatile	uint8_t g_spin1 = 0;

RET_ERROR_CODE SP336_Init( void )
{
	return AC_ERROR_OK;
}

RET_ERROR_CODE SP336_Config(USART_t * const pUsart,const usart_rs232_options_t * const pConf)
{

	usart_serial_init(pUsart, pConf);

	usart_set_rx_interrupt_level(pUsart,USART_INT_LVL_LO);
	usart_set_tx_interrupt_level(pUsart,USART_INT_LVL_LO);

	return AC_ERROR_OK;
}


RET_ERROR_CODE SP336_PutString(USART_t * const pUsart, const char * const sz)
{

	RET_ERROR_CODE ret = 1;
	const uint8_t en = (&SP336_USART0)==pUsart?0x00:0xFF;

	volatile uint8_t * spin;

	if (en)	{
			gpio_set_pin_high(SP336_USART1_PIN_TX_ENABLE);
			spin = &g_spin1;
	} else {
			gpio_set_pin_high(SP336_USART0_PIN_TX_ENABLE);
			spin = &g_spin0;
	}

	*spin = 1;

	uint8_t i = 0;
	while(i<255) {
		const uint8_t c = sz[i++];
		if (0==c)
		{
			ret = AC_ERROR_OK;
			break;
		}
		 
		usart_putchar(pUsart,c);
	}

	while (!(*spin) == 0) {
	}


	if (en)	{
		gpio_set_pin_low(SP336_USART1_PIN_TX_ENABLE);
	} else {
		gpio_set_pin_low(SP336_USART0_PIN_TX_ENABLE);
	}

	return ret;
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
	g_spin0 = 1;
}

static void __inline__ internal_HalfDuplex_1_IN(void)
{
	gpio_set_pin_high(SP336_USART1_PIN_TX_ENABLE);
	g_spin1 = 1;
}

#ifdef SP336_USART2
static volatile	uint8_t g_spin2 = 0;

static void __inline__ internal_HalfDuplex_2_IN(void)
{
	gpio_set_pin_high(SP336_USART2_PIN_TX_ENABLE);
	g_spin2 = 1;
}
#endif //SP336_USART2

#ifdef SP336_USART3
static volatile	uint8_t g_spin3 = 0;

static void __inline__ internal_HalfDuplex_3_IN(void)
{
	gpio_set_pin_high(SP336_USART3_PIN_TX_ENABLE);
	g_spin3 = 1;
}
#endif //SP336_USART3


static void __inline__ internal_HalfDuplex_0_OUT(void)
{
	while (g_spin0 != 0) { barrier(); };
	gpio_set_pin_low(SP336_USART0_PIN_TX_ENABLE);
}

static void __inline__ internal_HalfDuplex_1_OUT(void)
{
	while (g_spin1 != 0) { barrier(); };
	gpio_set_pin_low(SP336_USART1_PIN_TX_ENABLE);
}

#ifdef SP336_USART2
static void __inline__ internal_HalfDuplex_2_OUT(void)
{
	while (g_spin2 != 0) { barrier(); };
	gpio_set_pin_low(SP336_USART2_PIN_TX_ENABLE);
}
#endif

#ifdef SP336_USART3
static void __inline__ internal_HalfDuplex_3_OUT(void)
{
	while (g_spin3 != 0) { barrier(); };
	gpio_set_pin_low(SP336_USART3_PIN_TX_ENABLE);
}
#endif


RET_ERROR_CODE SP336_0_PutBuffer(const uint8_t * const pBuf,const uint16_t len)
{
	uint16_t i = 0;
	
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

RET_ERROR_CODE SP336_2_PutBuffer(const uint8_t * const pBuf,const uint16_t len)
{
	uint16_t i = 0;
	
#if (SP336_2_MODE==SP336_2_MODE_RS485_HALFDUP)
	internal_HalfDuplex_2_IN();
#endif
	
	
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

RET_ERROR_CODE SP336_3_PutBuffer(const uint8_t * const pBuf,const uint16_t len)
{
	uint16_t i = 0;
	
#if (SP336_2_MODE==SP336_2_MODE_RS485_HALFDUP) || (SP336_2_MODE_MIXED_HALFDUP)
	internal_HalfDuplex_3_IN();
#endif
	
	
	while(i<len) {
		const uint8_t c = pBuf[i++];
		usart_putchar(&SP336_USART3,c);
	}

#if (SP336_2_MODE==SP336_2_MODE_RS485_HALFDUP) || (SP336_2_MODE_MIXED_HALFDUP)
	internal_HalfDuplex_3_OUT();
#endif

	return AC_ERROR_OK;
}

#endif //SP336_USART2


#ifdef SP336_UART0_RX_CB

ISR(SP336_USART0_RX_Vect)
{
	SP336_UART0_RX_CB(SP336_USART0.DATA);
}


#endif

//
//ISR(SP336_USART0_RX_Vect)
//{
	//if(NULL!=cb_usart0) cb_usart0(SP336_USART0.DATA);
//}
//
//
//ISR(SP336_USART1_RX_Vect)
//{
 //
 	//if(NULL!=cb_usart1) cb_usart1(SP336_USART1.DATA);
//}
//
//ISR(SP336_USART0_TX_Vect)
//{
	//g_spin0 = 0;
//}
//
//ISR(SP336_USART1_TX_Vect)
//{
	//g_spin1 = 0;
//}
