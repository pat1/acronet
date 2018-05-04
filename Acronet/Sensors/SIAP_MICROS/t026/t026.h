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


#ifndef T026_H_
#define T026_H_

typedef struct
{
	int16_t			temp;
	int16_t			rh;
	uint8_t			samples;
} T026_DATA;


#define MODULE_PUBLIC_DATATYPE T026_DATA

#define MODULE_CHANNEL_GLUE_FILE "Acronet/Sensors/SIAP_MICROS/t056/t026.glue.h"


#define MODULE_INTERFACE_INIT t026_init
#define MODULE_INTERFACE_ENABLE t026_enable
#define MODULE_INTERFACE_DISABLE t026_disable
#define MODULE_INTERFACE_YIELD t026_Yield
#define MODULE_INTERFACE_RESET t026_reset_data
#define MODULE_INTERFACE_GETDATA t026_get_data
#define MODULE_INTERFACE_DATA2STRING t026_Data2String

#if defined(RMAP_SERVICES)
#define MODULE_INTERFACE_DATA2STRING_RMAP t026_Data2String_RMAP
#endif //RMAP_SERVICES

#define MODINST_PARAM_ID MOD_ID_T026_MODBUS
#include "Acronet/datalogger/modinst/module_interface_declaration.h"


#undef MODINST_PARAM_ID
/*
#undef MODULE_PUBLIC_DATATYPE

#undef MODULE_INTERFACE_INIT
#undef MODULE_INTERFACE_ENABLE
#undef MODULE_INTERFACE_DISABLE
#undef MODULE_INTERFACE_YIELD
#undef MODULE_INTERFACE_RESET
#undef MODULE_INTERFACE_GETDATA
#undef MODULE_INTERFACE_DATA2STRING
*/
#endif /* T026_H_ */


