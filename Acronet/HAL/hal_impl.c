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

#include "Acronet/setup.h"
#include "Acronet/HAL/hal_interface.h"

#if defined(BOARD_ACROSTATION_V4R02C)
#include "Acronet/HAL/board_version/V04R02C_impl.h"
#elif defined(BOARD_ACROSTATION_V4R01)
#include "Acronet/HAL/board_version/V04R01_impl.h"
#elif defined(BOARD_ACROSTATION_V3R02)
#include "Acronet/HAL/board_version/V03R02_impl.h"
#elif defined(BOARD_ACROSTATION_V2R05)
#include "Acronet/HAL/board_version/V02R05_impl.h"
#else
#error "Missing setup"
#endif //BOARD_VERSION
