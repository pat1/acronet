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
#include "Acronet/HAl/hal_interface.h"


#include <asf.h>
#include <stdio.h>
//#include "Acronet/globals.h"
#include "board.h"
#include "sysclk.h"
#include "twi_master.h"
//#include "led.h"

#include "conf_board.h"
#include "Acronet/drivers/PCAL9535A/PCAL9535A.h"

//static RET_ERROR_CODE internal_Init( void );
static RET_ERROR_CODE PCAL9535A_internalWrite(twi_package_t * ppak);
static RET_ERROR_CODE PCAL9535A_internalRead(twi_package_t * ppak);


//RET_ERROR_CODE PCAL9535A_Init( void )
//{
	//DEBUG_PRINT_FUNCTION_NAME(NORMAL,"PCAL9535A_INIT");
			//
	//twi_master_options_t opt = {
		//.speed = PCAL9535A_TWI_SPEED,
		//.chip  = TWI_MASTER_ADDRESS,
	//};
			//
//
	//const int e = twi_master_setup(PCAL9535A_TWI_PORT, &opt);
//
	//if (STATUS_OK==e)
	//{
		//return internal_Init();
	//}
//
	//return AC_UNSUPPORTED;
//
//}


static RET_ERROR_CODE PCAL9535A_internalWrite(twi_package_t * ppak)
{
	static const char __flash funName[] = "PCAL9535A_internalWrite";
	
	//debug_string("Before write\r\n");
	const status_code_t r = twi_master_write(PCAL9535A_TWI_PORT, ppak);

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

static RET_ERROR_CODE PCAL9535A_internalRead(twi_package_t * ppak)
{
	const char * const funName = PSTR("PCAL9535A_internalRead");

	const status_code_t r = twi_master_read(PCAL9535A_TWI_PORT, ppak);
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


RET_ERROR_CODE PCAL9535A_Write( const uint8_t cmd, const uint8_t data  )
{
	twi_package_t packet = {
		.addr[0]	  = cmd,
		.addr[1]	  = data,
		.addr_length  = 2,     // TWI slave memory address data size
		.chip         = PCAL9535A_CHIP_ADDR ,      // TWI slave bus address
		.buffer       = NULL, // transfer data source buffer
		.length       = 0// transfer data size (bytes)
	};

	return PCAL9535A_internalWrite(&packet);

	return AC_ERROR_OK;
}

RET_ERROR_CODE PCAL9535A_Read( const uint8_t cmd, uint8_t * const data  )
{
	twi_package_t packet = {
		.addr[0]	  = cmd,
		.addr_length  = 1,     // TWI slave memory address data size
		.chip         = PCAL9535A_CHIP_ADDR ,      // TWI slave bus address
		.buffer       = data, // transfer data source buffer
		.length       = 1// transfer data size (bytes)
	};

	return PCAL9535A_internalRead(&packet);

	return AC_ERROR_OK;
}

//static RET_ERROR_CODE internal_Init( void )
//{
//
	////	const char * const funName = PSTR("PCAL9535");
//
	///////////////////////////////////////////////////////////////
	///////////
	///////////  PORT0 INPUT/OUTPUT PIN CONFIG
	///////////  IF A	BIT IN THESE REGISTERS IS SET TO 1
	///////////  THE CORRESPONDING PORT PIN IS ENABLED AS A
	///////////  HIGH-IMPEDANCE INPUT.
	///////////  IF A BIT IN THESE REGISTERS IS CLEARED TO 0,
	///////////  THE CORRESPONDING PORT PIN IS ENABLED AS AN OUTPUT
	///////////
	///////////  On board V03 PORT0 PINS are configured as follow
	///////////  P0_0 : INPUT  -> NVALID1 (BACKUP POWER INPUT VOLTAGE VALIDITY)
	///////////  P0_1 : INPUT  -> NVALID2 (PRIMARY POWER INPUT VOLTAGE VALIDITY)
	///////////  P0_2 : INPUT  -> NVALID3 (WALL POWER INPUT VOLTAGE VALIDITY)
	///////////  P0_3 : OUTPUT -> I2C_SIGNAL
	///////////  P0_4 : OUTPUT -> SP336_M1
	///////////  P0_5 : OUTPUT -> SP336_M2
	///////////  P0_6 : OUTPUT -> SP336_M3
	///////////  P0_7 : OUTPUT -> SP336_SLEW_RATE_LIMIT
	///////////
	///////////
	//RET_ERROR_CODE r = PCAL9535A_Write(0x6,0b01101111); //0b01101111
	//delay_ms(5);
	//if(AC_ERROR_OK != r )
	//{
		//return r;
	//}
//
	///////////////////////////////////////////////////////////////
	///////////
	/////////// PORT0 PULLUP/PULLDOWN ENABLE PIN CONFIG
	///////////
	/////////// SETTING THE BIT TO LOGIC 1 ENABLES THE SELECTION OF PULL-UP/PULL-DOWN RESISTORS.
	/////////// SETTING THE BIT TO LOGIC 0 DISCONNECTS THE PULL-UP/PULL-DOWN RESISTORS FROM THE I/O PINS.
	/////////// THE RESISTORS WILL BE DISCONNECTED WHEN THE OUTPUTS ARE CONFIGURED AS OPEN-DRAIN OUTPUTS
	/////////// USE THE PULL-UP/PULL-DOWN REGISTERS TO SELECT EITHER A PULL-UP OR PULL-DOWN
	/////////// RESISTOR
	///////////
	//r = PCAL9535A_Write(0x46,0b00000111);
	//delay_ms(5);
	//if(AC_ERROR_OK != r )
	//{
		//return r;
	//}
//
//
	///////////////////////////////////////////////////////////////
	///////////
	/////////// PORT0 CHOOSE PULLUP OR PULLDOWN  PIN CONFIG
	///////////
	///////////  P0_0 : INPUT WITH PULL-UP    -> 1
	///////////  P0_1 : INPUT WITH PULL-UP    -> 1
	///////////  P0_2 : INPUT WITH PULL-UP    -> 1
	///////////  P0_3 : OUTPUT WITH PULL-DOWN -> 0
	///////////  P0_4 to P0_6 : SP336 MODE SELECTION
	///////////       000 -> LOOPBACK
	///////////       001 -> 2 RS485 HALFDUPLEX
	///////////       010 -> 2 RS232
	///////////       011 -> 2 RS485 FULLDUPLEX
	///////////       100 -> MIXED HALFDUPLEX - RS232 ON PORTA - RS485 ON PORTB
	///////////       101 -> LOWPOWER RS232 ONLY RX
	///////////       110 -> MIXED FULLDUPLEX - RS232 ON PORTA - RS485 ON PORTB
	///////////       111 -> SHUTDOWN
	///////////  P0_7 : SP336 SLEWRATE LIMIT SELECTION
	//r = PCAL9535A_Write(0x48,0b00000111);
	//delay_ms(5);
	//if(AC_ERROR_OK != r )
	//{
		//return r;
	//}
//
	///////////////////////////////////////////////////////////////
	/////////// PORT0 OUTPUT WRITE
	//r = PCAL9535A_Write(0x2,0b00000000);
	//delay_ms(5);
	//if(AC_ERROR_OK != r )
	//{
		//return r;
	//}
//
	///////////////////////////////////////////////////////////////
	///////////
	///////////  PORT1 INPUT/OUTPUT PIN CONFIG
	///////////  IF A	BIT IN THESE REGISTERS IS SET TO 1
	///////////  THE CORRESPONDING PORT PIN IS ENABLED AS A
	///////////  HIGH-IMPEDANCE INPUT.
	///////////  IF A BIT IN THESE REGISTERS IS CLEARED TO 0,
	///////////  THE CORRESPONDING PORT PIN IS ENABLED AS AN OUTPUT
	///////////
	///////////  On board V03 PORT0 PINS are configured as follow
	///////////  P1_7 : INPUT  -> PSW STATUS
	///////////  P1_6 : INPUT  -> PSW STATUS
	///////////  P1_5 : INPUT  -> PSW STATUS
	///////////  P1_4 : INPUT  -> PSW STATUS
	///////////  P1_3 : OUTPUT -> PSW COMMAND
	///////////  P1_2 : OUTPUT -> PSW COMMAND
	///////////  P1_1 : OUTPUT -> PSW COMMAND
	///////////  P1_0 : OUTPUT -> PSW COMMAND
	///////////
	///////////
	//r = PCAL9535A_Write(0x7,0b11110000);
	//delay_ms(5);
	//if(AC_ERROR_OK != r )
	//{
		//return r;
	//}
	//
	///////////////////////////////////////////////////////////////
	///////////
	/////////// PORT1 PULLUP/PULLDOWN ENABLE PIN CONFIG
	///////////
	/////////// SETTING THE BIT TO LOGIC 1 ENABLES THE SELECTION OF PULL-UP/PULL-DOWN RESISTORS.
	/////////// SETTING THE BIT TO LOGIC 0 DISCONNECTS THE PULL-UP/PULL-DOWN RESISTORS FROM THE I/O PINS.
	/////////// THE RESISTORS WILL BE DISCONNECTED WHEN THE OUTPUTS ARE CONFIGURED AS OPEN-DRAIN OUTPUTS
	/////////// USE THE PULL-UP/PULL-DOWN REGISTERS TO SELECT EITHER A PULL-UP OR PULL-DOWN
	/////////// RESISTOR
	///////////
	//r = PCAL9535A_Write(0x47,0b11110000);
	//delay_ms(5);
	//if(AC_ERROR_OK != r )
	//{
		//return r;
	//}
//
	///////////////////////////////////////////////////////////////
	///////////
	/////////// PORT1 CHOOSE PULLUP OR PULLDOWN  PIN CONFIG
	///////////
	///////////  P0_7 : INPUT WITH PULL-UP    -> 1
	///////////  P0_6 : INPUT WITH PULL-UP    -> 1
	///////////  P0_5 : INPUT WITH PULL-UP    -> 1
	///////////  P0_4 : INPUT WITH PULL-UP    -> 1
	///////////  
	//
	//r = PCAL9535A_Write(0x49,0b11110000);
	//delay_ms(5);
	//if(AC_ERROR_OK != r )
	//{
		//return r;
	//}
//
//
	/////////////////////////////////////////////////////////////
	/////////// PORT1 WRITE
	//r = PCAL9535A_Write(0x3,0b00001111);
	//delay_ms(5);
	//if(AC_ERROR_OK != r )
	//{
		//return r;
	//}
//
//
	//return AC_ERROR_OK;
//}
