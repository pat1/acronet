/*
 * gpio2log.h
 *
 * Created: 24/05/2017 13:50:49
 *  Author: fabio
 */ 


#ifndef GPIO2LOG_H_
#define GPIO2LOG_H_


#define MODULE_INTERFACE_INIT gpio2log_init
#define MODULE_INTERFACE_YIELD gpio2log_Yield

void gpio2log_periodic(void);

#define MODINST_PARAM_ID MOD_ID_GPIO2LOG
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
#endif /* GPIO2LOG_H_ */

