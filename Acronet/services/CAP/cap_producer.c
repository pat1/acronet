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
#include <conf_usart_serial.h>


#include "Acronet/globals.h"

#include "Acronet/services/config/config.h"

#include "config/conf_usart_serial.h"

#include "Acronet/drivers/SIM/sim900.h"
#include "Acronet/drivers/UART_INT/cbuffer_usart.h"
#include "Acronet/drivers/StatusLED/status_led.h"
#include "Acronet/drivers/AT24CXX/AT24CXX.h"
#include "Acronet/drivers/Voltmeter/voltmeter.h"
#include "Acronet/drivers/PowerSwitch/powerswitch.h"



#include "Acronet/services/CAP/cap_producer.h"
#include "Acronet/services/CAP/cap_common.h"

//typedef enum {IDLE,CHECK} CAP_PRODUCER_STATUS;

//static CAP_PRODUCER_STATUS g_cap_producer_status = IDLE;

RET_ERROR_CODE cap_schedule_check( void )
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"CAP_SCHEDULED_CHECK");
	for(CAP_TYPE it=CAP_TYPE_BEG;it<CAP_TYPE_END;++it) {
		CAP_ALERT_ACTION action;
		cap_check(it,&action);
		if (action.cap_status == CAP_ALERT_STATUS_ISSUE)
		{
			cap_issue(&action);
		} else if (action.cap_status == CAP_ALERT_STATUS_CLEAR)
		{
			cap_clear(&action);
		}
	}
	return AC_ERROR_OK;
}


RET_ERROR_CODE CAP_init(void)
{
	return AC_ERROR_OK;	
}

void CAP_enable(void)
{
	
}

void CAP_disable(void)
{
	
}

bool CAP_Yield(void)
{
//	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"CAP_Yield");
	
	//if (g_cap_producer_status == IDLE)
	//{
		//return false;
	//}
	//
	//if (g_cap_producer_status == CHECK)
	//{
		//g_cap_producer_status = IDLE;
		//
		//for(CAP_TYPE it=CAP_TYPE_BEG;it<CAP_TYPE_END;++it) {
			//CAP_ALERT_ACTION action;
			//cap_check(it,&action);
			//if (action.cap_status == CAP_ALERT_STATUS_ISSUE)
			//{
				//cap_issue(&action);
			//} else if (action.cap_status == CAP_ALERT_STATUS_CLEAR)
			//{
				//cap_clear(&action);
			//}
		//}
	//}
	return false;
}


