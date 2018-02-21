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
#include "progmem.h"
#include <conf_board.h>
#include <conf_usart_serial.h>


#include "Acronet/globals.h"
#include "Acronet/drivers/SP336/SP336.h"
#include "Acronet/services/MODBUS_RTU/master_rtu.h"

#ifdef MOD_DIG_0
#undef MOD_DIG_0
#endif
#ifdef MOD_DIG_1
#undef MOD_DIG_1
#endif
#ifdef MOD_DIG_2
#undef MOD_DIG_2
#endif
#ifdef MOD_DIG_3
#undef MOD_DIG_3
#endif

#define xstr(s) str(s)
#define str(s) #s

#ifdef MODBUS_CHAN_0 
#define MOD_DIG_0 1
#define MOD_IDX_0 0
#pragma message "MODBUS CH#0 is ON"
#else
#pragma message "MODBUS CH#0 is OFF"
#define MOD_DIG_0 0
#endif

#ifdef MODBUS_CHAN_1
#define MOD_DIG_1 1
#define MOD_IDX_1 (0 + MOD_DIG_0)
#pragma message "MODBUS CH#1 is ON"
#else
#pragma message "MODBUS CH#1 is OFF"
#define MOD_DIG_1 0
#endif

#ifdef MODBUS_CHAN_2
#define MOD_DIG_2 1
#define MOD_IDX_2 (0 + MOD_DIG_0 + MOD_DIG_1)
#pragma message "MODBUS CH#2 is ON"
#else
#pragma message "MODBUS CH#2 is OFF"
#define MOD_DIG_2 0
#endif

#ifdef MODBUS_CHAN_3
#define MOD_DIG_3 1
#define MOD_IDX_3 (0 + MOD_DIG_0 + MOD_DIG_1 + MOD_DIG_2)
#pragma message "MODBUS CH#3 is ON"
#else
#pragma message "MODBUS CH#3 is OFF"
#define MOD_DIG_3 0
#endif


#define MODBUS_CHANNELS (0 + MOD_DIG_0 + MOD_DIG_1 + MOD_DIG_2 + MOD_DIG_3)
#define MODBUS_UART_BUF_SIZE 32

#if (MODBUS_CHANNELS == 0)
#pragma message "MODBUS CH# is 0"
#elif (MODBUS_CHANNELS == 1)
#pragma message "MODBUS CH# is 1"
#elif (MODBUS_CHANNELS == 2)
#pragma message "MODBUS CH# is 2"
#elif (MODBUS_CHANNELS == 3)
#pragma message "MODBUS CH# is 3"
#endif

volatile uint8_t buf_usart[MODBUS_CHANNELS][MODBUS_UART_BUF_SIZE];

volatile uint8_t idx_beg_usart[MODBUS_CHANNELS] = {0};
volatile uint8_t idx_end_usart[MODBUS_CHANNELS] = {0};


#if   SP336_MODE == SP336_MODE_LOOPBACK
#pragma message "INFO: Compiling with SP336 mode loopback"
#elif SP336_MODE == SP336_MODE_RS485_HALFDUP
#pragma message "INFO: Compiling with SP336 mode RS485 halfduplex"
#elif SP336_MODE == SP336_MODE_RS232
#pragma message "INFO: Compiling with SP336 mode RS232"
#elif SP336_MODE == SP336_MODE_RS485_FULLDUP
#pragma message "INFO: Compiling with SP336 mode RS485 fullduplex"
#elif SP336_MODE == SP336_MODE_MIXED_HALFDUP
#pragma message "INFO: Compiling with SP336 mode mixed halfduplex"
#elif SP336_MODE == SP336_MODE_LOWPOWER_RX
#pragma message "INFO: Compiling with SP336 mode lowpower rx"
#elif SP336_MODE == SP336_MODE_MIXED_FULLDUP
#pragma message "INFO: Compiling with SP336 mode mixed fullduplex"
#elif SP336_MODE == SP336_MODE_HIGHZ
#pragma message "INFO: Compiling with SP336 mode high z"
#elif SP336_MODE == SP336_MODE_SLEW_LIMIT
#pragma message "INFO: Compiling with SP336 mode slew rate"
#endif

#if defined(SP336_UART2) || defined(SP336_UART3)
#if   SP336_2_MODE == SP336_2_MODE_LOOPBACK
#pragma message "INFO: Compiling with SP336_2 mode loopback"
#elif SP336_2_MODE == SP336_2_MODE_RS485_HALFDUP
#pragma message "INFO: Compiling with SP336_2 mode RS485 halfduplex"
#elif SP336_2_MODE == SP336_2_MODE_RS232
#pragma message "INFO: Compiling with SP336_2 mode RS232"
#elif SP336_2_MODE == SP336_2_MODE_RS485_FULLDUP
#pragma message "INFO: Compiling with SP336_2 mode RS485 fullduplex"
#elif SP336_2_MODE == SP336_2_MODE_MIXED_HALFDUP
#pragma message "INFO: Compiling with SP336_2 mode mixed halfduplex"
#elif SP336_2_MODE == SP336_2_MODE_LOWPOWER_RX
#pragma message "INFO: Compiling with SP336_2 mode lowpower rx"
#elif SP336_2_MODE == SP336_2_MODE_MIXED_FULLDUP
#pragma message "INFO: Compiling with SP336_2 mode mixed fullduplex"
#elif SP336_2_MODE == SP336_2_MODE_HIGHZ
#pragma message "INFO: Compiling with SP336_2 mode high z"
#elif SP336_2_MODE == SP336_2_MODE_SLEW_LIMIT
#pragma message "INFO: Compiling with SP336_2 mode slew rate"
#endif
#else
#pragma message "NO SP336-2 UART defined"
#endif



static bool is_usartx_empty(const uint8_t n)
{
	return 	(idx_beg_usart[n] == idx_end_usart[n]);
}

static void reset_usartx_buffer(const uint8_t n)
{
	idx_beg_usart[n] = 0;
	idx_end_usart[n] = 0;
}

static uint8_t get_usartx_byte(const uint8_t n)
{
	uint8_t ie = idx_end_usart[n];
	uint8_t ib = idx_beg_usart[n];

	if (ie == ib)
	{
		return 0;
	}

	
	uint8_t c = buf_usart[n][ib++];
	idx_beg_usart[n] = ib & (MODBUS_UART_BUF_SIZE-1);
	
	return c;
	
}

//static uint8_t put_usartx_buf(USART_t * const pusart,const char * const pBuf,uint16_t len)
//{
//
//#ifdef MODBUS_CHAN_0
	//if(ch_id==MODBUS_CHAN_0) {
		//return put_usartx_buf(MOD_IDX_0,pBuf,len);
	//} else
//#endif
//#ifdef MODBUS_CHAN_1
	//if(ch_id==MODBUS_CHAN_1) {
		//return put_usartx_buf(MOD_IDX_1,pBuf,len);
	//} else
//#endif
//#ifdef MODBUS_CHAN_2
	//if(ch_id==MODBUS_CHAN_2) {
		//return put_usartx_buf(MOD_IDX_2,pBuf,len);
	//} else
//#endif
//#ifdef MODBUS_CHAN_3
	//if(ch_id==MODBUS_CHAN_3) {
		//return put_usartx_buf(MOD_IDX_3,pBuf,len);
	//} else
//#endif
	//return AC_UNSUPPORTED;
//
//}


static void cb_usartx(const uint8_t n,const uint8_t c)
{
	uint8_t ie = idx_end_usart[n];
	uint8_t ib = idx_beg_usart[n];

	buf_usart[n][ie++]=c;
	idx_end_usart[n] = ie & (MODBUS_UART_BUF_SIZE-1);

	if (ie == ib)
	{
		idx_beg_usart[n] = (ie+1) & (MODBUS_UART_BUF_SIZE-1);
	}
	
}

#ifdef MBUSPUTFUN_SP336_0
#error "A symbol MBUSPUTFUN_SP336_0 has been defined elsewhere"
#else
#define MBUSPUTFUN_SP336_0
#endif

#ifdef MBUSPUTFUN_SP336_1
#error "A symbol MBUSPUTFUN_SP336_1 has been defined elsewhere"
#else
#define MBUSPUTFUN_SP336_1
#endif

#ifdef MBUSPUTFUN_SP336_2
#error "A symbol MBUSPUTFUN_SP336_2 has been defined elsewhere"
#else
#define MBUSPUTFUN_SP336_2
#endif

#ifdef MBUSPUTFUN_SP336_3
#error "A symbol MBUSPUTFUN_SP336_3 has been defined elsewhere"
#else
#define MBUSPUTFUN_SP336_3
#endif

#ifdef MODBUS_CHAN_0
#undef MBUSPUTFUN_SP336_0
#if(0==MOD_IDX_0)
#define MBUSPUTFUN_SP336_0 SP336_0_PutBuffer
#elif(1==MOD_IDX_0)
#define MBUSPUTFUN_SP336_0 SP336_1_PutBuffer
#elif(2==MOD_IDX_0)
#define MBUSPUTFUN_SP336_0 SP336_2_PutBuffer
#elif(3==MOD_IDX_0)
#define MBUSPUTFUN_SP336_0 SP336_3_PutBuffer
#endif
#endif //MODBUS_CHAN_0

#ifdef MODBUS_CHAN_1
#undef MBUSPUTFUN_SP336_1
#if(0==MOD_IDX_1)
#define MBUSPUTFUN_SP336_1 SP336_0_PutBuffer
#elif(1==MOD_IDX_1)
#define MBUSPUTFUN_SP336_1 SP336_1_PutBuffer
#elif(2==MOD_IDX_1)
#define MBUSPUTFUN_SP336_1 SP336_2_PutBuffer
#elif(3==MOD_IDX_1)
#define MBUSPUTFUN_SP336_1 SP336_3_PutBuffer
#endif
#endif //MODBUS_CHAN_1

#ifdef MODBUS_CHAN_2
#undef MBUSPUTFUN_SP336_2
#if(0==MOD_IDX_2)
#define MBUSPUTFUN_SP336_2 SP336_0_PutBuffer
#elif(1==MOD_IDX_2)
#define MBUSPUTFUN_SP336_2 SP336_1_PutBuffer
#elif(2==MOD_IDX_2)
#define MBUSPUTFUN_SP336_2 SP336_2_PutBuffer
#elif(3==MOD_IDX_2)
#define MBUSPUTFUN_SP336_2 SP336_3_PutBuffer
#endif
#endif //MODBUS_CHAN_2

#ifdef MODBUS_CHAN_3
#undef MBUSPUTFUN_SP336_3
#if(0==MOD_IDX_3)
#define MBUSPUTFUN_SP336_3 SP336_0_PutBuffer
#elif(1==MOD_IDX_3)
#define MBUSPUTFUN_SP336_3 SP336_1_PutBuffer
#elif(2==MOD_IDX_3)
#define MBUSPUTFUN_SP336_3 SP336_2_PutBuffer
#elif(3==MOD_IDX_3)
#define MBUSPUTFUN_SP336_3 SP336_3_PutBuffer
#endif
#endif //MODBUS_CHAN_3



RET_ERROR_CODE MBUS_issue_cmd(const uint8_t ch_id,const uint8_t * const pBuf,uint16_t len)
{
#ifdef MODBUS_CHAN_0
	if(ch_id==MODBUS_CHAN_0) {
		return MBUSPUTFUN_SP336_0(pBuf,len);
	} else
#endif
#ifdef MODBUS_CHAN_1
	if(ch_id==MODBUS_CHAN_1) {
		return MBUSPUTFUN_SP336_1(pBuf,len);
	} else
#endif
#ifdef MODBUS_CHAN_2
	if(ch_id==MODBUS_CHAN_2) {
		return MBUSPUTFUN_SP336_2(pBuf,len);
	} else
#endif
#ifdef MODBUS_CHAN_3
	if(ch_id==MODBUS_CHAN_3) {
		return MBUSPUTFUN_SP336_3(pBuf,len);
	} else
#endif
	return AC_UNSUPPORTED;
}

bool MBUS_is_empty(const uint8_t ch_id)
{

#ifdef MODBUS_CHAN_0
		if(ch_id==MODBUS_CHAN_0) {
			return is_usartx_empty(MOD_IDX_0);
		} else
#endif
#ifdef MODBUS_CHAN_1
		if(ch_id==MODBUS_CHAN_1) {
			return is_usartx_empty(MOD_IDX_1);
	} else
#endif
#ifdef MODBUS_CHAN_2
		if(ch_id==MODBUS_CHAN_2) {
			return is_usartx_empty(MOD_IDX_2);
		} else
#endif
#ifdef MODBUS_CHAN_3
		if(ch_id==MODBUS_CHAN_3) {
			return is_usartx_empty(MOD_IDX_3);
		} else
#endif		
	return true;
}

void MBUS_reset(const uint8_t ch_id)
{	
#ifdef MODBUS_CHAN_0
		if(ch_id==MODBUS_CHAN_0) {
			reset_usartx_buffer(MOD_IDX_0);
		} else
#endif
#ifdef MODBUS_CHAN_1
		if(ch_id==MODBUS_CHAN_1) {
			reset_usartx_buffer(MOD_IDX_1);
		} else
#endif
#ifdef MODBUS_CHAN_2
		if(ch_id==MODBUS_CHAN_2) {
			reset_usartx_buffer(MOD_IDX_2);
		} else
#endif
#ifdef MODBUS_CHAN_2
		if(ch_id==MODBUS_CHAN_3) {
			reset_usartx_buffer(MOD_IDX_3);
		} else
#endif
	return;	
}

uint8_t MBUS_get_byte(const uint8_t ch_id)
{
#ifdef MODBUS_CHAN_0
		if(ch_id==MODBUS_CHAN_0) {
			return get_usartx_byte(MOD_IDX_0);
		} else
#endif
#ifdef MODBUS_CHAN_1
		if(ch_id==MODBUS_CHAN_1) {
			return get_usartx_byte(MOD_IDX_1);
		} else
#endif
#ifdef MODBUS_CHAN_2
		if(ch_id==MODBUS_CHAN_2) {
			return get_usartx_byte(MOD_IDX_2);
		} else
#endif
#ifdef MODBUS_CHAN_3
		if(ch_id==MODBUS_CHAN_3) {
			return get_usartx_byte(MOD_IDX_3);
		} else
#endif
	return 0;
}


RET_ERROR_CODE MBUS_build_dgram(MBUS_CONTROL * const pControl,uint8_t b)
{
	const uint8_t status = pControl->status;
	if (MBUS_STATUS_BEGIN == status)
	{
		mb_crc_reset(pControl->crc);
	}
	
	return AC_ERROR_OK;
}


//#ifdef MODBUS_CHAN_0
//bool MBUS_ch0_is_empty(void)
//{
	//return 	is_usartx_empty(MOD_IDX_0);
//}
//
//void MBUS_ch0_reset(void)
//{
	//reset_usartx_buffer(MOD_IDX_0);
//}
//
//uint8_t MBUS_ch0_get_byte(void)
//{
	//return get_usartx_byte(MOD_IDX_0);
//}
//
//void hal_sp336_usart0_rx_cb(const uint8_t c)
//{
	//cb_usartx(MOD_IDX_0,c);
//}
//
//#pragma message "MODBUS CH#0 RX callback binded to SP336_UART0_RX_CB"
//
//#endif
//
//#ifdef MODBUS_CHAN_1
//bool MBUS_ch1_is_empty(void)
//{
	//return 	is_usartx_empty(MOD_IDX_1);
//}
//
//void MBUS_ch1_reset(void)
//{
	//reset_usartx_buffer(MOD_IDX_1);
//}
//
//uint8_t MBUS_ch1_get_byte(void)
//{
	//return get_usartx_byte(MOD_IDX_1);
//}
//
//void hal_sp336_usart1_rx_cb(const uint8_t c)
//{
	//cb_usartx(MOD_IDX_1,c);
//}
//
//#pragma message "MODBUS CH#1 RX callback binded to SP336_UART1_RX_CB"
//
//#endif
//
//#ifdef MODBUS_CHAN_2
//bool MBUS_ch2_is_empty(void)
//{
	//return 	is_usartx_empty(MOD_IDX_2);
//}
//
//void MBUS_ch2_reset(void)
//{
	//reset_usartx_buffer(MOD_IDX_2);
//}
//
//uint8_t MBUS_ch2_get_byte(void)
//{
	//return get_usartx_byte(MOD_IDX_2);
//}
//
//void hal_sp336_usart2_rx_cb(const uint8_t c)
//{
	//cb_usartx(MOD_IDX_2,c);
//}
//
//#pragma message "MODBUS CH#2 RX callback binded to SP336_UART2_RX_CB"
//
//#endif
//
//#ifdef MODBUS_CHAN_3
//bool MBUS_ch3_is_empty(void)
//{
	//return 	is_usartx_empty(MOD_IDX_3);
//}
//
//void MBUS_ch3_reset(void)
//{
	//reset_usartx_buffer(MOD_IDX_3);
//}
//
//uint8_t MBUS_ch3_get_byte(void)
//{
	//return get_usartx_byte(MOD_IDX_3);
//}
//
//void hal_sp336_usart3_rx_cb(const uint8_t c)
//{
	//cb_usartx(MOD_IDX_3,c);
//}
//
//#pragma message "MODBUS CH#3 RX callback binded to SP336_UART3_RX_CB"
//
//#endif


