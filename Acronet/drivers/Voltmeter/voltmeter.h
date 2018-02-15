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


#ifndef VOLTMETER_H_
#define VOLTMETER_H_

#include "Acronet/HAL/hal_interface.h"


static inline void voltmeter_init(void)
{
	hal_voltmeter_init();
}

static inline uint16_t voltmeter_getValue(void)
{
	return hal_voltmeter_getValue();
}

static inline uint16_t thermometer_getValue(void)
{
	return hal_thermometer_getValue();
}


#endif /* VOLTMETER_H_ */