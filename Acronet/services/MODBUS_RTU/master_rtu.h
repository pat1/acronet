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

bool MBUS_is_empty(const uint8_t ch_id);
void MBUS_reset(const uint8_t ch_id);
uint8_t MBUS_get_byte(const uint8_t ch_id);

RET_ERROR_CODE MBUS_issue_cmd(const uint8_t ch_id,const uint8_t * const pBuf,uint16_t len);

////#ifdef MODBUS_CHAN_0
//bool MBUS_ch0_is_empty(void);
//void MBUS_ch0_reset(void);
//uint8_t MBUS_ch0_get_byte(void);
////#endif
////#ifdef MODBUS_CHAN_1
//bool MBUS_ch1_is_empty(void);
//void MBUS_ch1_reset(void);
//uint8_t MBUS_ch1_get_byte(void);
////#endif
////#ifdef MODBUS_CHAN_2
//bool MBUS_ch2_is_empty(void);
//void MBUS_ch2_reset(void);
//uint8_t MBUS_ch2_get_byte(void);
////#endif
////#ifdef MODBUS_CHAN_3
//bool MBUS_ch3_is_empty(void);
//void MBUS_ch3_reset(void);
//uint8_t MBUS_ch3_get_byte(void);
////#endif

#endif /* MODBUS_MASTER_RTU_H_ */