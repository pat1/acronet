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


#ifndef VEGAPULS61_H_
#define VEGAPULS61_H_

typedef struct
{
	int16_t			v;
	int16_t			v_max;
	int16_t			v_min; /* ToDo */
	uint8_t			samples;
} VP61_DATA;

#define MODULE_PUBLIC_DATATYPE VP61_DATA

#define MODULE_INTERFACE_INIT vp61_init
#define MODULE_INTERFACE_ENABLE vp61_enable
#define MODULE_INTERFACE_DISABLE vp61_disable
#define MODULE_INTERFACE_RESET vp61_reset_data
#define MODULE_INTERFACE_GETDATA vp61_get_data
#define MODULE_INTERFACE_DATA2STRING vp61_Data2String

#if defined(RMAP_SERVICES)
#define MODULE_INTERFACE_DATA2STRING_RMAP vp61_Data2String_RMAP
#endif //RMAP_SERVICES

#define MODINST_PARAM_ID MOD_ID_VP61
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
#endif /* VEGAPULS61_H_ */

