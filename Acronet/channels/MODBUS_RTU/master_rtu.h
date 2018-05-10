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

void MBUS_PDU_reset(MBUS_PDU * const pPDU);

//bool MBUS_is_empty(const uint8_t ch_id);
//void MBUS_reset(const uint8_t ch_id);
//uint8_t MBUS_get_byte(const uint8_t ch_id);

//RET_ERROR_CODE MBUS_issue_cmd(const uint8_t ch_id,const uint8_t * const pBuf,uint16_t len);
//uint8_t MBUS_build_dgram(MBUS_CONTROL * const pControl,MBUS_PDU * const pPDU,const uint8_t b);

#define MBUS_PREP(p1,p2,...)			p1##p2(__VA_ARGS__)

#define MBUS_RELEASE(ch)			MBUS_PREP( MBUS_release_CH	, ch ) 
#define MBUS_LOCK(ch)				MBUS_PREP( MBUS_lock_CH		, ch ) 
#define MBUS_IS_RIPE(ch,address)	MBUS_PREP( MBUS_is_ripe_CH , ch , address ) 
#define MBUS_GET_BYTE(ch)			MBUS_PREP( MBUS_get_byte_CH , ch ) 
//#define MBUS_RESET(ch)				MBUS_PREP( MBUS_reset_CH	, ch ) 
#define MBUS_ISSUE_CMD(ch,a1,a2)	MBUS_PREP( MBUS_issue_cmd_CH , ch , a1 , a2 )
#define MBUS_BUILD_DGRAM(ch,a1,a2)	MBUS_PREP( MBUS_build_dgram_CH , ch , a1 , a2 )
#define MBUS_GET_CRC(ch)			MBUS_PREP( MBUS_get_crc_CH	, ch ) 

RET_ERROR_CODE MBUS_lock_CH0(void);
RET_ERROR_CODE MBUS_lock_CH1(void);
RET_ERROR_CODE MBUS_lock_CH2(void);
RET_ERROR_CODE MBUS_lock_CH3(void);

void MBUS_release_CH0(void);
void MBUS_release_CH1(void);
void MBUS_release_CH2(void);
void MBUS_release_CH3(void);

bool MBUS_is_ripe_CH0(const uint8_t address);
bool MBUS_is_ripe_CH1(const uint8_t address);
bool MBUS_is_ripe_CH2(const uint8_t address);
bool MBUS_is_ripe_CH3(const uint8_t address);

uint8_t MBUS_get_byte_CH0(void);
uint8_t MBUS_get_byte_CH1(void);
uint8_t MBUS_get_byte_CH2(void);
uint8_t MBUS_get_byte_CH3(void);

uint16_t MBUS_get_crc_CH0(void);
uint16_t MBUS_get_crc_CH1(void);
uint16_t MBUS_get_crc_CH2(void);
uint16_t MBUS_get_crc_CH3(void);

RET_ERROR_CODE MBUS_issue_cmd_CH0(const uint8_t * const pBuf,uint16_t len);
RET_ERROR_CODE MBUS_issue_cmd_CH1(const uint8_t * const pBuf,uint16_t len);
RET_ERROR_CODE MBUS_issue_cmd_CH2(const uint8_t * const pBuf,uint16_t len);
RET_ERROR_CODE MBUS_issue_cmd_CH3(const uint8_t * const pBuf,uint16_t len);

uint8_t MBUS_build_dgram_CH0(MBUS_PDU * const pPDU,uint8_t b);
uint8_t MBUS_build_dgram_CH1(MBUS_PDU * const pPDU,uint8_t b);
uint8_t MBUS_build_dgram_CH2(MBUS_PDU * const pPDU,uint8_t b);
uint8_t MBUS_build_dgram_CH3(MBUS_PDU * const pPDU,uint8_t b);


#endif /* MODBUS_MASTER_RTU_H_ */