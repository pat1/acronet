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


#ifndef STATUS_LED_H_
#define STATUS_LED_H_

#include "Acronet/HAL/hal_interface.h"

static inline void statusled_blink(uint8_t t)
{
	hal_status_led_blink(t);
}

static inline void status_led_toggle(void)
{
	hal_status_led_toggle();
}



#endif /* STATUS_LED_H_ */