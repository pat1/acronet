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

#include "config/conf_usart_serial.h"

#include "Acronet/globals.h"
#include "Acronet/drivers/SIM/sim900.h"
#include "Acronet/drivers/UART_INT/cbuffer_usart.h"
#include "Acronet/drivers/StatusLED/status_led.h"
#include "Acronet/drivers/AT24CXX/AT24CXX.h"
#include "Acronet/drivers/Voltmeter/voltmeter.h"

#include "Acronet/Sensors/raingauge/pulse_raingauge.h"

#include "Acronet/services/fw_update/fw_update.h"
#include "Acronet/services/fw_update/hex_processor.h"
#include "Acronet/services/config/config.h"

static uint16_t g_eeBuf_idx = 0;
static uint16_t g_eeBuf_size = 0;
static const char * g_eeBUF = NULL;



static uint8_t __attribute__((const)) ascii_hex(const uint8_t c);



RET_ERROR_CODE hex_processor_init(const char * const pBuf,const uint16_t size)
{
	g_eeBuf_idx = 0;
	g_eeBuf_size = size;
	g_eeBUF = pBuf;

	return AC_ERROR_OK;

	//g_eeBuf_idx = 0;
	//g_eeBuf_size = size-8;
	//g_eeBUF = pBuf+8;

	
	//const uint8_t v8 = ascii_hex(pBuf[0]);
	//const uint8_t v7 = ascii_hex(pBuf[1]);
	//const uint8_t v6 = ascii_hex(pBuf[2]);
	//const uint8_t v5 = ascii_hex(pBuf[3]);
	//const uint8_t v4 = ascii_hex(pBuf[4]);
	//const uint8_t v3 = ascii_hex(pBuf[5]);
	//const uint8_t v2 = ascii_hex(pBuf[6]);
	//const uint8_t v1 = ascii_hex(pBuf[7]);
	//
	//const uint16_t c1 = ( (uint16_t) ((v3<<4) | v4 )<<8 )  | (v1<<4) | v2;
	//const uint16_t c2 = ( (uint16_t) ((v7<<4) | v8 )<<8 )  | (v5<<4) | v6;
	//
	//const uint32_t cc = ((uint32_t) c1) << 16 | c2;
	//
	//const uint32_t crc = crc_io_checksum(g_eeBUF,g_eeBuf_size,CRC_32BIT);
	//
	//if (crc==cc) {
		//return AC_ERROR_OK;
	//}
	
	//return AC_ERROR_GENERIC;
}



//static uint8_t hex_reader_feed_buf( void )
//{
	////if(g_hex_iter.pg>7) {return -1;}
	////AT24CXX_ReadBlock(g_hex_iter.pg,g_hex_iter.msb,0,g_eeBUF  ,256);
	////g_hex_iter.msb++;
	////if (g_hex_iter.msb==0)
	////{
		////g_hex_iter.pg++;
	////}
	//return 0;
//}

static uint8_t  hex_reader_get_char(char * const pStatus)
{
	if (g_eeBuf_idx>=g_eeBuf_size)
	{
		*pStatus = (char) EOF;
	}

	return g_eeBUF[g_eeBuf_idx++];
}

static uint8_t hex_reader_trim_to_record(char * const pStatus)
{
	char ret = hex_reader_get_char(pStatus);
	if(*pStatus==(char) EOF) return -1;
	if (ret==':') return 0;
	
	ret = hex_reader_get_char(pStatus);
	if(*pStatus==(char) EOF) return -1;
	if (ret==':') return 0;
	
	ret = hex_reader_get_char(pStatus);
	if(*pStatus==(char) EOF) return -1;
	if (ret==':') return 0;
	
	return 1;
}

uint8_t __attribute__((const)) ascii_hex(const uint8_t c)
{
	if((c>47) && (c<58)) { // 0 to 9
		return (c-48);
	} else if((c>64) && (c<71)) { // A to F
		return (c-55);
	} else if((c>96) && (c<103)) { // a to f
		return (c-87);
	}
	
	//ERROR TO HANDLE
	
	return 0xFF;
}

static uint8_t hex_reader_decode_byte(uint8_t * val, char * const pStatus)
{
	const uint8_t l1 = ascii_hex(hex_reader_get_char(pStatus));
	if(*pStatus==(char) EOF) return -1;

	const uint8_t l2 = ascii_hex(hex_reader_get_char(pStatus));
	if(*pStatus==(char) EOF) return -1;

	*val = (l1<<4) | l2;

	return 0;
}

static uint8_t hex_reader_decode_len(uint8_t * len, char * const pStatus)
{
	return hex_reader_decode_byte(len, pStatus);
}

static uint8_t hex_reader_decode_rectype(uint8_t * type, char * const pStatus)
{
	return hex_reader_decode_byte(type, pStatus);
}

static uint8_t hex_reader_decode_checksum(uint8_t * val, char * const pStatus)
{
	return hex_reader_decode_byte(val, pStatus);
}

static uint8_t hex_reader_decode_address(uint16_t * val, char * const pStatus)
{
	const uint8_t v1 = ascii_hex(hex_reader_get_char(pStatus));
	if(*pStatus==(char) EOF) return -1;
	const uint8_t v2 = ascii_hex(hex_reader_get_char(pStatus));
	if(*pStatus==(char) EOF) return -1;
	const uint8_t v3 = ascii_hex(hex_reader_get_char(pStatus));
	if(*pStatus==(char) EOF) return -1;
	const uint8_t v4 = ascii_hex(hex_reader_get_char(pStatus));
	if(*pStatus==(char) EOF) return -1;

	*val = ( (uint16_t) ((v3<<4) | v4 )<<8 )  | (v1<<4) | v2;
	
	return 0;
}


RET_ERROR_CODE hex_processor_get_rec( HEX_READER_RECORD * pRec, char * const pStatus )
{
	
	if( (hex_reader_trim_to_record(pStatus)) || (*pStatus==(char) EOF))
	{
		//Start of record missing
		return AC_NOTHING_TO_DO;
	}

	uint8_t len = 0xFF;
	hex_reader_decode_len(&len, pStatus);
	if(*pStatus==(char) EOF) return AC_ERROR_GENERIC;

	pRec->len = len;
	
	uint16_t address = 0xFFFF;
	hex_reader_decode_address(&address, pStatus);
	if(*pStatus==(char) EOF) return AC_ERROR_GENERIC;

	pRec->address = address;
	
	uint8_t type = 0xFF;
	hex_reader_decode_rectype(&type, pStatus);
	if(*pStatus==(char) EOF) return AC_ERROR_GENERIC;

	pRec->type = type;

	uint8_t * const pData = pRec->data;
	for(uint8_t idx=0;idx<len;++idx) {
		hex_reader_decode_byte(pData+idx, pStatus);
		if(*pStatus==(char) EOF) return AC_ERROR_GENERIC;
	}

	uint8_t chk;
	hex_reader_decode_checksum(&chk, pStatus);
	if(*pStatus==(char) EOF) return AC_ERROR_GENERIC;

	pRec->chk = chk;
	
	return AC_ERROR_OK;
	
}

RET_ERROR_CODE hex_processor_verify_rec(const HEX_READER_RECORD * const pRec )
{
	uint8_t i=pRec->len;
	uint8_t v = i;

	while(i--)
	{
		v  += pRec->data[i];
	}
	
	v += pRec->type;
	v += (uint8_t)(pRec->address & 0xFF);
	v += (uint8_t)((pRec->address>>8)  & 0xFF);
	v += pRec->chk;

	if (v == 0)
	{
		return AC_ERROR_OK;
	}
	
	
	return AC_ERROR_GENERIC;
}
