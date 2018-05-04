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


#ifndef CV7L_H_
#define CV7L_H_

/*
#define CV7L_PUSART SP336_USART1
#define CV7L_PIN_TX_ENABLE SP336_USART1_PIN_TX_ENABLE
#define CV7L_PIN_TX_SIGNAL	SP336_USART1_PIN_TX_SIGNAL
#define CV7L_PIN_RX_SIGNAL SP336_USART1_PIN_RX_SIGNAL
*/

//#define CV7L_PUSART SP336_USART1
//#define CV7L_PIN_TX_ENABLE SP336_USART1_PIN_TX_ENABLE
//#define CV7L_PIN_TX_SIGNAL	SP336_USART1_PIN_TX_SIGNAL
//#define CV7L_PIN_RX_SIGNAL SP336_USART1_PIN_RX_SIGNAL

enum {	CV7L_STAT_BEG=0,
		CV7L_STAT_TEMPERATURE = CV7L_STAT_BEG,
		CV7L_STAT_TEMPERATURE_MAX,
		CV7L_STAT_TEMPERATURE_MIN,
		CV7L_STAT_WINDIR,
		CV7L_STAT_WINSPEED,
		CV7L_STAT_WINSPEED_GUST,			
		CV7L_STAT_END};


typedef struct
{
	float		m_stat[CV7L_STAT_END];
	uint16_t	m_samples;
} CV7L_DATA;

#define MODULE_PUBLIC_DATATYPE CV7L_DATA

#define MODULE_CHANNEL_GLUE_FILE "Acronet/Sensors/CV7L/CV7L.glue.h"

#define MODULE_INTERFACE_INIT CV7L_init
#define MODULE_INTERFACE_ENABLE CV7L_enable
#define MODULE_INTERFACE_DISABLE CV7L_disable
#define MODULE_INTERFACE_YIELD CV7L_Yield
#define MODULE_INTERFACE_RESET CV7L_reset_data
#define MODULE_INTERFACE_GETDATA CV7L_get_data
#define MODULE_INTERFACE_DATA2STRING CV7L_Data2String

#define MODINST_PARAM_ID MOD_ID_CV7L
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
#endif /* CV7L_H_ */
