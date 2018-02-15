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
//#include "Acronet/globals.h"
#include "board.h"
#include "sysclk.h"
#include "twi_master.h"
//#include "led.h"

#include "conf_board.h"
#include "AT24CXX.h"

RET_ERROR_CODE AT24CXX_Init()
{
		// TWI master initialization options.
		
		twi_master_options_t opt = {
			.speed = EEPROM_TWI_SPEED,
			.chip  = TWI_MASTER_ADDRESS,
		};
		

		const int e = twi_master_setup(EEPROM_TWI_PORT, &opt); 

		if (STATUS_OK==e)
		{
			return AC_ERROR_OK;
		} 

		return AC_UNSUPPORTED;
}

static void TWI_report_error(twi_package_t * ppak)
{
	char szBuf[256];
	
	int p[6] = {	[0] = ppak->addr_length, 
					[1] = ppak->addr[0], 
					[2] = ppak->addr[1],
					[3] = ppak->addr[2],
					[4] = ppak->chip,
					[5] = ppak->length			};
	
	snprintf_P(szBuf,sizeof(szBuf),PSTR("\r\n*** BEGIN TWI PACKET***\r\n"
										 "ADDRESS_Size\t:\t%d\r\n"
										 "ADDRESS[0]\t:\t%d\r\n"
										 "ADDRESS[1]\t:\t%d\r\n"
										 "ADDRESS[2]\t:\t%d\r\n"
										 "CHIP\t:\t%d\r\n"
										 "BUFLEN\t:\t%d\r\n"
										 "*** END TWI PACKET***\r\n"),p[0],p[1],p[2],p[3],p[4],p[5]);
						 
						 
}

static RET_ERROR_CODE AT24CXX_internalWrite(twi_package_t * ppak)
{
	static const char __flash funName[] = "AT24CXX_internalWrite";
	
	//debug_string("Before write\r\n");
	const status_code_t r = twi_master_write(EEPROM_TWI_PORT, ppak);
	RET_ERROR_CODE err = AC_UNSUPPORTED;
	
	switch(r) {
		case TWI_SUCCESS:
			//debug_string_P(NORMAL,PSTR("Write Succeeded\r\n"));
			return AC_ERROR_OK;
		break;
		
		case ERR_IO_ERROR:
			debug_string_2P(NORMAL,funName,PSTR("ERR_IO_ERROR"));
			err = AC_AT24C_IO_ERROR;
		break;
		
		case ERR_FLUSHED:
			debug_string_2P(NORMAL,funName,PSTR("ERR_FLUSHED"));
			err = AC_AT24C_FLUSHED;
		break;
		
		case ERR_TIMEOUT:
			debug_string_2P(NORMAL,funName,PSTR("ERR_TIMEOUT"));
			err =  AC_AT24C_TIMEOUT;
		break;
		
		case ERR_BAD_DATA:
			debug_string_2P(NORMAL,funName,PSTR("ERR_BAD_DATA"));
			err =  AC_AT24C_BAD_DATA;
		break;
		
		case ERR_PROTOCOL:
			debug_string_2P(NORMAL,funName,PSTR("ERR_PROTOCOL"));
			err =  AC_AT24C_PROTOCOL;
		break;
		
		case ERR_UNSUPPORTED_DEV:
			debug_string_2P(NORMAL,funName,PSTR("ERR_UNSUPPORTED_DEV"));
			err =  AC_AT24C_UNSOPPORTED_DEV;
		break;
		
		case ERR_NO_MEMORY:
			debug_string_2P(NORMAL,funName,PSTR("ERR_NO_MEMORY"));
			err =  AC_AT24C_NO_MEMORY;
		break;
		
		case ERR_INVALID_ARG:
			debug_string_2P(NORMAL,funName,PSTR("ERR_INVALID_ARG"));
			err =  AC_AT24C_INVALID_ARG;
		break;
		
		case ERR_BAD_ADDRESS:
			debug_string_2P(NORMAL,funName,PSTR("ERR_BAD_ADDRESS"));
			err =  AC_AT24C_BAD_ADDRESS;
		break;
		
		case ERR_BUSY:
			debug_string_2P(NORMAL,funName,PSTR("ERR_BUSY"));
			err = AC_AT24C_BUSY;
		break;
		
		case ERR_BAD_FORMAT:
			debug_string_2P(NORMAL,funName,PSTR("ERR_BAD_FORMAT"));
			err =  AC_AT24C_BAD_FORMAT;
		break;
		default:
		debug_string_2P(NORMAL,funName,PSTR("UNKONWN ERROR"));
	}
	TWI_report_error(ppak);
	return err;
}

static RET_ERROR_CODE AT24CXX_internalRead(twi_package_t * ppak)
{
	const char * const funName = PSTR("AT24CXX_internalRead");
	RET_ERROR_CODE err = AC_UNSUPPORTED;

	const status_code_t r = twi_master_read(EEPROM_TWI_PORT, ppak);
	switch(r) {
		case TWI_SUCCESS:
		//debug_string_P(NORMAL,PSTR("READ Succeeded\r\n"));
		return AC_ERROR_OK;
		break;
		case ERR_IO_ERROR:
			debug_string_2P(NORMAL,funName,PSTR("ERR_IO_ERROR"));
			err =  AC_AT24C_IO_ERROR;
		break;
		case ERR_FLUSHED:
			debug_string_2P(NORMAL,funName,PSTR("ERR_FLUSHED"));
			err =  AC_AT24C_FLUSHED;
		break;
		case ERR_TIMEOUT:
			debug_string_2P(NORMAL,funName,PSTR("ERR_TIMEOUT"));
			err =  AC_AT24C_TIMEOUT;
		break;
		case ERR_BAD_DATA:
			debug_string_2P(NORMAL,funName,PSTR("ERR_BAD_DATA"));
			err =  AC_AT24C_BAD_DATA;
		break;
		case ERR_PROTOCOL:
			debug_string_2P(NORMAL,funName,PSTR("ERR_PROTOCOL"));
			err =  AC_AT24C_PROTOCOL;
		break;
		case ERR_UNSUPPORTED_DEV:
			debug_string_2P(NORMAL,funName,PSTR("ERR_UNSUPPORTED_DEV"));
			err =  AC_AT24C_UNSOPPORTED_DEV;
		break;
		case ERR_NO_MEMORY:
			debug_string_2P(NORMAL,funName,PSTR("ERR_NO_MEMORY"));
			err =  AC_AT24C_NO_MEMORY;
		break;
		case ERR_INVALID_ARG:
			debug_string_2P(NORMAL,funName,PSTR("ERR_INVALID_ARG"));
			err =  AC_AT24C_INVALID_ARG;
		break;
		case ERR_BAD_ADDRESS:
			debug_string_2P(NORMAL,funName,PSTR("ERR_BAD_ADDRESS"));
			err =  AC_AT24C_BAD_ADDRESS;
		break;
		case ERR_BUSY:
			debug_string_2P(NORMAL,funName,PSTR("ERR_BUSY"));
			err =  AC_AT24C_BUSY;
		break;
		case ERR_BAD_FORMAT:
			debug_string_2P(NORMAL,funName,PSTR("ERR_BAD_FORMAT"));
			err =  AC_AT24C_BAD_FORMAT;
		break;
		default:
		debug_string_2P(NORMAL,funName,PSTR("UNKONWN ERROR"));

	}
	TWI_report_error(ppak);

	return err;
}

RET_ERROR_CODE AT24CXX_WritePage( const uint8_t addr_page, const uint8_t addr_msb,uint8_t outbuf[])
{
	twi_package_t packet = {
		.addr[0]	  = addr_msb,
		.addr[1]	  = 0,
		.addr_length  = 2,     // TWI slave memory address data size
		.chip         = EEPROM_CHIP_ADDR + addr_page,      // TWI slave bus address
		.buffer       = (void *)outbuf, // transfer data source buffer
		.length       = 256// transfer data size (bytes)
	};

	return AT24CXX_internalWrite(&packet);
	
}


RET_ERROR_CODE AT24CXX_WriteBlockA( const uint8_t addr_page, const uint8_t addr_msb,const uint8_t addr_lsb,uint8_t outbuf[],const uint16_t buflen )
{
	twi_package_t packet = {
		.addr[0]	  = addr_msb,
		.addr[1]	  = addr_lsb,
		.addr_length  = 2,     // TWI slave memory address data size
		.chip         = EEPROM_CHIP_ADDR + addr_page,      // TWI slave bus address
		.buffer       = (void *)outbuf, // transfer data source buffer
		.length       = buflen// transfer data size (bytes)
	};

	return AT24CXX_internalWrite(&packet);
	
}

RET_ERROR_CODE AT24CXX_WriteBlockB( const uint8_t addr_page, const uint8_t addr_msb,const uint8_t addr_lsb,uint8_t outbuf[],const uint16_t buflen )
{
	uint8_t buf[buflen+2];

	for(int i=0;i<buflen;++i) {
		buf[i+2] = outbuf[i];
	}
	
	buf[0] = addr_msb;
	buf[1] = addr_lsb;

	twi_package_t packet = {
		.addr_length  = 0,     // TWI slave memory address data size
		.chip         = EEPROM_CHIP_ADDR + addr_page,      // TWI slave bus address
		.buffer       = (void *)buf, // transfer data source buffer
		.length       = buflen+2  // transfer data size (bytes)
	};

	return AT24CXX_internalWrite(&packet);
	
}

RET_ERROR_CODE AT24CXX_ReadPage( const uint8_t addr_page, const uint8_t addr_msb,uint8_t inbuf[] )
{
	twi_package_t packet = {
		.addr[0]	  = addr_msb,
		.addr[1]	  = 0,
		.addr_length  = 2,     // TWI slave memory address data size
		.chip         = EEPROM_CHIP_ADDR + addr_page,      // TWI slave bus address
		.buffer       = (void *)inbuf, // transfer data source buffer
		.length       = 256// transfer data size (bytes)
	};

	return AT24CXX_internalRead(&packet);
}


RET_ERROR_CODE AT24CXX_ReadBlockA( const uint8_t addr_page, const uint8_t addr_msb,const uint8_t addr_lsb,uint8_t inbuf[],const uint16_t buflen )
{
	twi_package_t packet = {
		.addr[0]	  = addr_msb,
		.addr[1]	  = addr_lsb,
		.addr_length  = 2,     // TWI slave memory address data size
		.chip         = EEPROM_CHIP_ADDR + addr_page,      // TWI slave bus address
		.buffer       = (void *)inbuf, // transfer data source buffer
		.length       = buflen// transfer data size (bytes)
	};

	return AT24CXX_internalRead(&packet);
}

RET_ERROR_CODE AT24CXX_ReadBlockB( const uint8_t addr_page, const uint8_t addr_msb,const uint8_t addr_lsb,uint8_t inbuf[],const uint16_t buflen )
{
	uint8_t add_buff[2] = {addr_msb,addr_lsb};

	twi_package_t pak = {
		.addr_length  = 0,     // TWI slave memory address data size
		.chip         = EEPROM_CHIP_ADDR + addr_page,      // TWI slave bus address
		.buffer       = add_buff,        // transfer data destination buffer
		.length       = 2   // transfer data size (bytes)
	};

	AT24CXX_internalWrite(&pak);

	//debug_string(NORMAL,PSTR("After write\r\n"));
	
	pak.addr_length = 0;
	pak.chip = EEPROM_CHIP_ADDR + addr_page;
	pak.buffer = inbuf;
	pak.length = buflen;

	// Perform a multi-byte read access then check the result.
	return AT24CXX_internalRead(&pak);
	
}

void AT24CXX_iterator_report(const AT24CXX_iterator it) 
{
	char numbuf[16];
	
	debug_string(NORMAL,PSTR("@ "),PGM_STRING);
	itoa(it.byte[PAGE_BYTE],numbuf,10);
	debug_string(NORMAL,numbuf,RAM_STRING);
	debug_string(NORMAL,PSTR(" : "),PGM_STRING);
	itoa(it.byte[MSB_BYTE],numbuf,10);
	debug_string(NORMAL,numbuf,RAM_STRING);
	debug_string(NORMAL,PSTR(" : "),PGM_STRING);
	itoa(it.byte[LSB_BYTE],numbuf,10);
	debug_string(NORMAL,numbuf,RAM_STRING);
	debug_string(NORMAL,PSTR("\r\n"),PGM_STRING);
	
}

