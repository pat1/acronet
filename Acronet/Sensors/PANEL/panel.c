
#include "Acronet/setup.h"


#include <asf.h>
#include <stdio.h>
#include "Acronet/globals.h"
#include "Acronet/services/config/config.h"


#include "Acronet/Sensors/PANEL/panel.h"

static volatile PANEL_DATA panel_data;


RET_ERROR_CODE panel_init(void)
{
	return AC_ERROR_OK;
}


void panel_get_data(PANEL_DATA * const ps)
{
//	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"PANEL GETDATA");
	ps->status = panel_data.status;
}

void panel_set_data(PANEL_DATA * const ps)
{
	panel_data.status = ps->status;
}

void panel_reset_data(void)
{
	
}

RET_ERROR_CODE panel_Data2String(const PANEL_DATA * const st,char * const sz, int16_t * len_sz)
{
//	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"PANEL DATA2STRING");
	
	if ((*len_sz) < 9)
	{
		return AC_BUFFER_TOO_SMALL;
	}
	
	sz[0] = '&';
	sz[1] = 'P';
	sz[2] = 'S';
	sz[3] = 'T';
	sz[4] = '=';
	sz[5] = (st->status & 8) ? '1' : '0';
	sz[6] = (st->status & 4) ? '1' : '0';
	sz[7] = (st->status & 2) ? '1' : '0';
	sz[8] = (st->status & 1) ? '1' : '0';
	sz[9] = 0;

	*len_sz = 9;

	return AC_ERROR_OK;
}
