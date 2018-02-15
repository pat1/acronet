/**
 * \file *********************************************************************
 *
 * \brief USART Serial configuration
 *
 * Copyright (c) 2011 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#ifndef CONF_USART_SERIAL_H_INCLUDED
#define CONF_USART_SERIAL_H_INCLUDED

#define USART_DEBUG				&USARTF0
#define USART_DEBUG_BAUDRATE	115200

#define USART_GPRS				&USARTE0
#define USART_GPRS_BAUDRATE		19200

#define USART_CHAR_LENGTH        USART_CHSIZE_8BIT_gc
#define USART_PARITY             USART_PMODE_DISABLED_gc
#define USART_STOP_BIT           false

#define  USART_GPRS_RX_Vect			USARTE0_RXC_vect
#define  USART_GPRS_TX_Vect			USARTE0_TXC_vect
#define  USART_GPRS_DRE_Vect		USARTE0_DRE_vect
#define  USART_GPRS_SYSCLK			SYSCLK_USART0

//////////////////////////////////////////////////////////
// SENSORCODE Define here sensors' serial communications
//////////////////////////////////////////////////////////
// SENSORCODE Define here sensors' serial communications
//#define USART_RS485_1				&USARTC0
//#define USART_RS485_1_BAUDRATE		9600
//
//#define USART_RS232_1				&USARTC0
//#define USART_RS232_1_BAUDRATE		9600
//
//#define USART_SENSIT				&USARTC0
//#define USART_SENSIT_BAUDRATE		9600
//
//#define USART_MBXXXX				SP336_USART0
//#define USART_MBXXXX_BAUDRATE		9600
//#define MBXXXX_UART_RXC_VECT		USARTC0_RXC_vect
//#define MBXXXX_UART_RX_PIN          SP336_USART0_PIN_RX_SIGNAL

//#define USART_MBXXXX				&USARTF0
//#define USART_MBXXXX_BAUDRATE		9600
//#define MBXXXX_UART_RXC_VECT		USARTF0_RXC_vect
//#define MBXXXX_UART_RX_PIN          IOPORT_CREATE_PIN(PORTF, 6)

//#define USART_MBXXXX				&USARTC0
//#define USART_MBXXXX_BAUDRATE		9600
//#define MBXXXX_UART_RXC_VECT		USARTC0_RXC_vect
//#define MBXXXX_UART_RX_PIN          IOPORT_CREATE_PIN(PORTC, 2)

//#define USART_MBXXXX				&USARTD1
//#define USART_MBXXXX_BAUDRATE		9600
//#define MBXXXX_UART_RXC_VECT		USARTD1_RXC_vect
//#define MBXXXX_UART_RX_PIN          IOPORT_CREATE_PIN(PORTD, 6)

//////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////


#endif /* CONF_USART_SERIAL_H_INCLUDED */
