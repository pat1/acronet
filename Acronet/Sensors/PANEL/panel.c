
#include "Acronet/setup.h"


#include <asf.h>
#include <stdio.h>
#include "Acronet/globals.h"
#include "Acronet/services/config/config.h"
#include "Acronet/HAL/hal_interface.h"


#include "Acronet/Sensors/PANEL/panel.h"

static PANEL_DATA panel_data;


RET_ERROR_CODE panel_init(PANEL_DATA * const pSelf)
{
	return AC_ERROR_OK;
}


RET_ERROR_CODE panel_get_data(const PANEL_DATA * const pSelf,PANEL_DATA * const pDest)
{
//	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"PANEL GETDATA");
	pDest->status = pSelf->status;
}

static void panel_set_data(PANEL_DATA * const ps)
{
		hal_psw_set(ps->status);
		panel_data.status = ps->status;
}

void panel_reset_data(void)
{
	
}

RET_ERROR_CODE panel_Data2String(const PANEL_DATA * const st,char * const sz, size_t * const len_sz)
{
//	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"PANEL DATA2STRING");
	
	if ((*len_sz) < 9)
	{
		return AC_BUFFER_TOO_SMALL;
	}
	
	sz[0] = '&';
	sz[1] = 'S';
	sz[2] = 'T';
	sz[3] = '=';
	sz[4] = (st->status & 8) ? '1' : '0';
	sz[5] = (st->status & 4) ? '1' : '0';
	sz[6] = (st->status & 2) ? '1' : '0';
	sz[7] = (st->status & 1) ? '1' : '0';
	sz[8] = 0;

	*len_sz = 9;

	return AC_ERROR_OK;
}

RET_ERROR_CODE panel_cmd(const  const char * const pPara)
{
	
	PANEL_DATA a = { .status=0 };
	for(uint8_t idx=0;idx<4;++idx)
	{
		const char c = pPara[idx];
		if(c==0) goto invalid_psw;
		if (c!='0')
		{
			a.status |= (((uint8_t)1)<<(3-idx));
		}
	}
		
	char szBuf[32];
	int v = a.status;
	sprintf_P(szBuf,PSTR("\t->\t%d\r\n\r\n"),v);
		
	debug_string_1P(NORMAL,PSTR("\r\n\r\nPANEL_UPDATE: ---> "));
	debug_string(NORMAL,pPara,RAM_STRING);
	debug_string(NORMAL,szBuf,RAM_STRING);
		
	panel_set_data(&a);
	
invalid_psw:
	return AC_ERROR_OK;

}

#define MODULE_PRIVATE_DATA panel_data

#define MODINST_PARAM_ID MOD_ID_PANEL
#include "Acronet/datalogger/modinst/module_interface_definition.h"
#undef MODINST_PARAM_ID
