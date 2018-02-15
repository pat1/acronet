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


#include <asf.h>
#include <stdio.h>
#include "progmem.h"
#include <conf_board.h>
#include "Acronet/globals.h"
#include "Acronet/drivers/ADC/ADC_Manager.h"





RET_ERROR_CODE ADC_MAN_Init(const ADC_MAN_SETUP * const pTable,uint8_t tbSize)
{


	for(uint8_t i=0;i<tbSize;++i)
	{
		debug_string_1P(NORMAL,pTable[i].szBrief);
	}

	return AC_ERROR_OK;	
}


RET_ERROR_CODE ADC_MAN_Enable(void)
{
	return AC_ERROR_OK;
	
}


RET_ERROR_CODE ADC_MAN_Disable(void)
{
	
	return AC_ERROR_OK;
}

