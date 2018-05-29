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

////////////////////////////////////////////////////////////////////////////////////
//
// HD3910 module
// - modbus connected device
// each instance of this module requires its own command to be spawned through
// the periodic function; this command is defined in the HD3910_PER_ISTANCE_CMD
// that is a BOOST::preprocessor sequence of tuples
// each tuple is the command, the sequence must contain as many tuples as many
// instances of the module
//

#ifndef HD3910_PER_ISTANCE_CMD
#error "HD3910 module requires the definition of the HD3910_PER_ISTANCE_CMD variable"
#endif


#include "Acronet/channels/MODBUS_RTU/mb_crc.h"
#include "Acronet/channels/MODBUS_RTU/master_rtu.h"


#define ISTANCE_NUM BOOST_PP_FRAME_ITERATION(2)

#define MODULE_ISTANCE_CHAN BOOST_PP_SEQ_ELEM(ISTANCE_NUM,MODULE_ISTANCES)
#define METHOD_NAME_ISTANCE_TRAIL BOOST_PP_CAT(_ist_,ISTANCE_NUM)

#define MODULE_PRIVATE_DATA  BOOST_PP_CAT(MODULE_INTERFACE_PRIVATE_DATATYPE,BOOST_PP_CAT(_ist_,ISTANCE_NUM))

static MODULE_INTERFACE_PRIVATE_DATATYPE MODULE_PRIVATE_DATA;

#define MODULE_FLASH_DATA_TUPLE BOOST_PP_SEQ_ELEM(ISTANCE_NUM,HD3910_PER_ISTANCE_CMD)
#define MODULE_MODBUS_ADDRESS BOOST_PP_TUPLE_ELEM(0,MODULE_FLASH_DATA_TUPLE)


#ifdef MODULE_INTERFACE_YIELD
#define MODULE_METHOD_YIELD BOOST_PP_CAT(MODULE_INTERFACE_YIELD,METHOD_NAME_ISTANCE_TRAIL)
bool  MODULE_METHOD_YIELD( void )
{
	while( MBUS_IS_RIPE(MODULE_ISTANCE_CHAN,MODULE_MODBUS_ADDRESS) )
	{
		const uint8_t b = MBUS_GET_BYTE(MODULE_ISTANCE_CHAN);
		if ( MBUS_STATUS_END == MBUS_BUILD_DGRAM(MODULE_ISTANCE_CHAN, &(MODULE_PRIVATE_DATA.pdu), b) )
		{
			//usart_putchar(USART_DEBUG,'y');

			const uint16_t crcc = MBUS_GET_CRC(MODULE_ISTANCE_CHAN);
			const uint16_t crcp =( (((uint16_t) MODULE_PRIVATE_DATA.pdu.crc_hi) << 8) | MODULE_PRIVATE_DATA.pdu.crc_lo );
			if (crcc == crcp)
			{
				//usart_putchar(USART_DEBUG,'u');
				interpret_pdu(&(MODULE_PRIVATE_DATA));
				} else {
				char szBUF[64];
				sprintf_P(szBUF,PSTR("%04X != %04X\r\n"),crcc,crcp);
				debug_string(NORMAL,szBUF,RAM_STRING);
			}
			MBUS_RELEASE(MODULE_ISTANCE_CHAN);
		}
		//		usart_putchar(USART_DEBUG,'Y');
		
		return true;
	}
	return false;
}
#undef MODULE_METHOD_YIELD
#endif

#ifdef MODULE_INTERFACE_PERIODIC
#define MODULE_METHOD_PERIODIC BOOST_PP_CAT(MODULE_INTERFACE_PERIODIC,METHOD_NAME_ISTANCE_TRAIL)

void MODULE_METHOD_PERIODIC(void)
{

	if ( AC_ERROR_OK != MBUS_LOCK(MODULE_ISTANCE_CHAN) )
	{
		return;
	}

	static const __flash uint8_t cmd[] = { BOOST_PP_TUPLE_ENUM(MODULE_FLASH_DATA_TUPLE) };

	uint8_t buf[32];
	
	//{
		//uint8_t i = 0;
		//for(;i<8;i++) {
			//uint8_t v = cmd[i];
			//sprintf_P(buf, PSTR("%02X "),v);
			//debug_string(NORMAL,buf,RAM_STRING);
		//}
//
		//debug_string_1P(NORMAL,PSTR("\r\n"));
	//}

	debug_string_1P(NORMAL,PSTR("HD3910 ISTANCE "BOOST_PP_STRINGIZE(ISTANCE_NUM)" LOCKS MBUS CHAN "BOOST_PP_STRINGIZE(MODULE_ISTANCE_CHAN)) );
	memcpy_P(buf,cmd,8);
	
	MBUS_ISSUE_CMD(MODULE_ISTANCE_CHAN,buf,8);
}

#undef MODULE_METHOD_PERIODIC

#endif


#ifdef MODULE_INTERFACE_INIT 
#define MODULE_METHOD_NAME BOOST_PP_CAT(MODULE_INTERFACE_INIT,METHOD_NAME_ISTANCE_TRAIL)
RET_ERROR_CODE MODULE_METHOD_NAME(void)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"HD3910 INIT ON CHAN "BOOST_PP_STRINGIZE(MODULE_ISTANCE_CHAN) );
	return MODULE_INTERFACE_INIT(&(MODULE_PRIVATE_DATA));
}

#undef MODULE_METHOD_NAME
#endif

#ifdef MODULE_INTERFACE_RESET
#define MODULE_METHOD_NAME BOOST_PP_CAT(MODULE_INTERFACE_RESET,METHOD_NAME_ISTANCE_TRAIL)
RET_ERROR_CODE MODULE_METHOD_NAME(void)
{
	return MODULE_INTERFACE_RESET(&(MODULE_PRIVATE_DATA));
}

#undef MODULE_METHOD_NAME
#endif

#ifdef MODULE_INTERFACE_ENABLE
#define MODULE_METHOD_NAME BOOST_PP_CAT(MODULE_INTERFACE_ENABLE,METHOD_NAME_ISTANCE_TRAIL)
void MODULE_METHOD_NAME(void)
{
	
}

#undef MODULE_METHOD_NAME
#endif

#ifdef MODULE_INTERFACE_DISABLE
#define MODULE_METHOD_NAME BOOST_PP_CAT(MODULE_INTERFACE_DISABLE,METHOD_NAME_ISTANCE_TRAIL)
void MODULE_METHOD_NAME(void)
{
	
}

#undef MODULE_METHOD_NAME
#endif

#ifdef MODULE_INTERFACE_GETDATA
#define MODULE_METHOD_NAME BOOST_PP_CAT(MODULE_INTERFACE_GETDATA,METHOD_NAME_ISTANCE_TRAIL)
RET_ERROR_CODE  MODULE_METHOD_NAME(MODULE_PUBLIC_DATATYPE * const pData)
{
	return MODULE_INTERFACE_GETDATA(&(MODULE_PRIVATE_DATA),pData);
}
#undef MODULE_METHOD_NAME
#endif
