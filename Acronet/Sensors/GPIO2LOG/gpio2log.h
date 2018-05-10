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

#define MODULE_INTERFACE_INIT gpio2log_init
#define MODULE_INTERFACE_YIELD gpio2log_Yield


#ifndef GPIO2LOG_H_
#define GPIO2LOG_H_


void gpio2log_periodic(void);

#define MODINST_PARAM_ID MOD_ID_GPIO2LOG
#include "Acronet/datalogger/modinst/module_interface_declaration.h"
#undef MODINST_PARAM_ID

#endif /* GPIO2LOG_H_ */

