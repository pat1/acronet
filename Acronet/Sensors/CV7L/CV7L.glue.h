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

#include "Acronet/channels/NMEA/nmea.h"

#define ISTANCE_NUM BOOST_PP_FRAME_ITERATION(2)

#define MODULE_ISTANCE_CHAN BOOST_PP_SEQ_ELEM(ISTANCE_NUM,MODULE_ISTANCES)
#define METHOD_NAME_ISTANCE_TRAIL BOOST_PP_CAT(_ist_,ISTANCE_NUM)

#define MODULE_PRIVATE_DATA  BOOST_PP_CAT(MODULE_INTERFACE_PRIVATE_DATATYPE,BOOST_PP_CAT(_ist_,ISTANCE_NUM))

static MODULE_INTERFACE_PRIVATE_DATATYPE MODULE_PRIVATE_DATA;


#ifdef MODULE_INTERFACE_YIELD
#define MODULE_METHOD_YIELD BOOST_PP_CAT(MODULE_INTERFACE_YIELD,METHOD_NAME_ISTANCE_TRAIL)
bool  MODULE_METHOD_YIELD( void )
{
	static char szBuf[128];
	static uint8_t idx = 0;
	char c;
	while((c = NMEA_LINE_GETCHAR(MODULE_ISTANCE_CHAN)) != 0)
	{
		if (c=='\r')	{
			szBuf[idx]=0;
			if(szBuf[1] != 'P' && 0==NMEA_Line_checksum_check(szBuf,idx)) { //szBuf[1] != 'P' esclude le due stringhe di controllo
				CV7L_process_NMEA_Statement(&MODULE_PRIVATE_DATA,szBuf);
			}
			idx=0;
			} else if (c=='$') {
			szBuf[0]='$';
			szBuf[1]=0;
			idx = 1;
			} else if(idx>0) {
			szBuf[idx++] = c;
		}

		if (idx==(sizeof(szBuf)-1))
		{
			
			debug_string_2P(NORMAL,PSTR("CV7L_Yield"),PSTR("Synchronization lost"));
			szBuf[0]=0;
			idx = 0;
		}
		
		return true;
	}
	return false;
}
#endif

#ifdef MODULE_INTERFACE_INIT
#define MODULE_METHOD_NAME BOOST_PP_CAT(MODULE_INTERFACE_INIT,METHOD_NAME_ISTANCE_TRAIL)
RET_ERROR_CODE MODULE_METHOD_NAME(void)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"CV7L Init");

	NMEA_LINE_RESET(MODULE_ISTANCE_CHAN);
	CV7L_reset_data(&(MODULE_PRIVATE_DATA));
	return AC_ERROR_OK;
//	return MODULE_INTERFACE_INIT(&(MODULE_PRIVATE_DATA));
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
	NMEA_LINE_ENABLE_RX(MODULE_ISTANCE_CHAN);
}

#undef MODULE_METHOD_NAME
#endif

#ifdef MODULE_INTERFACE_DISABLE
#define MODULE_METHOD_NAME BOOST_PP_CAT(MODULE_INTERFACE_DISABLE,METHOD_NAME_ISTANCE_TRAIL)
void MODULE_METHOD_NAME(void)
{
	NMEA_LINE_DISABLE_RX(MODULE_ISTANCE_CHAN);
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
