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

#include "Acronet/setup.h"
#include "Acronet/HAL/hal_interface.h"

#include <asf.h>
#include <stdio.h>
#include "progmem.h"
#include <conf_board.h>
#include <conf_usart_serial.h>


#include "Acronet/globals.h"

#include "../config/config.h"

#include "config/conf_usart_serial.h"

#include "Acronet/drivers/SIM/sim900.h"
#include "Acronet/drivers/UART_INT/cbuffer_usart.h"

#include "Acronet/Sensors/raingauge/pulse_raingauge.h"
#include "Acronet/drivers/StatusLED/status_led.h"
#include "Acronet/drivers/AT24CXX/AT24CXX.h"
#include "Acronet/drivers/Voltmeter/voltmeter.h"


//#include "services/datalogger/datalogger.h"
#include "Acronet/services/fw_update/fw_update.h"
#include "Acronet/services/fw_update/hex_processor.h"
#include "Acronet/services/fw_update/eeprom_manager.h"


static const __flash AT24CXX_iterator ITER_FW_UPDA_BEG = {	  .byte[FLAG_BYTE] = 0 
															, .byte[PAGE_BYTE] = 0 
															, .byte[ MSB_BYTE] = 1 
															, .byte[ LSB_BYTE] = 0   };
														
static const __flash uint8_t STR_FWDW[4] = {'F','W','D','W'};
static const __flash uint8_t STR_UPDA[4] = {'U','P','D','A'};
static const __flash uint8_t STR_FAIL[4] = {'F','A','I','L'};

RET_ERROR_CODE em_init(EM_BUFFER * const g_emr, const uint8_t mode) 
{
	if (mode==MODE_FIRMWARE_UPDATE)
	{
		g_emr->it = ITER_FW_UPDA_BEG;
	} else 	{
		g_emr->it.plain = 0;
		return AC_ERROR_OK;
	}
	char str[4] = { [0] = STR_FWDW[0] , [1] = STR_FWDW[1] , [2] = STR_FWDW[2] , [3] = STR_FWDW[3] };

	AT24CXX_WriteBlock(0,0,0,str,4);
	delay_ms(5);
	
	return AC_ERROR_OK;
}

RET_ERROR_CODE em_flush( EM_BUFFER * const g_emr, const uint8_t mode )
{
	AT24CXX_iterator i = g_emr->it;

	if (mode==MODE_CONFIG_UPDATE)
	{
		i.byte[LSB_BYTE]=0;
		AT24CXX_iterator_report(i);
		nvm_eeprom_erase_and_write_buffer(i.plain,g_emr->buf,256);
		return AC_ERROR_OK;
	} 
	
	//AT24CXX_iterator * const it = &g_emr->it;
	uint8_t * const pb = g_emr->buf;
	
	
	const RET_ERROR_CODE r = AT24CXX_WritePage(i.byte[PAGE_BYTE],i.byte[MSB_BYTE],pb);
	delay_ms(5);
	return r;
}


RET_ERROR_CODE em_close(const bool success, const uint8_t mode)
{
	if (mode==MODE_CONFIG_UPDATE)
	{
		return AC_ERROR_OK;
	}

	uint8_t str[4] = { [0] = STR_UPDA[0],[1] = STR_UPDA[1],[2] = STR_UPDA[2],[3] = STR_UPDA[3] } ;
										   
	if (success!=true)
	{
		str[0] = STR_FAIL[0];
		str[1] = STR_FAIL[1];
		str[2] = STR_FAIL[2];
		str[3] = STR_FAIL[3];
	}


	AT24CXX_WriteBlock(0,0,0,str,4);

	delay_ms(5);

	return AC_ERROR_OK;

}

RET_ERROR_CODE em_insert(EM_BUFFER * const g_emr, const HEX_READER_RECORD* const pRec,const uint8_t mode)
{
	const size_t l = (mode==MODE_FIRMWARE_UPDATE) ? sizeof(HEX_READER_RECORD) - sizeof(pRec->data) + pRec->len : pRec->len;

	if(l>224) { //
		return AC_UNSUPPORTED;
	}
	
	AT24CXX_iterator it = g_emr->it;
	uint8_t msb,lsb,page;

	AT24CXX_iterator_to_address(it,&page,&msb,&lsb);
	AT24CXX_iterator_add(&it,l);

	const uint8_t * p = (mode==MODE_FIRMWARE_UPDATE) ? ((uint8_t *) pRec ) : ((uint8_t *) pRec->data);
	const uint8_t v = it.byte[LSB_BYTE];
	
	if(v>=l)  { //inside the same chip page
		memcpy_ram2ram(g_emr->buf+lsb,(void *)p,l);
	} else { //overlaps on two different chip pages
		const uint8_t x = (l - v);
		memcpy_ram2ram(g_emr->buf+lsb,(void *)p,x);
		const RET_ERROR_CODE r = em_flush(g_emr,mode);
		if (AC_ERROR_OK!=r)
		{
			return r;
		}
		memcpy_ram2ram(g_emr->buf,(void *)(p+x),v);
	}
	
	g_emr->it = it;
	return AC_ERROR_OK;

}

//RET_ERROR_CODE em_compute_crc(EM_BUFFER * const g_emr, uint32_t * const crc)
//{
	//DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"EM_COMPUTE_CRC");
	//
	//uint8_t mp[256];
//
	//AT24CXX_iterator it = ITER_FW_UPDA_BEG;
//
	//crc_io_checksum_byte_start(CRC_32BIT);
	//
	//const AT24CXX_iterator itEND = g_emr->it; 
	//while(it.plain < itEND.plain) {
//
		//AT24CXX_ReadPage(it.byte[PAGE_BYTE],it.byte[MSB_BYTE],mp);
		//
		//const uint16_t segLen = ((itEND.byte[PAGE_BYTE]==it.byte[PAGE_BYTE]) && (itEND.byte[MSB_BYTE]==it.byte[MSB_BYTE])) ? itEND.byte[LSB_BYTE] : 256;
//
		//for(uint16_t i = 0;i<segLen;i++) {
			//crc_io_checksum_byte_add(mp[i]);
		//}
		//it.plain += 256;
	//}
	//
	//*crc = crc_io_checksum_byte_stop();
	//return AC_ERROR_OK;
//}
