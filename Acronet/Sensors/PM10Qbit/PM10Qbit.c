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
 * Notes: PM10Qbit provides measurements of PM10 concentrations every 4s. This driver provides
 *        an # of samples, average, max and min values.
 */

#include "Acronet/setup.h"
#include "Acronet/HAL/hal_interface.h"


#include <asf.h>
#include <stdio.h>
#include <string.h>
#include "board.h"
#include "sysclk.h"

#include "conf_board.h"
//#include <util/delay.h>
//#include "Acronet/globals.h"

#include "Acronet/drivers/UART_INT/buffer_usart.h"
#include "config/conf_usart_serial.h"

#include "Acronet/Sensors/PM10Qbit/PM10Qbit.h"

#define USART_PM10QBIT				&USARTC0
#define USART_PM10QBIT_BAUDRATE		9600
#define PM10QBIT_UART_RXC_VECT		USARTC0_RXC_vect


#define PM10_BUFSIZE	32 // (60/4)=15 samples/min * 2 min = 30
#define BUFSIZE			8
#define MAXPM10VAL		20

#define LG_UARTBUFFERSIZE		128	// Is the length of the string buffer
#define LG_DATABUFSIZE			17	// Raw measures buffer size. On this measures array statistics are done.

uint32_t g_partialSum;
//uint16_t g_maxVal = 0, g_minVal = UINT16_MAX;
uint16_t g_measureCounter;
uint16_t g_PM10val;
uint16_t g_zeros;


static volatile uint8_t g_iter_data_put = 0;
static volatile uint8_t g_iter_data_get = 0;

static volatile char g_UART_Buffer[LG_UARTBUFFERSIZE];

static inline void PM10QBIT_line_reset(void)
{
	g_UART_Buffer[0] = 0;
	g_iter_data_put = 0;
	g_iter_data_get = 0;
	
}


static char PM10QBIT_line_GetChar(void)
{
	if(g_iter_data_put == 0) return 0;
	const char c = g_UART_Buffer[g_iter_data_get++];

	if(g_iter_data_get==g_iter_data_put) {
		PM10QBIT_line_reset();
	}
	return c;

}

void PM10QBIT_init(void){
	
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"PM10QBIT_INIT");
	//USART options.
	
	ioport_configure_pin(PM10QBIT_PIN_RX_SIGNAL, IOPORT_DIR_INPUT);
	
	static usart_rs232_options_t RS232_SERIAL_OPTIONS = {
		.baudrate = USART_PM10QBIT_BAUDRATE,
		.charlength = USART_CHAR_LENGTH,
		.paritytype = USART_PARITY,
		.stopbits = USART_STOP_BIT
	};
	sysclk_enable_module(SYSCLK_PORT_C,PR_USART0_bm);
	usart_serial_init(USART_PM10QBIT, &RS232_SERIAL_OPTIONS);

	usart_rx_disable(USART_PM10QBIT);
	
	PM10QBIT_line_reset();
	PM10QBIT_reset_data();
}

void PM10QBIT_enable(void)
{
	//Enable interrupt
	usart_set_rx_interrupt_level(USART_PM10QBIT,USART_INT_LVL_LO);
	usart_rx_enable(USART_PM10QBIT);
}

void PM10QBIT_disable(void)
{
	usart_set_rx_interrupt_level(USART_PM10QBIT,USART_INT_LVL_OFF);
	usart_rx_disable(USART_PM10QBIT);
}

void PM10QBIT_reset_data(void)
{
	g_partialSum= 0;
	g_measureCounter = 0;
	g_zeros = 0;
}

void PM10QBIT_get_data (PM10QBIT_DATA * const temp)
{
//TODO: adjust race condition		
	temp->mean = (float)(g_partialSum*100) / (float)(g_measureCounter-g_zeros);
	temp->meanZeros = (float)(g_partialSum*100) / (float)(g_measureCounter);
	temp->samples=g_measureCounter;
	temp->zeros=g_zeros;

	//char szBuf[256];
	//sprintf_P(szBuf,PSTR("mean = %u\tsamples = %u\r\n"),temp->mean,temp->samples);
	//debug_string(NORMAL,szBuf,false);
	
}

static void PM10QBIT_process_line(const char * pSZ)
{
	
	uint16_t reading = 0;
	
	if (pSZ[0] != 'P' || pSZ[1] != 'M' || pSZ[2] != ':' || pSZ[3] != ' ')
		return;
	
	for (uint8_t i = 0 ; i < 15 ; i++)
	{
		
		if ((pSZ[i]) >= 48 && (pSZ[i]) <= 57 )
		{
			reading = reading * 10 + (uint16_t)(pSZ[i] - 48);
		}
	}
	
	if(reading!=0)
		g_partialSum += reading;
	else
		g_zeros++;
		
	g_measureCounter++;
}

bool PM10QBIT_Yield( void )
{
	static char szBuf[16];
	static uint8_t idx = 0;
	char c;
	while((c = PM10QBIT_line_GetChar()) != 0)
	{

		if (c==0x0D)	{
			szBuf[idx]=0;
			idx=0;
			PM10QBIT_process_line(szBuf);
			} else if (c=='P') {
			szBuf[0]='P';
			szBuf[1]=0;
			idx = 1;
			} else if(idx>0) { //This ensures that the only first character allowed is 'R'
			szBuf[idx++] = c;
		}

		if ( (c=='*') || (idx>(sizeof(szBuf)-1)) )
		{
			debug_string_2P(NORMAL,PSTR("PM10QBIT_Yield") ,PSTR("Synchronization lost"));
			szBuf[0]=0;
			idx = 0;
		}
		
		return true;
		
	}
	return false;
}

RET_ERROR_CODE PM10QBIT_Data2String(const PM10QBIT_DATA * const st,char * const sz, int16_t * const len_sz)
{

	int16_t len = snprintf_P(	sz,
								*len_sz,
								PSTR("&PM10_s=%u&PM10_s0=%u&PM10=%u&PM10zer=%u") ,
								st->samples,
								st->zeros,
								st->mean,
								st->meanZeros );


	const RET_ERROR_CODE e = (len < *len_sz) ? AC_ERROR_OK : AC_BUFFER_OVERFLOW;
	*len_sz = len;
	return e;
}



#ifdef SETUP_PM10QBIT
ISR(PM10QBIT_UART_RXC_VECT)
{
	const char c = usart_get(USART_PM10QBIT);
	//	usart_putchar(USART_DEBUG,c);
	if (g_iter_data_put<LG_UARTBUFFERSIZE)
	{
		g_UART_Buffer[g_iter_data_put++] = c;
	} else {
		
		if(g_UART_Buffer[LG_UARTBUFFERSIZE-1] != '*')
		{
			g_UART_Buffer[LG_UARTBUFFERSIZE-1] = '*';
		}
		
	}
	
}
#endif
