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


#ifndef PM10QBIT_H_
#define PM10QBIT_H_

//#define PM10QBIT_PIN_RX_SIGNAL SP336_USART0_PIN_RX_SIGNAL

typedef struct {
	// remember to initialize attributes
	// PM10QBIT_STATS PM10val = {.mean = 0.0f, .samples = 0, .maxVal = 0, .minVal = 0};
	uint16_t samples;
	uint16_t mean;
	uint16_t meanZeros;
	//uint16_t maxVal;
	//uint16_t minVal;
	uint16_t zeros;
} PM10QBIT_DATA;

#define MODULE_PUBLIC_DATATYPE PM10QBIT_DATA

#define MODULE_INTERFACE_INIT PM10QBIT_init
#define MODULE_INTERFACE_ENABLE PM10QBIT_enable
#define MODULE_INTERFACE_DISABLE PM10QBIT_disable
#define MODULE_INTERFACE_YIELD PM10QBIT_Yield
#define MODULE_INTERFACE_RESET PM10QBIT_reset_data
#define MODULE_INTERFACE_GETDATA PM10QBIT_get_data
#define MODULE_INTERFACE_DATA2STRING PM10QBIT_Data2String

#define MODINST_PARAM_ID MOD_ID_PM10QBIT
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
#endif /* PM10QBIT_H_ */

