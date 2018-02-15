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



#ifndef ANYTYPE_H_
#define ANYTYPE_H_


typedef struct ANY_TYPE {
	uint8_t typeIDX;
	union {
		uint8_t		_byteVal;
		uint16_t	_wordVal;
		uint32_t	_dwordVal;
		float		_floatVal;
		char		_qchar[4];
		flash_addr_t	_pFlashDictionary;
		void *			_pRAMDictionary;
		flash_addr_t	_pFlashString;
		void *			_pRAMString;
	};
} ANY_TYPE;




#endif /* ANYTYPE_H_ */