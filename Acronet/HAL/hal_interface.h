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



#ifndef HAL_H_
#define HAL_H_

#include <asf.h>
#include <stdio.h>
#include <stdint.h>
#include "Acronet/globals.h"

//////////////////////////////////////////////////////////////////////////
//
// List of the hal modules interfaces
//
//////////////////////////////////////////////////////////////////////////

#include "Acronet/HAL/modules/interface/hal_board_iface.h"
#include "Acronet/HAL/modules/interface/hal_psw_iface.h"
#include "Acronet/HAL/modules/interface/hal_rtc_iface.h"
#include "Acronet/HAL/modules/interface/hal_status_led_iface.h"
#include "Acronet/HAL/modules/interface/hal_powerswitch_iface.h"
#include "Acronet/HAL/modules/interface/hal_voltmeter_iface.h"
#include "Acronet/HAL/modules/interface/hal_mcu_thermo_iface.h"





//////////////////////////////////////////////////////////////////////////
//
// inline implementations
//
//////////////////////////////////////////////////////////////////////////


#if defined(BOARD_ACROSTATION_V4R01)
#include "Acronet/HAL/board_version/V04R01.h"
#elif defined(BOARD_ACROSTATION_V3R02)
#include "Acronet/HAL/board_version/V03R02.h"
#elif defined(BOARD_ACROSTATION_V2R05)
#include "Acronet/HAL/board_version/V02R05.h"
#else
#error "Missing setup"
#endif


#endif /* HAL_H_ */