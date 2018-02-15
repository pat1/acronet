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
#include "Acronet/HAL/hal_interface.h"
 
 #include <asf.h>
 #include <stdio.h>
 #include "progmem.h"
 #include <conf_board.h>
 #include <conf_usart_serial.h>


 #include "Acronet/globals.h"

 #include "config/conf_usart_serial.h"
 #include "Acronet/drivers/SIM/sim900.h"
 #include "Acronet/drivers/UART_INT/cbuffer_usart.h"
 #include "Acronet/drivers/StatusLED/status_led.h"
 #include "Acronet/drivers/AT24CXX/AT24CXX.h"
 #include "../config/config.h"

 #include "fw_update.h"
 
 static uint32_t g_timeBegin = 0;
 
 static void fw_update_timer(uint32_t timeNow)
 {
	//hal_rtc_set_alarm_relative(5);
	gpio_toggle_pin(STATUS_LED_PIN);

	wdt_reset();
	 
	if(g_timeBegin==0) g_timeBegin = timeNow;
	if( (timeNow-g_timeBegin) > 3600 ) {
		wdt_reset_mcu();
	}
	 
 }
 
 RET_ERROR_CODE fw_update_init( const char * pPara )
{

	//DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"FW_UPDATE_INIT");

	if(0!=sim900_GPRS_check_line())
	{
		uint8_t ii=0;
		while(1) {
			if(++ii==10) {
				wdt_set_timeout_period(WDT_TIMEOUT_PERIOD_8CLK);
				wdt_enable();
				wdt_reset_mcu();
				delay_ms(1000);
			}
			if(AC_ERROR_OK==sim900_init()) {
				break;
			}
		}
	}

	g_timeBegin = 0;
	hal_rtc_set_period_cb(fw_update_timer);
	//hal_rtc_set_alarm_relative(5);
	wdt_set_timeout_period(WDT_TIMEOUT_PERIOD_2KCLK);
	wdt_enable();

	return 0;
 }

#include "hex_processor.h"
#include "eeprom_manager.h"

static RET_ERROR_CODE fw_update_process_hex_rec(EM_BUFFER * const pemb, const HEX_READER_RECORD* const pRec ,const uint8_t mode )
{
	return	em_insert(pemb,pRec,mode);
}


static RET_ERROR_CODE fw_update_process_hex_chunk( EM_BUFFER * const pemb, const char * const pBuf,uint16_t size, uint16_t * const numLines, const uint8_t mode)
{
	if( !(size>0) ) return AC_ERROR_GENERIC;
	
	RET_ERROR_CODE err = hex_processor_init(pBuf,size);
	
	HEX_READER_RECORD aRec;
	char status = (char) 0;
	uint16_t nl = 0;

	while(err==AC_ERROR_OK) {
		err = hex_processor_get_rec(&aRec,&status);
		
		if ((status==(char)EOF) && (err==AC_NOTHING_TO_DO))
		{
			debug_string_1P(NORMAL,PSTR("GOT EOF while requesting a new record"));
			break;
		}

		if (err==AC_NOTHING_TO_DO)
		{
			debug_string_1P(NORMAL,PSTR("GOT a line without a record, skip"));
			break;
		}


		if ((status==(char)EOF))
		{
			debug_string_1P(NORMAL,PSTR("[ERROR] The record wasn't completely in the buffer"));
			return AC_ERROR_GENERIC;
		}

		
		if(AC_ERROR_OK!=hex_processor_verify_rec(&aRec)) {
			debug_string_1P(NORMAL,PSTR("[ERROR] hex processor checksum mismatch"));
			return AC_ERROR_GENERIC;
		}
		
		if (AC_ERROR_OK != fw_update_process_hex_rec(pemb,&aRec,mode))
		{
			debug_string_1P(NORMAL,PSTR("[ERROR] hex processor unable to process record"));
			return AC_ERROR_GENERIC;
		}
		
		nl++;
	}
	
	*numLines = nl;
	return AC_ERROR_OK;
}

#include "eeprom_manager.h"

//void datalogger_dump_all();

 
 void fw_update_run( const char * pPara,const uint8_t mode )
{
	 
	//DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"FW_UPDATE_RUN");

	char szGetQuery[256];
	static const int SIZE_GET_QUERY = sizeof(szGetQuery);

	char szGetAnswer[4096];
	static const int SIZE_GET_ANSWER = sizeof(szGetAnswer);

	CFG_ITEM_ADDRESS f = 0;

	if( AC_ERROR_OK != cfg_find_item(CFG_TAG_DATALOGGER_SEND_URL,&f))
	{
		debug_string_1P(NORMAL,PSTR("[ERROR] Missing configuration file"));
		wdt_reset_mcu();
	}
	
	cfg_get_item_file(f,szGetQuery,64);

//	cfg_get_service_url_send(szGetQuery,64);
	int l1 = strnlen(szGetQuery,64)-1;

	if (mode==MODE_FIRMWARE_UPDATE)
	{
		l1 += snprintf_P(szGetQuery+l1,max(0,SIZE_GET_QUERY-l1),PSTR("/GET_HEX?PASS=cirippi&AWSID="));
	} 
	else
	{
		l1 += snprintf_P(szGetQuery+l1,max(0,SIZE_GET_QUERY-l1),PSTR("/GET_CFG_HEX?PASS=cirippi&AWSID="));
	}
	

	if( AC_ERROR_OK != cfg_find_item(CFG_TAG_DATALOGGER_AWS_ID,&f))
	{
		debug_string_1P(NORMAL,PSTR("[ERROR] Missing configuration file"));
		wdt_reset_mcu();
	}
	cfg_get_item_file(f,szGetQuery+l1,64);
	
	
	l1 += strnlen(szGetQuery+l1,64);
	
	const uint16_t lp = l1;

	l1 += snprintf_P(szGetQuery+l1,max(0,SIZE_GET_QUERY-l1),PSTR("&BUFLEN=%d&STARTLINE="),SIZE_GET_ANSWER);


	RET_ERROR_CODE err = sim900_bearer_simple_open();

	uint8_t i=0, j=0;

	//HACK: the following loop works only if the first field (size of FW in bytes) is smaller than 8 chars
	for( ; (i<8) && (pPara[i]!=0) && (j<1); ++i) { 
		if(pPara[i]==',') j++;
	}

	if(j!=1) {
		debug_string(NORMAL,pPara,RAM_STRING);
		debug_string_1P(NORMAL,PSTR("\r\n[ERROR] unable to get num of lines, aborting\r\n"));
		wdt_reset_mcu();
	}

	EM_BUFFER em_buffer;

	const uint16_t nLines = atoi(pPara+i);

/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//char buf[32];
	//int s;
/////////////////////////////////////////////////////////////////////////////////////////////////////////


	bool success = true;
	em_init(&em_buffer,mode);
	crc_set_initial_value(0xFFFFFFFF);
	crc_io_checksum_byte_start(CRC_32BIT);

	uint16_t lid=0;
	while(nLines>lid) {
		int lenRetBuf=SIZE_GET_ANSWER;

		const int dd = SIZE_GET_QUERY-l1;
		if(dd<=0) {
			success = false;
			goto quit_update;
		}
		
		snprintf_P(szGetQuery+l1,dd,PSTR("%d"),lid);
		err = sim900_http_get(szGetQuery,RAM_STRING,szGetAnswer,&lenRetBuf,true);
		sim900_http_close();

		if(err!=AC_ERROR_OK) {
			continue;
		}
		
/////////////////////////////////////////////////////////////////////////////////////////////////////////		
		//s = lenRetBuf;
		//sprintf_P(buf,PSTR("LENRETBUF = %d\r\n"),s);
		//debug_string(NORMAL,buf,RAM_STRING);
/////////////////////////////////////////////////////////////////////////////////////////////////////////

		uint16_t idxLine = 0;

		err = fw_update_process_hex_chunk(&em_buffer,szGetAnswer,lenRetBuf,&idxLine,mode);

		if (err!=AC_ERROR_OK)	{
			//We got EOF while processing the chunk
			debug_string_1P(NORMAL,PSTR("[ERROR] FW_UPDATE got EOF while processing hex record, aborting process"));

			success = false;
			goto quit_update;
		} 
		
		for (uint16_t ix=0;ix<lenRetBuf;++ix) {
			const uint8_t c = szGetAnswer[ix];
			if ( (c!='\r') && (c!='\n') )
			{
				crc_io_checksum_byte_add(c);
			//	usart_putchar(USART_DEBUG,c);
			} 
/////////////////////////////////////////////////////////////////////////////////////////////////////////
			//else {
				//s = c;
				//sprintf_P(buf,PSTR("\r\nSTRIPPED CHAR: %d\r\n"),s);
				//debug_string(NORMAL,buf,RAM_STRING);
			//}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
			
		}
		
		lid += idxLine;
	}
	
	debug_string_1P(NORMAL,PSTR("[INFO] FW_UPDATE reached the end of file, proceeding with update"));

	em_flush(&em_buffer,mode);
	
	uint16_t lenRetBuf=SIZE_GET_ANSWER;
			
	snprintf_P(szGetQuery+lp,SIZE_GET_QUERY-lp,PSTR("&BUFLEN=-1&STARTLINE=-1"));
	err = sim900_http_get(szGetQuery,RAM_STRING,szGetAnswer,&lenRetBuf,true);

	szGetAnswer[lenRetBuf] = 0;
	
	debug_string_1P(NORMAL,PSTR("[INFO] FW_UPDATE server sent checksum :"));
	debug_string(NORMAL,szGetAnswer,RAM_STRING);
	debug_string_1P(NORMAL,g_szCRLF);
	
	if (MODE_FIRMWARE_UPDATE==mode)
	{
		uint32_t dwcrc = atol(szGetAnswer);
		uint32_t eecrc = crc_io_checksum_byte_stop();

		
		debug_string_1P(NORMAL,PSTR("[INFO] FW_UPDATE client checksum :"));
		sprintf_P(szGetAnswer,PSTR("%lu\r\n"),eecrc);
		debug_string(NORMAL,szGetAnswer,RAM_STRING);
		

		if (dwcrc!=eecrc)
		{
			success = false;
		}
	}
		
			

quit_update:

	em_close(success, mode);
	
	sim900_http_close();
	//sim900_bearer_close();
	
	wdt_enable();
	wdt_reset_mcu();
	debug_string_1P(NORMAL,PSTR("FW_UPDATE terminated, reboot"));
	while(1) 
	{
		delay_ms(1000);
	}
 }

