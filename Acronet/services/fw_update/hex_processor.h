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



#ifndef HEX_PROCESSOR_H_
#define HEX_PROCESSOR_H_


typedef struct HEX_READER_RECORD
{
	uint8_t len;
	uint16_t address;
	uint8_t type;
	uint8_t  chk;
	
	uint8_t data[255];
} HEX_READER_RECORD;

RET_ERROR_CODE hex_processor_init(const char * const pBuf,const uint16_t size);
RET_ERROR_CODE hex_processor_verify_rec(const HEX_READER_RECORD * const pRec );
RET_ERROR_CODE hex_processor_get_rec( HEX_READER_RECORD * pRec, char * const status );


#endif /* HEX_PROCESSOR_H_ */