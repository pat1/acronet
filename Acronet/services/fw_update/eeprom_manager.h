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



#ifndef EEPROM_MANAGER_H_
#define EEPROM_MANAGER_H_

typedef struct
{
	AT24CXX_iterator it;
	
	uint8_t	buf[256];
} EM_BUFFER;


RET_ERROR_CODE em_init(EM_BUFFER * const g_emr, const uint8_t mode);
RET_ERROR_CODE em_close(const bool success, const uint8_t mode);
RET_ERROR_CODE em_insert(EM_BUFFER * const g_emr, const HEX_READER_RECORD* const pRec,const uint8_t mode);

RET_ERROR_CODE em_flush( EM_BUFFER * const g_emr, const uint8_t mode );
RET_ERROR_CODE em_cfg_flush( void );

RET_ERROR_CODE em_compute_crc(EM_BUFFER * const g_emr, uint32_t * const crc);

#endif /* EEPROM_MANAGER_H_ */
