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

#include "Acronet/HAl/hal_interface.h"


#include <asf.h>
#include <stdio.h>
#include "conf_board.h"
#include "Acronet/globals.h"
#include "powerswitch.h"

void powerSwitch_init(void)
{
	hal_powerSwitch_init();
}

void powerSwitch_toggle(void)
{
	hal_powerSwitch_toggle();
}
