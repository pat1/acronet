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
#include "Acronet/HAl/hal_interface.h"


#include <asf.h>
#include <stdio.h>
#include "Acronet/globals.h"
#include "board.h"
#include "sysclk.h"
#include "twi_master.h"
//#include "led.h"

#include "conf_board.h"
#include "SHT21.h"

static status_code_t SHT21_internalWrite(twi_package_t * ppak);
static status_code_t SHT21_internalRead(twi_package_t * ppak);

#define SHT21_TWI_ADDR  0x40

int SHT21_Init(void) 
{
		// TWI master initialization options.
		
		twi_master_options_t opt = {
			.speed = 50000,
			.chip  = TWI_MASTER_ADDRESS,
		};
		
		// Initialize the TWI master driver.
		
		return twi_master_setup(AUX_TWI_PORT, &opt);
}


status_code_t SHT21_TriggerReadTemp_hold(SHTVAL *val)
{
	
	uint8_t cmd_buff = 0b11100011;

	//debug_string_1P(VERBOSE,PSTR("[SHT21_TriggerReadTemp_hold] IN\r\n"));

//
	//
	//twi_package_t pak = {
		//.addr_length  = 0,     
		//.chip         = SHT21_TWI_ADDR,
		//.buffer       = (void *)&cmd_buff,
		//.length       = 1
	//};
//
	//SHT21_internalWrite(&pak);
	//
	//debug_string(VERBOSE,"[SHT21_TriggerReadTemp_hold] step 1 ok\r\n");
		//
	//pak.addr_length = 0;
	//pak.chip = SHT21_TWI_ADDR;
	//pak.buffer = pVal;
	//pak.length = 2;
//
	//return SHT21_internalRead(&pak);


	twi_package_t pak = {
		.addr[0]	  = cmd_buff,
		.addr_length  = 1,
		.chip         = SHT21_TWI_ADDR,
		.buffer       = (void *)val,
		.length       = 2
	};

	return SHT21_internalRead(&pak);

	
}

status_code_t SHT21_TriggerReadTemp_noHold(SHTVAL * val)
{

	uint8_t cmd_buff = 0b11110011;

	//debug_string_1P(VERBOSE,PSTR("[SHT21_TriggerReadTemp_noHold] IN\r\n"));


	
	twi_package_t pak = {
		.addr_length  = 0,
		.chip         = SHT21_TWI_ADDR,
		.buffer       = (void *)&cmd_buff,
		.length       = 1
	};


	SHT21_internalWrite(&pak);
	
	//debug_string_1P(VERBOSE,PSTR("[SHT21_TriggerReadTemp_noHold] step 1 ok\r\n"));
	//delay_ms(10);
	
	pak.addr_length = 0;
	pak.chip = SHT21_TWI_ADDR;
	pak.buffer = val;
	pak.length = 2;

	return SHT21_internalRead(&pak);
	
}


status_code_t SHT21_TriggerReadRH_hold(SHTVAL * val)
{

	uint8_t cmd_buff = 0b11100101;

	
	twi_package_t pak = {
		.addr_length  = 0,
		.chip         = SHT21_TWI_ADDR,
		.buffer       = (void *)&cmd_buff,
		.length       = 1
	};

	SHT21_internalWrite(&pak);
	
	pak.addr_length = 0;
	pak.chip = SHT21_TWI_ADDR;
	pak.buffer = val;
	pak.length = 2;

	return SHT21_internalRead(&pak);
	
}



static status_code_t SHT21_internalWrite(twi_package_t * ppak)
{
	static const char __flash funName[] = "SHT21_internalWrite";
	//debug_string("Before write\r\n");
	const status_code_t r = twi_master_write(AUX_TWI_PORT, ppak);
	switch(r) {
		case TWI_SUCCESS:
		//debug_string_2P(VERBOSE,funName,PSTR("Write Succeeded"));
		return r;
		break;
		case ERR_IO_ERROR:
		debug_string_2P(NORMAL,funName,PSTR("ERR_IO_ERROR"));
		break;
		case ERR_FLUSHED:
		debug_string_2P(NORMAL,funName,PSTR("ERR_FLUSHED"));
		break;
		case ERR_TIMEOUT:
		debug_string_2P(NORMAL,funName,PSTR("ERR_TIMEOUT"));
		break;
		case ERR_BAD_DATA:
		debug_string_2P(NORMAL,funName,PSTR("ERR_BAD_DATA"));
		break;
		case ERR_PROTOCOL:
		debug_string_2P(NORMAL,funName,PSTR("ERR_PROTOCOL"));
		break;
		case ERR_UNSUPPORTED_DEV:
		debug_string_2P(NORMAL,funName,PSTR("ERR_UNSUPPORTED_DEV"));
		break;
		case ERR_NO_MEMORY:
		debug_string_2P(NORMAL,funName,PSTR("ERR_NO_MEMORY"));
		break;
		case ERR_INVALID_ARG:
		debug_string_2P(NORMAL,funName,PSTR("ERR_INVALID_ARG"));
		break;
		case ERR_BAD_ADDRESS:
		debug_string_2P(NORMAL,funName,PSTR("ERR_BAD_ADDRESS"));
		break;
		case ERR_BUSY:
		debug_string_2P(NORMAL,funName,PSTR("ERR_BUSY"));
		break;
		case ERR_BAD_FORMAT:
		debug_string_2P(NORMAL,funName,PSTR("ERR_BAD_FORMAT"));
		break;
		default:
		debug_string_2P(NORMAL,funName,PSTR("UNKONWN ERROR"));
	}
	return r;
}

static status_code_t SHT21_internalRead(twi_package_t * ppak)
{
	static const char __flash funName[] = "SHT21_internalRead";

	const status_code_t r = twi_master_read(AUX_TWI_PORT, ppak);
	switch(r) {
		case TWI_SUCCESS:
		//debug_string(NORMAL,PSTR("READ Succeeded\r\n"));
		return r;
		break;
		case ERR_IO_ERROR:
		debug_string_2P(NORMAL,funName,PSTR("ERR_IO_ERROR"));
		break;
		case ERR_FLUSHED:
		debug_string_2P(NORMAL,funName,PSTR("ERR_FLUSHED"));
		break;
		case ERR_TIMEOUT:
		debug_string_2P(NORMAL,funName,PSTR("ERR_TIMEOUT"));
		break;
		case ERR_BAD_DATA:
		debug_string_2P(NORMAL,funName,PSTR("ERR_BAD_DATA"));
		break;
		case ERR_PROTOCOL:
		debug_string_2P(NORMAL,funName,PSTR("ERR_PROTOCOL"));
		break;
		case ERR_UNSUPPORTED_DEV:
		debug_string_2P(NORMAL,funName,PSTR("ERR_UNSUPPORTED_DEV"));
		break;
		case ERR_NO_MEMORY:
		debug_string_2P(NORMAL,funName,PSTR("ERR_NO_MEMORY"));
		break;
		case ERR_INVALID_ARG:
		debug_string_2P(NORMAL,funName,PSTR("ERR_INVALID_ARG"));
		break;
		case ERR_BAD_ADDRESS:
		debug_string_2P(NORMAL,funName,PSTR("ERR_BAD_ADDRESS"));
		break;
		case ERR_BUSY:
		debug_string_2P(NORMAL,funName,PSTR("ERR_BUSY"));
		break;
		case ERR_BAD_FORMAT:
		debug_string_2P(NORMAL,funName,PSTR("ERR_BAD_FORMAT"));
		break;
		default:
		debug_string_2P(NORMAL,funName,PSTR("UNKONWN ERROR"));

	}
	return r;
}
