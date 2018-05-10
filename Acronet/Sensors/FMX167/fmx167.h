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


#define MODULE_PUBLIC_DATATYPE FMX167_DATA

#define MODULE_INTERFACE_INIT fmx167_init
#define MODULE_INTERFACE_ENABLE fmx167_enable
#define MODULE_INTERFACE_DISABLE fmx167_disable
#define MODULE_INTERFACE_YIELD fmx167_Yield
#define MODULE_INTERFACE_RESET fmx167_reset_data
#define MODULE_INTERFACE_GETDATA fmx167_get_data
#define MODULE_INTERFACE_DATA2STRING fmx167_Data2String


#ifndef FMX167_H_
#define FMX167_H_

#define FMX167_VOLTMETER			ADCB
#define FMX167_VOLTMETER_CH			ADC_CH0
#define FMX167_VOLTMETER_PIN_POS	ADCCH_POS_PIN5
#define FMX167_VOLTMETER_PIN_NEG	ADCCH_POS_PIN2

typedef struct
{
	int16_t			v;
	int16_t			v_max;
	int16_t			v_min;
	uint16_t		level;
} FMX167_DATA;


#define MODINST_PARAM_ID MOD_ID_FMX167
#include "Acronet/datalogger/modinst/module_interface_declaration.h"

#undef MODINST_PARAM_ID


#endif /* FMX167_H_ */

