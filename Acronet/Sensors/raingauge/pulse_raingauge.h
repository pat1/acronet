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

#define MODULE_PUBLIC_DATATYPE RAINGAUGE_DATA
#define MODULE_CHANNEL_GLUE_FILE "Acronet/Sensors/raingauge/pulse_raingauge.glue.h"
#define MODULE_INTERFACE_INIT raingauge_init
#define MODULE_INTERFACE_RESET raingauge_reset_data
#define MODULE_INTERFACE_GETDATA raingauge_get_data
//#define MODULE_INTERFACE_YIELD raingauge_yield
#define MODULE_INTERFACE_DATA2STRING raingauge_Data2String
#if defined(RMAP_SERVICES)
#define MODULE_INTERFACE_DATA2STRING_RMAP raingauge_Data2String_RMAP
#endif //RMAP_SERVICES


#ifndef PULSE_RAINGAUGE_H_
#define PULSE_RAINGAUGE_H_


typedef struct
{
	uint16_t			maxSlope;
	uint16_t			tips;
} RAINGAUGE_DATA;



#define MODINST_PARAM_ID MOD_ID_RAINGAUGE
#include "Acronet/datalogger/modinst/module_interface_declaration.h"
#undef MODINST_PARAM_ID

#endif //PULSE_RAINGAUGE_H_
