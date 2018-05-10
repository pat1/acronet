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

#define MODULE_PUBLIC_DATATYPE L8095N_DATA
#define MODULE_CHANNEL_GLUE_FILE "Acronet/Sensors/L8095-N/L8095-N.glue.h"
#define MODULE_INTERFACE_INIT l8095n_init
#define MODULE_INTERFACE_ENABLE l8095n_enable
#define MODULE_INTERFACE_DISABLE l8095n_disable
#define MODULE_INTERFACE_YIELD l8095n_Yield
#define MODULE_INTERFACE_RESET l8095n_reset_data
#define MODULE_INTERFACE_GETDATA l8095n_get_data
#define MODULE_INTERFACE_DATA2STRING l8095n_Data2String

#if defined(RMAP_SERVICES)
#define MODULE_INTERFACE_DATA2STRING_RMAP l8095n_Data2String_RMAP
#endif //RMAP_SERVICES


#ifndef L8095N_H_
#define L8095N_H_

enum {		L8095N_STAT_BEG=0,
			L8095N_STAT_PRESSURE=L8095N_STAT_BEG,
			L8095N_STAT_TEMPERATURE,
			L8095N_STAT_TEMPERATURE_MAX,
			L8095N_STAT_TEMPERATURE_MIN,
			L8095N_STAT_RH,
			L8095N_STAT_DEWPOINT,
			L8095N_STAT_END};


typedef struct
{
	float		m_stat[L8095N_STAT_END];
	uint16_t	m_samples;
} L8095N_DATA;


#define MODINST_PARAM_ID MOD_ID_L8095N
#include "Acronet/datalogger/modinst/module_interface_declaration.h"
#undef MODINST_PARAM_ID


#endif /* L8095N_H_ */
