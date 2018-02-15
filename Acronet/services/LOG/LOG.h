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



#ifndef LOG_H_
#define LOG_H_

#include "Acronet/drivers/ExtEEPROM/ext_eeprom.h"
#include "Acronet/drivers/AT24CXX/AT24CXX.h"

//extern AT24CXX_iterator g_log_iter_beg;
//extern AT24CXX_iterator g_log_iter_end;


typedef struct {
	uint8_t  buffer[256];
	//uint8_t read_pos;

	AT24CXX_iterator it;	
} LOG_LOCK_PARAMS;


RET_ERROR_CODE LOG_reset( void );
//RET_ERROR_CODE LOG_reset_test( void );
RET_ERROR_CODE LOG_init( void );

RET_ERROR_CODE LOG_lock(LOG_LOCK_PARAMS * const pPara);
RET_ERROR_CODE LOG_unlock( LOG_LOCK_PARAMS * const pPara  );

RET_ERROR_CODE LOG_get_next_string( LOG_LOCK_PARAMS * const pPara, char * szDest, uint16_t * szLen );

RET_ERROR_CODE LOG_say(const char * const pszStatement);
RET_ERROR_CODE LOG_flush(uint8_t * const pBuffer,uint16_t * pBufferSize);

RET_ERROR_CODE LOG_process_buffer(void);

RET_ERROR_CODE LOG_get_length(uint32_t * const pLen);

#endif /* LOG_H_ */