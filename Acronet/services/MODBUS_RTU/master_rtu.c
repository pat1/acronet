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
#include "Acronet/services/MODBUS_RTU/mb_crc.h"
#include "Acronet/services/MODBUS_RTU/master_rtu.h"

typedef struct {
	uint8_t status;

	uint8_t addr;
	uint8_t func;

	uint8_t transmission_crc[2];
} MBUS_CONTROL;


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

#ifdef MODBUS_CHAN_0 
#define MOD_DIG_0 1
#define MODBUS_CHAN_0_IDX 0
#pragma message "MODBUS CH#0 is ON"
#else
#pragma message "MODBUS CH#0 is OFF"
#define MOD_DIG_0 0
#endif

#ifdef MODBUS_CHAN_1
#define MOD_DIG_1 1
#define MODBUS_CHAN_1_IDX (0 + MOD_DIG_0)
#pragma message "MODBUS CH#1 is ON"
#else
#pragma message "MODBUS CH#1 is OFF"
#define MOD_DIG_1 0
#endif

#ifdef MODBUS_CHAN_2
#define MOD_DIG_2 1
#define MODBUS_CHAN_2_IDX (0 + MOD_DIG_0 + MOD_DIG_1)
#pragma message "MODBUS CH#2 is ON"
#else
#pragma message "MODBUS CH#2 is OFF"
#define MOD_DIG_2 0
#endif

#ifdef MODBUS_CHAN_3
#define MOD_DIG_3 1
#define MODBUS_CHAN_3_IDX (0 + MOD_DIG_0 + MOD_DIG_1 + MOD_DIG_2)
#pragma message "MODBUS CH#3 is ON"
#else
#pragma message "MODBUS CH#3 is OFF"
#define MOD_DIG_3 0
#endif


#define MODBUS_CHANNELS (0 + MOD_DIG_0 + MOD_DIG_1 + MOD_DIG_2 + MOD_DIG_3)
#define MODBUS_UART_BUF_SIZE 32

#if (MODBUS_CHANNELS == 0)
#pragma message "MODBUS CHANNELS 0"
#elif (MODBUS_CHANNELS == 1)
#pragma message "MODBUS CHANNELS 1"
#elif (MODBUS_CHANNELS == 2)
#pragma message "MODBUS CHANNELS 2"
#elif (MODBUS_CHANNELS == 3)
#pragma message "MODBUS CHANNELS 3"
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

#if defined(SP336_USART2) || defined(SP336_USART3)
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

#ifdef MODBUS_CHAN_0_PUT
#error "A symbol MODBUS_CHAN_0_PUT has been defined elsewhere"
#endif

#ifdef MODBUS_CHAN_1_PUT
#error "A symbol MODBUS_CHAN_1_PUT has been defined elsewhere"
#endif

#ifdef MODBUS_CHAN_2_PUT
#error "A symbol MODBUS_CHAN_2_PUT has been defined elsewhere"
#endif

#ifdef MODBUS_CHAN_3_PUT
#error "A symbol MODBUS_CHAN_3_PUT has been defined elsewhere"
#endif

#ifdef MODBUS_CHAN_0
#undef MODBUS_CHAN_0_PUT
#undef MODBUS_CHAN_0_CB
#undef MBUS_CHAN_0_USART
#if(0==MOD_IDX_0)
#define MODBUS_CHAN_0_PUT SP336_0_PutBuffer
#define MODBUS_CHAN_0_CB  SP336_USART0_RX_Vect
#define MODBUS_CHAN_0_USART SP336_USART0
#elif(1==MOD_IDX_0)
#define MODBUS_CHAN_0_PUT SP336_1_PutBuffer
#define MODBUS_CHAN_0_CB  SP336_USART1_RX_Vect
#define MODBUS_CHAN_0_USART SP336_USART1
#elif(2==MOD_IDX_0)
#define MODBUS_CHAN_0_PUT SP336_2_PutBuffer
#define MODBUS_CHAN_0_CB  SP336_USART2_RX_Vect
#define MODBUS_CHAN_0_USART SP336_USART2
#elif(3==MOD_IDX_0)
#define MODBUS_CHAN_0_PUT SP336_3_PutBuffer
#define MODBUS_CHAN_0_CB  SP336_USART3_RX_Vect
#define MODBUS_CHAN_0_USART SP336_USART3
#endif
#endif //MODBUS_CHAN_0

#ifdef MODBUS_CHAN_1
#undef MODBUS_CHAN_1_PUT
#undef MODBUS_CHAN_1_CB
#undef MODBUS_CHAN_1_USART
#if(0==MOD_IDX_0)
#define MODBUS_CHAN_1_PUT SP336_0_PutBuffer
#define MODBUS_CHAN_1_CB  SP336_USART0_RX_Vect
#define MODBUS_CHAN_1_USART SP336_USART0
#elif(1==MOD_IDX_0)
#define MODBUS_CHAN_1_PUT SP336_1_PutBuffer
#define MODBUS_CHAN_1_CB  SP336_USART1_RX_Vect
#define MODBUS_CHAN_1_USART SP336_USART1
#elif(2==MOD_IDX_0)
#define MODBUS_CHAN_1_PUT SP336_2_PutBuffer
#define MODBUS_CHAN_1_CB  SP336_USART2_RX_Vect
#define MODBUS_CHAN_1_USART SP336_USART2
#elif(3==MOD_IDX_0)
#define MODBUS_CHAN_1_PUT SP336_3_PutBuffer
#define MODBUS_CHAN_1_CB  SP336_USART3_RX_Vect
#define MODBUS_CHAN_1_USART SP336_USART3
#endif
#endif //MODBUS_CHAN_1

#ifdef MODBUS_CHAN_2
#undef MODBUS_CHAN_2_PUT
#undef MODBUS_CHAN_2_CB
#undef MODBUS_CHAN_2_USART
#if(0==MOD_IDX_0)
#define MODBUS_CHAN_2_PUT SP336_0_PutBuffer
#define MODBUS_CHAN_2_CB  SP336_USART0_RX_Vect
#define MODBUS_CHAN_2_USART SP336_USART0
#elif(1==MOD_IDX_0)
#define MODBUS_CHAN_2_PUT SP336_1_PutBuffer
#define MODBUS_CHAN_2_CB  SP336_USART1_RX_Vect
#define MODBUS_CHAN_2_USART SP336_USART1
#elif(2==MOD_IDX_0)
#define MODBUS_CHAN_2_PUT SP336_2_PutBuffer
#define MODBUS_CHAN_2_CB  SP336_USART2_RX_Vect
#define MODBUS_CHAN_2_USART SP336_USART2
#elif(3==MOD_IDX_0)
#define MODBUS_CHAN_2_PUT SP336_3_PutBuffer
#define MODBUS_CHAN_2_CB  SP336_USART3_RX_Vect
#define MODBUS_CHAN_2_USART SP336_USART3
#endif
#endif //MODBUS_CHAN_2


#ifdef MODBUS_CHAN_3
#undef MODBUS_CHAN_3_PUT
#undef MODBUS_CHAN_3_CB
#undef MODBUS_CHAN_3_USART
#if(0==MOD_IDX_0)
#define MODBUS_CHAN_3_PUT SP336_0_PutBuffer
#define MODBUS_CHAN_3_CB  SP336_USART0_RX_Vect
#define MODBUS_CHAN_3_USART SP336_USART0
#elif(1==MOD_IDX_0)
#define MODBUS_CHAN_3_PUT SP336_1_PutBuffer
#define MODBUS_CHAN_3_CB  SP336_USART1_RX_Vect
#define MODBUS_CHAN_3_USART SP336_USART1
#elif(2==MOD_IDX_0)
#define MODBUS_CHAN_3_PUT SP336_2_PutBuffer
#define MODBUS_CHAN_3_CB  SP336_USART2_RX_Vect
#define MODBUS_CHAN_3_USART SP336_USART2
#elif(3==MOD_IDX_0)
#define MODBUS_CHAN_3_PUT SP336_3_PutBuffer
#define MODBUS_CHAN_3_CB  SP336_USART3_RX_Vect
#define MODBUS_CHAN_3_USART SP336_USART3
#endif
#endif //MODBUS_CHAN_3



//RET_ERROR_CODE MBUS_issue_cmd(const uint8_t ch_id,const uint8_t * const pBuf,uint16_t len)
//{
//#ifdef MODBUS_CHAN_0
	//if(ch_id==MODBUS_CHAN_0) {
		//return MODBUS_CHAN_0_PUT(pBuf,len);
	//} else
//#endif
//#ifdef MODBUS_CHAN_1
	//if(ch_id==MODBUS_CHAN_1) {
		//return MODBUS_CHAN_1_PUT(pBuf,len);
	//} else
//#endif
//#ifdef MODBUS_CHAN_2
	//if(ch_id==MODBUS_CHAN_2) {
		//return MODBUS_CHAN_2_PUT(pBuf,len);
	//} else
//#endif
//#ifdef MODBUS_CHAN_3
	//if(ch_id==MODBUS_CHAN_3) {
		//return MODBUS_CHAN_3_PUT(pBuf,len);
	//} else
//#endif
	//return AC_UNSUPPORTED;
//}
/*
bool MBUS_is_empty(const uint8_t ch_id)
{

#ifdef MODBUS_CHAN_0
		if(ch_id==MODBUS_CHAN_0) {
			return is_usartx_empty(MODBUS_CHAN_0_IDX);
		} else
#endif
#ifdef MODBUS_CHAN_1
		if(ch_id==MODBUS_CHAN_1) {
			return is_usartx_empty(MODBUS_CHAN_1_IDX);
	} else
#endif
#ifdef MODBUS_CHAN_2
		if(ch_id==MODBUS_CHAN_2) {
			return is_usartx_empty(MODBUS_CHAN_2_IDX);
		} else
#endif
#ifdef MODBUS_CHAN_3
		if(ch_id==MODBUS_CHAN_3) {
			return is_usartx_empty(MODBUS_CHAN_3_IDX);
		} else
#endif		
	return true;
}

*/

//void MBUS_reset(const uint8_t ch_id)
//{	
//#ifdef MODBUS_CHAN_0
		//if(ch_id==MODBUS_CHAN_0) {
			//reset_usartx_buffer(MODBUS_CHAN_0_IDX);
		//} else
//#endif
//#ifdef MODBUS_CHAN_1
		//if(ch_id==MODBUS_CHAN_1) {
			//reset_usartx_buffer(MODBUS_CHAN_1_IDX);
		//} else
//#endif
//#ifdef MODBUS_CHAN_2
		//if(ch_id==MODBUS_CHAN_2) {
			//reset_usartx_buffer(MODBUS_CHAN_2_IDX);
		//} else
//#endif
//#ifdef MODBUS_CHAN_2
		//if(ch_id==MODBUS_CHAN_3) {
			//reset_usartx_buffer(MODBUS_CHAN_3_IDX);
		//} else
//#endif
	//return;	
//}

//uint8_t MBUS_get_byte(const uint8_t ch_id)
//{
//#ifdef MODBUS_CHAN_0
		//if(ch_id==MODBUS_CHAN_0) {
			//return get_usartx_byte(MODBUS_CHAN_0_IDX);
		//} else
//#endif
//#ifdef MODBUS_CHAN_1
		//if(ch_id==MODBUS_CHAN_1) {
			//return get_usartx_byte(MODBUS_CHAN_1_IDX);
		//} else
//#endif
//#ifdef MODBUS_CHAN_2
		//if(ch_id==MODBUS_CHAN_2) {
			//return get_usartx_byte(MODBUS_CHAN_2_IDX);
		//} else
//#endif
//#ifdef MODBUS_CHAN_3
		//if(ch_id==MODBUS_CHAN_3) {
			//return get_usartx_byte(MODBUS_CHAN_3_IDX);
		//} else
//#endif
	//return 0;
//}


uint8_t MBUS_build_dgram(MBUS_CONTROL * const pControl,MBUS_PDU * const pPDU,const uint8_t b)
{
	const uint8_t status = pControl->status;
	
	if (MBUS_STATUS_BEGIN == status)
	{
		pPDU->data.bc = 0;
		mb_crc_reset(pControl->transmission_crc);
	}
	
//	char szBUF[256];

	switch(pControl->status) {
		case MBUS_STATUS_ADDR:
			mb_crc_push(pControl->transmission_crc, b);
			pPDU->addr = b;
//			sprintf_P(szBUF,PSTR("MB ADDRESS %02X\r\n"),b);
			if (b!=pControl->addr)
			{
				pControl->status = MBUS_STATUS_END;
				break;
			}
					
			pControl->status = MBUS_STATUS_FUNC;
		break;
		case MBUS_STATUS_FUNC:
			mb_crc_push(pControl->transmission_crc, b);
			pPDU->func = b;
//			sprintf_P(szBUF,PSTR("MB FUNC %02X\r\n"),b);
			pControl->status = MBUS_STATUS_DATA_BYTE_COUNT;
		break;
		case MBUS_STATUS_DATA_BYTE_COUNT:
			mb_crc_push(pControl->transmission_crc, b);
			pPDU->data_size = b;
//			sprintf_P(szBUF,PSTR("MB BYTES %02X\r\n"),b);
			pControl->status = MB_STATUS_DATA_BYTE;
		break;
		case MB_STATUS_DATA_BYTE:
			mb_crc_push(pControl->transmission_crc, b);
			pPDU->data.byte[pPDU->data.bc++] = b;
			if (pPDU->data.bc >= pPDU->data_size)
			{
//				sprintf_P(szBUF,PSTR("%02X\r\n"),b);
				pControl->status = MBUS_STATUS_CRC_HI;
			} //else {
//				sprintf_P(szBUF,PSTR("%02X "),b);
//			}
		break;
		case MBUS_STATUS_CRC_HI:
			pPDU->crc_hi  = b;
//			sprintf_P(szBUF,PSTR("CRC HI %02X\r\n"),b);
			pControl->status = MBUS_STATUS_CRC_LO;
		break;
		case MBUS_STATUS_CRC_LO:
			pPDU->crc_lo  = b;
//			sprintf_P(szBUF,PSTR("CRC LO %02X\r\n"),b);
			pControl->status = MBUS_STATUS_END;
		break;
		default:
		break;
	}

//	debug_string(NORMAL,szBUF,RAM_STRING);

	
	return pControl->status;
}



#ifdef MODBUS_CHAN_0

bool MBUS_is_empty_CH0(void)
{
	return is_usartx_empty(MODBUS_CHAN_0_IDX);	
}

uint8_t MBUS_get_byte_CH0(void)
{
	return get_usartx_byte(MODBUS_CHAN_0_IDX);
}

static MBUS_CONTROL g_bc0 = {.status = MBUS_STATUS_BEGIN};

RET_ERROR_CODE MBUS_issue_cmd_CH0(const uint8_t * const pBuf,uint16_t len)
{
	g_bc0.addr = pBuf[0];
	g_bc0.func = pBuf[1];
	
	return MODBUS_CHAN_0_PUT(pBuf,len);
}

RET_ERROR_CODE MBUS_lock_CH0(void)
{
	if (g_bc0.status!=MBUS_STATUS_BEGIN )
	{
		debug_string_1P(NORMAL,PSTR("MBUS CH0 BUSY"));
		return AC_ERROR_GENERIC;
	}
	
	return AC_ERROR_OK;
}

void MBUS_release_CH0(void)
{
	g_bc0.status = MBUS_STATUS_BEGIN;
}


uint8_t MBUS_build_dgram_CH0(MBUS_PDU * const pPDU,uint8_t b)
{
	return MBUS_build_dgram(&g_bc0,pPDU,b);
}

uint16_t MBUS_get_crc_CH0(void)
{
	return mb_crc_get(g_bc0.transmission_crc);
}

ISR(MODBUS_CHAN_0_CB)
{
	//usart_putchar(USART_DEBUG,'.');
	cb_usartx(MODBUS_CHAN_0_IDX,MODBUS_CHAN_0_USART.DATA);
}
#endif



#ifdef MODBUS_CHAN_1

bool MBUS_is_empty_CH1(void)
{
	return is_usartx_empty(MODBUS_CHAN_1_IDX);
}

uint8_t MBUS_get_byte_CH1(void)
{
	return get_usartx_byte(MODBUS_CHAN_1_IDX);
}

static MBUS_CONTROL g_bc1 = {.status = MBUS_STATUS_BEGIN};

RET_ERROR_CODE MBUS_issue_cmd_CH1(const uint8_t * const pBuf,uint16_t len)
{
	g_bc1.addr = pBuf[0];
	g_bc1.func = pBuf[1];
	
	return MODBUS_CHAN_1_PUT(pBuf,len);
}


RET_ERROR_CODE MBUS_lock_CH1(void)
{
	if (g_bc1.status!=MBUS_STATUS_BEGIN )
	{
		debug_string_1P(NORMAL,PSTR("MBUS CH1 BUSY"));
		return AC_ERROR_GENERIC;
	}
	
	return AC_ERROR_OK;
}

void MBUS_release_CH1(void)
{
	g_bc1.status = MBUS_STATUS_BEGIN;
}


uint8_t MBUS_build_dgram_CH1(MBUS_PDU * const pPDU,uint8_t b)
{
	return MBUS_build_dgram(&g_bc1,pPDU,b);
}

uint16_t MBUS_get_crc_CH1(void)
{
	return mb_crc_get(g_bc1.transmission_crc);
}


ISR(MODBUS_CHAN_1_CB)
{
	cb_usartx(MODBUS_CHAN_1_IDX,MODBUS_CHAN_1_USART.DATA);
}
#endif


#ifdef MODBUS_CHAN_2

bool MBUS_is_empty_CH2(void)
{
	return is_usartx_empty(MODBUS_CHAN_2_IDX);
}

uint8_t MBUS_get_byte_CH2(void)
{
	return get_usartx_byte(MODBUS_CHAN_2_IDX);
}

static MBUS_CONTROL g_bc2 = {.status = MBUS_STATUS_BEGIN};

RET_ERROR_CODE MBUS_issue_cmd_CH2(const uint8_t * const pBuf,uint16_t len)
{
	g_bc2.addr = pBuf[0];
	g_bc2.func = pBuf[1];
	
	return MODBUS_CHAN_2_PUT(pBuf,len);
}

RET_ERROR_CODE MBUS_lock_CH2(void)
{
	if (g_bc2.status!=MBUS_STATUS_BEGIN )
	{
		debug_string_1P(NORMAL,PSTR("MBUS CH2 BUSY"));
		return AC_ERROR_GENERIC;
	}
	
	return AC_ERROR_OK;
}

void MBUS_release_CH2(void)
{
	g_bc2.status = MBUS_STATUS_BEGIN;
}


uint8_t MBUS_build_dgram_CH2(MBUS_PDU * const pPDU,uint8_t b)
{
	return MBUS_build_dgram(&g_bc2,pPDU,b);
}

uint16_t MBUS_get_crc_CH2(void)
{
	return mb_crc_get(g_bc2.transmission_crc);
}

ISR(MODBUS_CHAN_2_CB)
{
	cb_usartx(MODBUS_CHAN_2_IDX,MODBUS_CHAN_2_USART.DATA);
}
#endif




#ifdef MODBUS_CHAN_3

bool MBUS_is_empty_CH3(void)
{
	return is_usartx_empty(MODBUS_CHAN_3_IDX);
}

uint8_t MBUS_get_byte_CH3(void)
{
	return get_usartx_byte(MODBUS_CHAN_3_IDX);
}


static MBUS_CONTROL g_bc3 = {.status = MBUS_STATUS_BEGIN};

RET_ERROR_CODE MBUS_issue_cmd_CH3(const uint8_t * const pBuf,uint16_t len)
{
	g_bc3.addr = pBuf[0];
	g_bc3.func = pBuf[1];
	
	return MODBUS_CHAN_3_PUT(pBuf,len);
}

RET_ERROR_CODE MBUS_lock_CH3(void)
{
	if (g_bc3.status!=MBUS_STATUS_BEGIN )
	{
		debug_string_1P(NORMAL,PSTR("MBUS CH3 BUSY"));
		return AC_ERROR_GENERIC;
	}
	
	return AC_ERROR_OK;
}

void MBUS_release_CH3(void)
{
	g_bc3.status = MBUS_STATUS_BEGIN;
}


uint8_t MBUS_build_dgram_CH3(MBUS_PDU * const pPDU,uint8_t b)
{
	return MBUS_build_dgram(&g_bc3,pPDU,b);
}

uint16_t MBUS_get_crc_CH3(void)
{
	return mb_crc_get(g_bc3.transmission_crc);
}

ISR(MODBUS_CHAN_3_CB)
{
	cb_usartx(MODBUS_CHAN_3_IDX,MODBUS_CHAN_3_USART.DATA);
}
#endif




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


