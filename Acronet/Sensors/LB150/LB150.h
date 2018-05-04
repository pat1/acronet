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


#ifndef LB150_H_
#define LB150_H_

enum {	LB150_STAT_BEG=0,
		LB150_STAT_PRESSURE=LB150_STAT_BEG,
		LB150_STAT_TEMPERATURE,
		LB150_STAT_TEMPERATURE_MAX,
		LB150_STAT_TEMPERATURE_MIN,
		LB150_STAT_RH,
		LB150_STAT_DEWPOINT,
		LB150_STAT_WINDIR,
		LB150_STAT_WINDIR_GUST,
		LB150_STAT_WINSPEED,
		LB150_STAT_WINSPEED_GUST,
#ifdef LB150_ENABLE_GPS
		LB150_STAT_LATITUDE,
		LB150_STAT_LONGITUDE,
#endif	//	LB150_ENABLE_GPS
		LB150_STAT_END};


typedef struct
{
	float		m_stat[LB150_STAT_END];
	uint16_t	m_samples;
} LB150_DATA;

#define MODULE_PUBLIC_DATATYPE LB150_DATA

#define MODULE_INTERFACE_INIT LB150_init
#define MODULE_INTERFACE_ENABLE LB150_enable
#define MODULE_INTERFACE_DISABLE LB150_disable
#define MODULE_INTERFACE_YIELD LB150_Yield
#define MODULE_INTERFACE_RESET LB150_reset_data
#define MODULE_INTERFACE_GETDATA LB150_get_data
#define MODULE_INTERFACE_DATA2STRING LB150_Data2String

#define MODINST_PARAM_ID MOD_ID_LB150
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
#endif /* LB150_H_ */
