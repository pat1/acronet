
#ifndef MODULE_PANEL_H_
#define MODULE_PANEL_H_


typedef struct PANEL_DATA
{
	uint8_t			status;
} PANEL_DATA;


#define MODULE_PUBLIC_DATATYPE PANEL_DATA

#define MODULE_INTERFACE_INIT panel_init
#define MODULE_INTERFACE_RESET panel_reset_data
#define MODULE_INTERFACE_GETDATA panel_get_data
#define MODULE_INTERFACE_DATA2STRING panel_Data2String

#define MODINST_PARAM_ID MOD_ID_PANEL
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

#endif /* MODULE_PANEL_H_ */

