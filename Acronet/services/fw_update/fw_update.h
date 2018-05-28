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



#ifndef FW_UPDATE_H_
#define FW_UPDATE_H_


enum {MODE_FIRMWARE_UPDATE=0,MODE_CONFIG_UPDATE=1};


RET_ERROR_CODE fw_update_init(const char * pPara);
void fw_update_run(const char * pPara,const uint8_t mode);
void fw_update_run_test( const char * pPara );


#endif /* FW_UPDATE_H_ */