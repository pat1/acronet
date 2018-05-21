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


#ifndef DB_H_
#define DB_H_

#include "Acronet/drivers/AT24CXX/AT24CXX.h"
#include "Acronet/drivers/ExtEEPROM/ext_eeprom.h"
//#include "Acronet/utils/AnyType/AnyType.h"

struct DB_RECORD;
typedef AT24CXX_iterator DB_ITERATOR;


//typedef ANY_TYPE DB_FIELD;

void DB_iterator_get_begin(DB_ITERATOR * const pIter);
void DB_iterator_get_end(DB_ITERATOR * const pIter);

void LOG_iterator_get_begin(DB_ITERATOR * const pIter);
void LOG_iterator_get_end(DB_ITERATOR * const pIter);


void DB_dump_all(void);
void DB_dump(DB_ITERATOR itBeg,DB_ITERATOR itEnd);

uint32_t DB_size(void);
uint32_t DB_iterator_distance(const DB_ITERATOR * const itBeg,const DB_ITERATOR * const itEnd);

void DB_iterator_moveback(DB_ITERATOR * pIter,uint16_t range);
void DB_iterator_moveforward(DB_ITERATOR * const pIter,uint16_t range);

RET_ERROR_CODE DB_init(uint16_t recSize);
RET_ERROR_CODE DB_iterator_read_from_eeprom(void);
RET_ERROR_CODE DB_iterator_write_to_eeprom(void);
RET_ERROR_CODE DB_iterator_init_eeprom(void);
RET_ERROR_CODE DB_reset_eeprom(void);
RET_ERROR_CODE DB_query_startup_code(char szCode[4]);
RET_ERROR_CODE DB_get_record(const DB_ITERATOR * const pIter,struct DB_RECORD * const pDS);

RET_ERROR_CODE DB_append_record(const struct DB_RECORD * const pDS);
RET_ERROR_CODE DB_put_record(DB_ITERATOR * const pIter,const struct DB_RECORD * const pDS);

RET_ERROR_CODE DB_mark_record(const DB_ITERATOR iter, const uint8_t flag);
//RET_ERROR_CODE DB_select(const struct DB_RECORD * const pRecord,const uint8_t fieldID, DB_FIELD * const pField);

#endif /* DB_H_ */