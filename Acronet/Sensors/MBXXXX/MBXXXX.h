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

#define MODULE_PUBLIC_DATATYPE MBXXXX_DATA
#define MODULE_INTERFACE_INIT MBXXXX_init
#define MODULE_INTERFACE_ENABLE MBXXXX_enable
#define MODULE_INTERFACE_DISABLE MBXXXX_disable
#define MODULE_INTERFACE_YIELD MBXXXX_Yield
#define MODULE_INTERFACE_RESET MBXXXX_reset_data
#define MODULE_INTERFACE_GETDATA MBXXXX_get_data
#define MODULE_INTERFACE_DATA2STRING MBXXXX_Data2String


#ifndef LEVELGAUGE_H_
#define LEVELGAUGE_H_

#define LG_ADC_PORT		PORTB
#define LG_ADC			ADCB
#define LG_ADC_CH		ADC_CH0
#define LG_ID			0


#ifdef MB_RESOLUTION_MM
#define MBXXXX_NORANGEFOUND		9999 //MB7563
#else
#define MBXXXX_NORANGEFOUND		765	//MB7040
#endif

typedef struct {
	int16_t val;
	int16_t maxVal;
	int16_t minVal;
	uint16_t samples;
	uint16_t noRangeFound;
#ifdef 	MBXXXX_MISS_STATS
	uint16_t misses;
#endif	
} MBXXXX_DATA;


void MBXXXX_triggerReading(void);

#define MODINST_PARAM_ID MOD_ID_MBXXXX
#include "Acronet/datalogger/modinst/module_interface_declaration.h"

#undef MODINST_PARAM_ID


#endif /* LEVELGAUGE_H_ */

