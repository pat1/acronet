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
#include "Acronet/HAL/hal_interface.h"


#include <asf.h>
#include <stdio.h>
#include <string.h>
#include "board.h"
//#include "sysclk.h"

#include "conf_board.h"

#include "Acronet/drivers/UART_INT/buffer_usart.h"
#include "config/conf_usart_serial.h"
//#include "Acronet/globals.h"
#include "Acronet/drivers/SP336/SP336.h"
#include "Acronet/Sensors/MBXXXX/MBXXXX.h"


//#define MBXXXX_MISS_STATS
#define MB_RESOLUTION_MM
#define MBXXXX_SWITCH_PIN			IOPORT_CREATE_PIN(PORTD, 1)
#define USART_MBXXXX_BAUDRATE		9600
#define MBXXXX_PUSART				(&SP336_USART0)
#define MBXXXX_PIN_RX_SIGNAL		SP336_USART0_PIN_RX_SIGNAL


// SENSORCODE
//#define LG_EQUALMEASURES		3	// Number of measures with the same value we want to obtain 
#define LG_UARTBUFFERSIZE		128	// Is the length of the string buffer
#define LG_DATABUFSIZE			17	// Raw measures buffer size. On this measures array statistics are done. 
#define LG_MEASUREBUFMID        8   //


static uint16_t g_Data[LG_DATABUFSIZE];
static uint16_t g_samples = 0;
static uint16_t g_noRangeFound = 0;


static volatile uint8_t g_iter_data_put = 0;
static volatile uint8_t g_iter_data_get = 0;

#ifdef 	MBXXXX_MISS_STATS
static volatile uint16_t g_misses = 0;
#endif



static volatile char g_UART_Buffer[LG_UARTBUFFERSIZE];


static uint8_t medianInsert(const uint16_t val);
static void MBXXXX_process_line(const char * pSZ);
static void MBXXXX_rx(const char c);

static inline void MBXXXX_line_reset(void)
{
	g_UART_Buffer[0] = 0;
	g_iter_data_put = 0;
	g_iter_data_get = 0;
	
}


static char MBXXXX_line_GetChar(void)
{
	if(g_iter_data_put == 0) return 0;
	const char c = g_UART_Buffer[g_iter_data_get++];

	if(g_iter_data_get==g_iter_data_put) {
		MBXXXX_line_reset();
	}
	return c;

}

RET_ERROR_CODE MBXXXX_init(void)
{

	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"MBXXXX_INIT");

	
//	ioport_configure_pin(MBXXXX_UART_RX_PIN, IOPORT_DIR_INPUT | IOPORT_INV_ENABLED );
	ioport_configure_pin(MBXXXX_PIN_RX_SIGNAL, IOPORT_DIR_INPUT );
	
	//USART options.
	const usart_rs232_options_t usart_options = {
		.baudrate = USART_MBXXXX_BAUDRATE,
		.charlength = USART_CHAR_LENGTH,
		.paritytype = USART_PARITY,
		.stopbits = USART_STOP_BIT
	};
	SP336_Config(MBXXXX_PUSART,&usart_options);
	usart_tx_enable(MBXXXX_PUSART);
	SP336_RegisterCallback(MBXXXX_PUSART,MBXXXX_rx);
	

	usart_rx_disable(MBXXXX_PUSART);
	
	usart_set_rx_interrupt_level(MBXXXX_PUSART,USART_INT_LVL_LO);
	MBXXXX_line_reset();
	MBXXXX_reset_data();
	
	return AC_ERROR_OK;
}

void MBXXXX_enable(void)
{
	//Enable interrupt
	ioport_configure_pin(MBXXXX_SWITCH_PIN, IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
	usart_set_rx_interrupt_level(MBXXXX_PUSART,USART_INT_LVL_LO);
	usart_rx_enable(MBXXXX_PUSART);
}

void MBXXXX_disable(void)
{
	ioport_configure_pin(MBXXXX_SWITCH_PIN, IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
	usart_set_rx_interrupt_level(MBXXXX_PUSART,USART_INT_LVL_LO);
	usart_rx_disable(MBXXXX_PUSART);
	
}


//void MBXXXX_triggerReading(void)
//{
	////Enable switch
	//MBXXXX_interrupt_start();
//}

void MBXXXX_get_data(MBXXXX_DATA * const ps)
{
	//LG_MBXXXX_STATS measureBuf;
	//char szBuf[128];
	
	if (g_samples == 0)
	{
		ps->val		= -9999;
		ps->maxVal	= -9999;
		ps->minVal	= -9999;
	} else
	{
		ps->val		= g_Data[LG_MEASUREBUFMID];
		ps->maxVal	= g_Data[ LG_DATABUFSIZE-1 ];
		ps->minVal	= g_Data[0];
	}
	
	if (ps->val==9999)
		ps->val = -9999;
	if (ps->maxVal==9999)
		ps->maxVal = -9999;
	if (ps->minVal==9999)
		ps->minVal = -9999;
	
	ps->samples	= g_samples;
	ps->noRangeFound = g_noRangeFound;
	
	//sprintf_P(szBuf,PSTR("val: %u - maxVal: %u - minVal: %u \r\n"),g_Data[LG_MEASUREBUFMID],g_Data[ LG_DATABUFSIZE-1 ],g_Data[0]);
	//debug_string(VERBOSE,szBuf,RAM_STRING);

#ifdef 	MBXXXX_MISS_STATS
	ps->misses = g_misses;
#endif

	//MBXXXX_reset_data();
}

void MBXXXX_reset_data(void)
{
	g_samples = 0;
	g_noRangeFound = 0;

#ifdef 	MBXXXX_MISS_STATS
	g_misses   = 0;
#endif

}

//static uint16_t MBXXXX_adcGetValue( void )
//{
	////ToDO
	//return 0;
//}

//static void MBXXXX_interrupt_start(void)
//{
	////Enable interrupt
	//usart_set_rx_interrupt_level(USART_RS232_1,USART_INT_LVL_LO);  //Occhio alla priorità
	//usart_rx_enable(USART_RS232_1);
//}
//
//static void MBXXXX_interrupt_stop(void)
//{
	////Disable switch
//
//}

static uint8_t medianInsert_right(uint16_t val,uint8_t pos)
{
	uint16_t v0 = val;
	for(uint8_t idx=pos;idx<LG_DATABUFSIZE;++idx)
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
	const uint16_t vm = g_Data[LG_MEASUREBUFMID];
	
	//if (vm==0) {
		//return medianInsert_right(val,0);
	//} else 
	if (val<vm) {
		for(idx=LG_MEASUREBUFMID;idx>0;--idx) {
			const uint16_t vr = g_Data[idx];
			const uint16_t vl = g_Data[idx-1];
			if ((val>=vl) && (val<=vr)) {
				return medianInsert_right(val,idx);
			}
		}
		//New Minimum, it also manage the undef value 
		return medianInsert_right(val,0);

	} else if(val>vm)	{
		for(idx=LG_MEASUREBUFMID;idx<LG_DATABUFSIZE-1;++idx) {
			const uint16_t vl = g_Data[idx];
			const uint16_t vr = g_Data[idx+1];
			if ((val>=vl) && (val<=vr)) {
				return medianInsert_left(val,idx);
			}
		}
		//New Maximum
		return medianInsert_left(val,LG_DATABUFSIZE-1);
	} else if (val==vm) {
		medianInsert_right(val,LG_MEASUREBUFMID);
		medianInsert_left(val,LG_MEASUREBUFMID);
	} 

	return 0;
}

static void MBXXXX_process_line(const char * pSZ)
{
	
	uint16_t reading;
#ifdef MB_RESOLUTION_MM
	for (uint8_t i = 1 ; i < 5 ; i++)
	{
		if ((pSZ[i]) < 48 || (pSZ[i]) > 57 )
		return;
	}
	reading = (uint16_t)(pSZ[4] - 48);
	reading += (uint16_t)(pSZ[3]-48) * 10 ;
	reading += (uint16_t)(pSZ[2]-48) * 100 ;
	reading += (uint16_t)(pSZ[1]-48) * 1000 ;
#else
	for (uint8_t i = 1 ; i < 4 ; i++)
	{
		if ((pSZ[i]) < 48 || (pSZ[i]) > 57 )
		return;
	}
	reading = (uint16_t)(pSZ[3] - 48);
	reading += (uint16_t)(pSZ[2]-48) * 10 ;
	reading += (uint16_t)(pSZ[1]-48) * 100 ;
#endif
	
	if (reading != MBXXXX_NORANGEFOUND)
	{
		g_samples++;
		medianInsert(reading);
	}
	else
		g_noRangeFound++;
	
//#if 0 //If you want to debug...
	//if (g_log_verbosity>=VERBOSE)
	//{
		//char szBuf[256];
		//uint8_t d = 0;
		//for(uint8_t i = 0;i<LG_DATABUFSIZE;++i) {
			//d += sprintf_P(szBuf+d,PSTR("%u "),g_Data[i]);
		//}
		//sprintf_P(szBuf+d,PSTR("\r\n"));
		//debug_string(VERBOSE,szBuf,RAM_STRING);
	//}
//
//#endif
	

}



bool MBXXXX_Yield( void )
{
	static char szBuf[8];
	static uint8_t idx = 0;
	char c;
	while((c = MBXXXX_line_GetChar()) != 0)
	{

		if (c==0x0D)	{
			szBuf[idx]=0;
			idx=0;
			MBXXXX_process_line(szBuf);
		} else if (c=='R') {
			szBuf[0]='R';
			szBuf[1]=0;
			idx = 1;
		} else if(idx>0) { //This ensures that the only first character allowed is 'R'
			szBuf[idx++] = c;
		}

		if ( (c=='*') || (idx>(sizeof(szBuf)-1)) )
		{
			debug_string_2P(NORMAL,PSTR("MBXXXX_Yield"),PSTR("Synchronization lost"));
			szBuf[0]=0;
			idx = 0;
		}
		
		return true;
		
	}
	return false;
}


RET_ERROR_CODE MBXXXX_Data2String(const MBXXXX_DATA * const st,char * const sz, uint16_t * const len_sz)
{

#ifdef MBXXXX_MISS_STATS
	uint16_t len = snprintf_P(sz,*len_sz,PSTR("&L=%d&Lmax=%d&Lmin=%d&n=%u&Z=%u&NRF=%u"),st->val,st->maxVal,st->minVal,st->samples,st->misses,st->noRangeFound);
#else
	uint16_t len = snprintf_P(sz,*len_sz,PSTR("&L=%d&Lmax=%d&Lmin=%d&n=%u&NRF=%u"),st->val,st->maxVal,st->minVal,st->samples,st->noRangeFound);
#endif


	const RET_ERROR_CODE e = (len < *len_sz) ? AC_ERROR_OK : AC_BUFFER_OVERFLOW;
	*len_sz = len;
	return e;
}




static void MBXXXX_rx(const char c)
{
//	usart_putchar(USART_DEBUG,c);
	if (g_iter_data_put<LG_UARTBUFFERSIZE)
	{
		g_UART_Buffer[g_iter_data_put++] = c;
	} else {
		
		if(g_UART_Buffer[LG_UARTBUFFERSIZE-1] != '*')
		{
			g_UART_Buffer[LG_UARTBUFFERSIZE-1] = '*';
#ifdef MBXXXX_MISS_STATS
			g_misses++;
#endif			
		}
		
	}
	
}
