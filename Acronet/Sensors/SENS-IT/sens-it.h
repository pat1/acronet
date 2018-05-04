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

#ifndef SENS_IT_H_
#define SENS_IT_H_


enum {
	SENSIT_STAT_BEG=0,
	SENSIT_STAT_CO=SENSIT_STAT_BEG,
	SENSIT_STAT_NO2,
	SENSIT_STAT_O3,
	SENSIT_STAT_CH4,
	SENSIT_STAT_NOx,
	SENSIT_STAT_C6H6,
	SENSIT_STAT_END
	};

typedef struct {
		float		m_stat[SENSIT_STAT_END];
		uint16_t	m_samples[SENSIT_STAT_END];
		uint16_t	m_err[SENSIT_STAT_END];
} SENSIT_DATA;

#define MODULE_PUBLIC_DATATYPE SENSIT_DATA

#define MODULE_INTERFACE_INIT sens_it_init
#define MODULE_INTERFACE_ENABLE sens_it_enable
#define MODULE_INTERFACE_DISABLE sens_it_disable
#define MODULE_INTERFACE_YIELD sens_it_Yield
#define MODULE_INTERFACE_RESET sens_it_reset_data
#define MODULE_INTERFACE_GETDATA sens_it_get_data
#define MODULE_INTERFACE_DATA2STRING sens_it_Data2String

#if defined(RMAP_SERVICES)

#else

#endif //RMAP_SERVICES


void sens_it_triggerReading(uint8_t address);

#endif /* SENS_IT_H_ */

