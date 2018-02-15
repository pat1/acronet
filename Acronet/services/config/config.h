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


#ifndef CONFIG_H_
#define CONFIG_H_


typedef enum {	CFG_TAG_DATALOGGER_TIMING		  = 0x0101,
				CFG_TAG_DATALOGGER_AWS_ID		  = 0x0102,
				CFG_TAG_DATALOGGER_SEND_URL		  = 0x0103,
				CFG_TAG_DATALOGGER_TIME_URL		  = 0x0104,
				CFG_TAG_DATALOGGER_LOG_ADDRESS	  = 0x0105,
				CFG_TAG_DATALOGGER_LOG_PORT		  = 0x0106,
				CFG_TAG_DATALOGGER_ID_USERNAME    = 0x0107,
				CFG_TAG_DATALOGGER_ID_PASSWORD    = 0x0108,
				CFG_TAG_DATALOGGER_AWS_PRETTYNAME = 0x0109,
				CFG_TAG_DATALOGGER_RMAP_SERVER    = 0x0150,
				CFG_TAG_DATALOGGER_RMAP_SPORT     = 0x0151,
				CFG_TAG_DATALOGGER_RMAP_USERNAME  = 0x0152,
				CFG_TAG_DATALOGGER_RMAP_PASSWORD  = 0x0153,
				CFG_TAG_DATALOGGER_RMAP_TOPIC     = 0x0154,
				CFG_TAG_DATALOGGER_CMDCHK_URL     = 0x0190,
				CFG_TAG_SIM_PIN_CODE			  = 0x0201,
				CFG_TAG_SIM_APN_LIST_ADDRESS	  = 0x0202,
				CFG_TAG_RAINGAUGE_PORT			  = 0x0301,
				CFG_TAG_RAINGAUGE_COEFF			  = 0x0302,
				CFG_TAG_CAP_SERVER_URL			  = 0x1001,
				CFG_TAG_CAP_NUM					  = 0x1002,
				CFG_TAG_CAP_SENDER_NAME			  = 0x1003,
				CFG_TAG_CAP_AREA_DESC			  = 0x1004,
				CFG_TAG_CAP_AREA_POLYGON		  = 0x1005

} CFG_ITEM_TAG;


//typedef uint16_t FILE_TAG;
typedef uint16_t CFG_ITEM_ADDRESS;

RET_ERROR_CODE cfg_check				(void);
RET_ERROR_CODE cfg_find_item			(CFG_ITEM_TAG	  item_tag		,CFG_ITEM_ADDRESS * item_address);
RET_ERROR_CODE cfg_get_item_BYTE		(CFG_ITEM_ADDRESS item_address	,uint8_t * pByte);
RET_ERROR_CODE cfg_get_item_DWORD		(CFG_ITEM_ADDRESS item_address	,uint32_t * pDWORD);
RET_ERROR_CODE cfg_get_item_file		(CFG_ITEM_ADDRESS item_address	,void * pBuf , uint16_t len);
RET_ERROR_CODE cfg_get_item_dictionary	(CFG_ITEM_ADDRESS item_address	,char szKey[], char szVal[], const uint8_t lenBufVal);

RET_ERROR_CODE cfg_upgrade_from_old(void);

//Old deprecated functions
uint8_t __DEPRECATED__ cfg_old_check(void);

void __DEPRECATED__ cfg_old_get_sim_pin (char szVal[], const uint8_t len);
void __DEPRECATED__ cfg_old_get_gprs_apn(char szKey[], char szVal[], const uint8_t lenBufVal);

void __DEPRECATED__ cfg_old_get_aws_id(char szVal[], const uint8_t len);
void __DEPRECATED__ cfg_old_get_service_url_time( char szVal[], const uint8_t len);
void __DEPRECATED__ cfg_old_get_service_url_send( char szVal[], const uint8_t len);
void __DEPRECATED__ cfg_get_service_url_send_post(char szVal[], const uint8_t len);
//void __DEPRECATED__ cfg_old_get_service_url_send(char szVal[],const uint8_t len);


void __DEPRECATED__ cfg_old_get_datalogger_timings(uint32_t * const sendDT,uint32_t * const storeDT,uint32_t * const syncDT,uint32_t * const tickDT);


#endif /* CONFIG_H_ */