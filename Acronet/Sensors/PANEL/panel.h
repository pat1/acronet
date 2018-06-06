
#define MODULE_PUBLIC_DATATYPE PANEL_DATA

#define MODULE_CHANNEL_GLUE_FILE "Acronet/Sensors/PANEL/panel.glue.h"


#define MODULE_INTERFACE_INIT panel_init
#define MODULE_INTERFACE_RESET panel_reset_data
#define MODULE_INTERFACE_GETDATA panel_get_data
#define MODULE_INTERFACE_DATA2STRING panel_Data2String

#define MODULE_CMD_FILTER (panel_cmd,"PANEL_STATUS:")

#ifndef MODULE_PANEL_H_
#define MODULE_PANEL_H_


typedef struct PANEL_DATA
{
	uint8_t			status;
} PANEL_DATA;


RET_ERROR_CODE panel_cmd(const char * const);

#define MODINST_PARAM_ID MOD_ID_PANEL
#include "Acronet/datalogger/modinst/module_interface_declaration.h"
#undef MODINST_PARAM_ID

#endif /* MODULE_PANEL_H_ */

