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
//#include "Acronet/globals.h"
#include "board.h"
#include "sysclk.h"
#include "twi_master.h"
//#include "led.h"

#include "conf_board.h"
#include "Acronet/drivers/MPL3115A2/MPL3115A2.h"




static RET_ERROR_CODE MPL3115A2_internalWrite(twi_package_t * ppak)
{
	static const char __flash funName[] = "MPL3115A2_internalWrite";
	
	//debug_string("Before write\r\n");
	const status_code_t r = twi_master_write(MPL3115A2_TWI_PORT, ppak);

	switch(r) {
		case TWI_SUCCESS:
		//debug_string_P(NORMAL,PSTR("Write Succeeded\r\n"));
		return AC_ERROR_OK;
		break;
		
		case ERR_IO_ERROR:
		debug_string_2P(NORMAL,funName,PSTR("ERR_IO_ERROR"));
		return AC_AT24C_IO_ERROR;
		break;
		
		case ERR_FLUSHED:
		debug_string_2P(NORMAL,funName,PSTR("ERR_FLUSHED"));
		return AC_AT24C_FLUSHED;
		break;
		
		case ERR_TIMEOUT:
		debug_string_2P(NORMAL,funName,PSTR("ERR_TIMEOUT"));
		return AC_AT24C_TIMEOUT;
		break;
		
		case ERR_BAD_DATA:
		debug_string_2P(NORMAL,funName,PSTR("ERR_BAD_DATA"));
		return AC_AT24C_BAD_DATA;
		break;
		
		case ERR_PROTOCOL:
		debug_string_2P(NORMAL,funName,PSTR("ERR_PROTOCOL"));
		return AC_AT24C_PROTOCOL;
		break;
		
		case ERR_UNSUPPORTED_DEV:
		debug_string_2P(NORMAL,funName,PSTR("ERR_UNSUPPORTED_DEV"));
		return AC_AT24C_UNSOPPORTED_DEV;
		break;
		
		case ERR_NO_MEMORY:
		debug_string_2P(NORMAL,funName,PSTR("ERR_NO_MEMORY"));
		return AC_AT24C_NO_MEMORY;
		break;
		
		case ERR_INVALID_ARG:
		debug_string_2P(NORMAL,funName,PSTR("ERR_INVALID_ARG"));
		return AC_AT24C_INVALID_ARG;
		break;
		
		case ERR_BAD_ADDRESS:
		debug_string_2P(NORMAL,funName,PSTR("ERR_BAD_ADDRESS"));
		return AC_AT24C_BAD_ADDRESS;
		break;
		
		case ERR_BUSY:
		debug_string_2P(NORMAL,funName,PSTR("ERR_BUSY"));
		return AC_AT24C_BUSY;
		break;
		
		case ERR_BAD_FORMAT:
		debug_string_2P(NORMAL,funName,PSTR("ERR_BAD_FORMAT"));
		return AC_AT24C_BAD_FORMAT;
		break;
		default:
		debug_string_2P(NORMAL,funName,PSTR("UNKONWN ERROR"));
	}
	return AC_UNSUPPORTED;
}

static RET_ERROR_CODE MPL3115A2_internalRead(twi_package_t * ppak)
{
	const char * const funName = PSTR("MPL3115A2_internalRead");

	const status_code_t r = twi_master_read(MPL3115A2_TWI_PORT, ppak);
	switch(r) {
		case TWI_SUCCESS:
		//debug_string_P(NORMAL,PSTR("READ Succeeded\r\n"));
		return AC_ERROR_OK;
		break;
		case ERR_IO_ERROR:
		debug_string_2P(NORMAL,funName,PSTR("ERR_IO_ERROR"));
		return AC_AT24C_IO_ERROR;
		break;
		case ERR_FLUSHED:
		debug_string_2P(NORMAL,funName,PSTR("ERR_FLUSHED"));
		return AC_AT24C_FLUSHED;
		break;
		case ERR_TIMEOUT:
		debug_string_2P(NORMAL,funName,PSTR("ERR_TIMEOUT"));
		return AC_AT24C_TIMEOUT;
		break;
		case ERR_BAD_DATA:
		debug_string_2P(NORMAL,funName,PSTR("ERR_BAD_DATA"));
		return AC_AT24C_BAD_DATA;
		break;
		case ERR_PROTOCOL:
		debug_string_2P(NORMAL,funName,PSTR("ERR_PROTOCOL"));
		return AC_AT24C_PROTOCOL;
		break;
		case ERR_UNSUPPORTED_DEV:
		debug_string_2P(NORMAL,funName,PSTR("ERR_UNSUPPORTED_DEV"));
		return AC_AT24C_UNSOPPORTED_DEV;
		break;
		case ERR_NO_MEMORY:
		debug_string_2P(NORMAL,funName,PSTR("ERR_NO_MEMORY"));
		return AC_AT24C_NO_MEMORY;
		break;
		case ERR_INVALID_ARG:
		debug_string_2P(NORMAL,funName,PSTR("ERR_INVALID_ARG"));
		return AC_AT24C_INVALID_ARG;
		break;
		case ERR_BAD_ADDRESS:
		debug_string_2P(NORMAL,funName,PSTR("ERR_BAD_ADDRESS"));
		return AC_AT24C_BAD_ADDRESS;
		break;
		case ERR_BUSY:
		debug_string_2P(NORMAL,funName,PSTR("ERR_BUSY"));
		return AC_AT24C_BUSY;
		break;
		case ERR_BAD_FORMAT:
		debug_string_2P(NORMAL,funName,PSTR("ERR_BAD_FORMAT"));
		return AC_AT24C_BAD_FORMAT;
		break;
		default:
		debug_string_2P(NORMAL,funName,PSTR("UNKONWN ERROR"));

	}
	return AC_UNSUPPORTED;
}


RET_ERROR_CODE MPL3115A2_Write( const uint8_t cmd, const uint8_t data )
{
	twi_package_t packet = {
		.addr[0]	  = cmd,
		.addr[1]	  = data,
		.addr_length  = 2,     // TWI slave memory address data size
		.chip         = MPL3115A2_CHIP_ADDR ,      // TWI slave bus address
		.buffer       = NULL, // transfer data source buffer
		.length       = 0// transfer data size (bytes)
	};

	return MPL3115A2_internalWrite(&packet);

	return AC_ERROR_OK;
}

RET_ERROR_CODE MPL3115A2_Read( const uint8_t cmd, uint8_t * const data, const uint8_t len )
{
	twi_package_t packet = {
		.addr[0]	  = cmd,
		.addr_length  = 1,     // TWI slave memory address data size
		.chip         = MPL3115A2_CHIP_ADDR ,      // TWI slave bus address
		.buffer       = data, // transfer data source buffer
		.length       = len// transfer data size (bytes)
	};

	return MPL3115A2_internalRead(&packet);

	return AC_ERROR_OK;
}
