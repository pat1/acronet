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

#pragma once

#define FLAG_BYTE 3
#define PAGE_BYTE 2
#define MSB_BYTE  1
#define LSB_BYTE  0

//#define AT24CXX_ReadBlock  AT24CXX_ReadSequentialA
#define AT24CXX_WriteBlock AT24CXX_WriteBlockA
#define AT24CXX_ReadBlock  AT24CXX_ReadBlockA


typedef union 
{
	uint32_t plain;
	uint8_t  byte[4];
} AT24CXX_iterator;


RET_ERROR_CODE AT24CXX_Init(void);

RET_ERROR_CODE AT24CXX_WriteBlockA(const uint8_t addr_page, const uint8_t addr_msb,const uint8_t addr_lsb,uint8_t outbuf[],const uint16_t buflen);
RET_ERROR_CODE AT24CXX_WriteBlockB(const uint8_t addr_page, const uint8_t addr_msb,const uint8_t addr_lsb,uint8_t outbuf[],const uint16_t buflen);

RET_ERROR_CODE AT24CXX_WritePage( const uint8_t addr_page, const uint8_t addr_msb,uint8_t outbuf[]);
RET_ERROR_CODE AT24CXX_ReadPage( const uint8_t addr_page, const uint8_t addr_msb,uint8_t inbuf[]);


RET_ERROR_CODE AT24CXX_ReadBlockA(const uint8_t addr_page, const uint8_t addr_msb,const uint8_t addr_lsb,uint8_t inbuf[],const uint16_t buflen);
RET_ERROR_CODE AT24CXX_ReadBlockB(const uint8_t addr_page, const uint8_t addr_msb,const uint8_t addr_lsb,uint8_t inbuf[],const uint16_t buflen);

void AT24CXX_iterator_report(const AT24CXX_iterator it);

//uint8_t AT24CXX_busy (void);


static inline uint8_t AT24CXX_idle (void)
{
	return ((EEPROM_TWI_PORT->MASTER.STATUS & TWI_MASTER_BUSSTATE_gm)== TWI_MASTER_BUSSTATE_IDLE_gc);
}

static inline uint8_t AT24CXX_iterator_valid(const AT24CXX_iterator * const it)
{
	return (it->byte[FLAG_BYTE]==0);
}

static inline void AT24CXX_iterator_add(AT24CXX_iterator * const it,const uint8_t dim)
{
	it->plain += dim;
}

static inline void AT24CXX_iterator_to_address(	const AT24CXX_iterator it,
												uint8_t * const addr_page,
												uint8_t * const addr_msb,
												uint8_t * const addr_lsb)
{
	*addr_page = it.byte[PAGE_BYTE];
	*addr_msb  = it.byte[MSB_BYTE];
	*addr_lsb  = it.byte[LSB_BYTE];
}

static inline void AT24CXX_address_to_iterator(	const uint8_t addr_page,
												const uint8_t addr_msb,
												const uint8_t addr_lsb,
												AT24CXX_iterator * const it)
{
	it->byte[PAGE_BYTE] = addr_page;
	it->byte[MSB_BYTE]   = addr_msb;
	it->byte[LSB_BYTE]   = addr_lsb;
}

