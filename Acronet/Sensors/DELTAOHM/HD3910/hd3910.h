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


#define MODULE_PUBLIC_DATATYPE HD3910_DATA
#define MODULE_CHANNEL_GLUE_FILE "Acronet/Sensors/DELTAOHM/HD3910/hd3910.glue.h"
#define MODULE_INTERFACE_INIT hd3910_init
#define MODULE_INTERFACE_ENABLE hd3910_enable
#define MODULE_INTERFACE_DISABLE hd3910_disable
#define MODULE_INTERFACE_YIELD hd3910_Yield
#define MODULE_INTERFACE_RESET hd3910_reset_data
#define MODULE_INTERFACE_GETDATA hd3910_get_data
#define MODULE_INTERFACE_DATA2STRING hd3910_Data2String
#define MODULE_INTERFACE_PERIODIC hd3910_periodic
#if defined(RMAP_SERVICES)
#define MODULE_INTERFACE_DATA2STRING_RMAP hd3910_Data2String_RMAP
#endif //RMAP_SERVICES


#ifndef HD3910_H_
#define HD3910_H_

typedef struct
{
	int16_t			wvc;
	int16_t			temp;
	uint8_t			samples;
} HD3910_DATA;


#define MODINST_PARAM_ID MOD_ID_HD3910_MODBUS
#include "Acronet/datalogger/modinst/module_interface_declaration.h"
#undef MODINST_PARAM_ID

#endif /* HD3910_H_ */


