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



#ifndef MODBUS_MASTER_RTU_H_
#define MODBUS_MASTER_RTU_H_

enum {	MBUS_STATUS_BEGIN=0,
		MBUS_STATUS_ADDR=MBUS_STATUS_BEGIN,
		MBUS_STATUS_FUNC,
		MBUS_STATUS_DATA_BYTE_COUNT,
		MB_STATUS_DATA_BYTE,
		MBUS_STATUS_CRC_HI,
		MBUS_STATUS_CRC_LO,
		MBUS_STATUS_END };

typedef struct {
	uint8_t status;

	uint8_t addr;
	uint8_t func;

	uint8_t transmission_crc[2];
} MBUS_CONTROL;

typedef struct {

	uint8_t addr;
	uint8_t func;
	uint8_t data_size;
	struct {
		uint8_t byte[255];
		uint8_t bc;
		
	} data;
	uint8_t crc_hi;
	uint8_t crc_lo;
	
} MBUS_PDU;

bool MBUS_is_empty(const uint8_t ch_id);
void MBUS_reset(const uint8_t ch_id);
uint8_t MBUS_get_byte(const uint8_t ch_id);

RET_ERROR_CODE MBUS_issue_cmd(const uint8_t ch_id,const uint8_t * const pBuf,uint16_t len);
RET_ERROR_CODE MBUS_build_dgram(MBUS_CONTROL * const pControl,MBUS_PDU * const pPDU,uint8_t b);


#endif /* MODBUS_MASTER_RTU_H_ */