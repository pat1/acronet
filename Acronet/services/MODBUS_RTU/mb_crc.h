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



#ifndef MB_CRC_H_
#define MB_CRC_H_

void mb_crc_push(uint8_t regcrc[2]	,const uint8_t b);


static inline void mb_crc_reset(uint8_t regcrc[2])
{
	regcrc[0] = 0xFF;
	regcrc[1] = 0xFF;
}

static inline uint16_t mb_crc_get(uint8_t regcrc[2])
{
	return ( (((uint16_t) regcrc[0]) << 8) | regcrc[1] );
}



#endif /* MB_CRC_H_ */