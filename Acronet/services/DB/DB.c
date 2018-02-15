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

#include "Acronet/setup.h"
#include "Acronet/HAl/hal_interface.h"

#include <asf.h>
#include <stdio.h>
#include "progmem.h"
#include <conf_board.h>

#include "Acronet/globals.h"
#include "Acronet/utils/AnyType/AnyType.h"
#include "Acronet/drivers/AT24CXX/AT24CXX.h"
#include "Acronet/services/DB/DB.h"


//Memory pointers and partitions
#define DL_EEPROM_ITERATORS 2
#define ITERATOR_DB_BEGIN 0
#define ITERATOR_DB_END 1


static uint16_t SIZE_OF_DB_RECORD=0;// 32


static DB_ITERATOR g_eeprom_iter[DL_EEPROM_ITERATORS];


void DB_iterator_get_begin(DB_ITERATOR * const pIter)
{
	*pIter = g_eeprom_iter[ITERATOR_DB_BEGIN];
}

void DB_iterator_get_end(DB_ITERATOR * const pIter)
{
	*pIter = g_eeprom_iter[ITERATOR_DB_END];
}

//void LOG_iterator_get_begin(DB_ITERATOR * const pIter)
//{
	//*pIter = g_eeprom_iter[ITERATOR_LOG_BEGIN];
//}
//
//void LOG_iterator_get_end(DB_ITERATOR * const pIter)
//{
	//*pIter = g_eeprom_iter[ITERATOR_LOG_END];
//}


void DB_dump_all(void)
{
	char szBuf[128];

	debug_string_1P(NORMAL,PSTR("Memory dump image BEGIN\r\n\r\n\r\n\r\n"));

	DB_ITERATOR it = {.plain = 0};
	uint8_t buf[16];
	AT24CXX_Init();

	for(;it.plain<PARTITION_DB_END;it.plain+=16)
	{
		snprintf_P(szBuf,sizeof(szBuf),PSTR("\r\n%03d:%03d:%03d\t"),it.byte[PAGE_BYTE],it.byte[MSB_BYTE],it.byte[LSB_BYTE]);
		debug_string(NORMAL,szBuf,RAM_STRING);

		uint8_t pg,msb,lsb;
		AT24CXX_iterator_to_address(it,&pg,&msb,&lsb);

		AT24CXX_ReadBlock(pg,msb,lsb,buf,16);
		for (uint8_t i=0;i<16;++i)
		{
			const uint8_t c = buf[i];
			sprintf_P(szBuf+(3*i),PSTR(" %02X"),c);
			if((c>31) && (c<127)) {
				szBuf[52+i] = c;
				} else {
				szBuf[52+i] = '.';
			}
		}
		szBuf[48] = 32;
		szBuf[49] = 32;
		szBuf[50] = 32;
		szBuf[51] = 32;

		szBuf[68] = 0;
			
		debug_string(NORMAL,szBuf,RAM_STRING);
		delay_ms(5);
	}

	debug_string_1P(NORMAL,PSTR("\r\n\r\n\r\n\r\nMemory dump image END\r\n\r\n\r\n\r\n"));
}



void DB_dump(DB_ITERATOR itBeg,DB_ITERATOR itEnd)
{
	char szBuf[128];

	debug_string_1P(NORMAL,PSTR("Memory dump image BEGIN\r\n\r\n\r\n\r\n"));

	DB_ITERATOR it = itBeg;
	uint8_t buf[16];
	AT24CXX_Init();

	for(;it.plain<itEnd.plain;it.plain+=16)
	{
		snprintf_P(szBuf,sizeof(szBuf),PSTR("\r\n%03d:%03d:%03d\t"),it.byte[PAGE_BYTE],it.byte[MSB_BYTE],it.byte[LSB_BYTE]);
		debug_string(NORMAL,szBuf,RAM_STRING);

		uint8_t pg,msb,lsb;
		AT24CXX_iterator_to_address(it,&pg,&msb,&lsb);

		AT24CXX_ReadBlock(pg,msb,lsb,buf,16);
		for (uint8_t i=0;i<16;++i)
		{
			const uint8_t c = buf[i];
			sprintf_P(szBuf+(3*i),PSTR(" %02X"),c);
			if((c>31) && (c<127)) {
				szBuf[52+i] = c;
				} else {
				szBuf[52+i] = '.';
			}
		}
		szBuf[48] = 32;
		szBuf[49] = 32;
		szBuf[50] = 32;
		szBuf[51] = 32;

		szBuf[68] = 0;
		
		debug_string(NORMAL,szBuf,RAM_STRING);
		delay_ms(5);
	}

	debug_string_1P(NORMAL,PSTR("\r\n\r\n\r\n\r\nMemory dump image END\r\n\r\n\r\n\r\n"));

}


/**************************************************************************************/
// Iterators are stored in a circular buffer stored from address 0x04 
// Dimension of such buffer has to be a power of 2
/**************************************************************************************/
#define DL_ITER_CBUFFER_LEN 0x08


RET_ERROR_CODE DB_iterator_read_from_eeprom(void)
{
	static const uint8_t S = sizeof(AT24CXX_iterator);
	static const AT24CXX_iterator TAG = {.plain = 0xFAFAFAFA};
	AT24CXX_iterator tmp;
	
	//Iterators are stored in the eeprom in a circular buffer
	//that allows a 1/8th of the writes on the singular address
	
	uint8_t i;
	uint8_t j;
	
	AT24CXX_Init();

	for(j=0;j<DL_EEPROM_ITERATORS;++j)	{
		const size_t idx = j*DL_ITER_CBUFFER_LEN;
		for(i=0;i<DL_ITER_CBUFFER_LEN;++i) {
			AT24CXX_iterator it = {.plain = (4+i*S)+idx};
			uint8_t msb,lsb,pg;
			AT24CXX_iterator_to_address(it,&pg,&msb,&lsb);
			AT24CXX_ReadBlock(pg,msb,lsb,(uint8_t*)&tmp,S);
			//delay_ms(5); //TODO: TEST
			//If we have found the TAG that defines the end of the Buffer
			//it means that at the previous position we can find the
			//actual value of the variable
			if(tmp.plain==TAG.plain) {
				const uint8_t l = (i-1) & (DL_ITER_CBUFFER_LEN-1);
				AT24CXX_iterator it = {.plain = (4+l*S)+idx};
				AT24CXX_iterator_to_address(it,&pg,&msb,&lsb);
				AT24CXX_ReadBlock(pg,msb,lsb,(uint8_t*)&g_eeprom_iter[j],S);
				//delay_ms(5); //TODO: TEST
				
				break;
			}
		}
	}

	return AC_ERROR_OK;
}

RET_ERROR_CODE DB_iterator_write_to_eeprom(void)
{
	static const uint16_t S = sizeof(AT24CXX_iterator);
	static const AT24CXX_iterator TAG = {.plain = 0xFAFAFAFA};
	AT24CXX_iterator tmp;
	
	//Iterators are stored in the eeprom in a circular buffer
	//that allows a 1/8th of the writes on the singular address
	
	uint8_t i;
	uint8_t j;
	
	AT24CXX_Init();


	for(j=0;j<DL_EEPROM_ITERATORS;++j)	{
		const size_t idx = j*DL_ITER_CBUFFER_LEN;
		for(i=0;i<8;++i) {
			AT24CXX_iterator it = {.plain = (4+i*S)+idx};
			uint8_t msb,lsb,pg;
			AT24CXX_iterator_to_address(it,&pg,&msb,&lsb);
			AT24CXX_ReadBlock(pg,msb,lsb,(uint8_t*)&tmp,S);
			//delay_ms(5);//TODO: TEST
			//If we have found the TAG that defines the end of the Buffer
			//it means that at the previous position we can find the
			//actual value of the variable
			if(tmp.plain==TAG.plain) {
				//It is a circular buffer so the previous
				//position may be at the end of the regular,linear, buffer
				const uint8_t l = (i-1) & (DL_ITER_CBUFFER_LEN-1);
				AT24CXX_iterator it = {.plain = (4+l*S)+idx};
				AT24CXX_iterator_to_address(it,&pg,&msb,&lsb);
				AT24CXX_ReadBlock(pg,msb,lsb,(uint8_t*)&tmp,S);
				//delay_ms(5);//TODO: TEST
				//Verifies that the value stored in the eeprom
				//it is not equal to the one that we want to write
				//in that case we simply skip
				if(tmp.plain==g_eeprom_iter[j].plain) {
					break;
				}
				//Values are different proceed on writing it
				AT24CXX_WriteBlock(pg,msb,lsb,(uint8_t*)&g_eeprom_iter[j],S);
				delay_ms(5);
				//Write the TAG in the next position
				const uint8_t m = (l+1) & (DL_ITER_CBUFFER_LEN-1);
				it.plain = (4+m*S)+idx;
				AT24CXX_iterator_to_address(it,&pg,&msb,&lsb);
				AT24CXX_WriteBlock(pg,msb,lsb,(uint8_t*)&TAG,S);
				delay_ms(5);
				break;
			}
		}
	}

	return AC_ERROR_OK;
}

RET_ERROR_CODE DB_iterator_init_eeprom(void)
{
	static const uint16_t S = sizeof(AT24CXX_iterator);
	static const AT24CXX_iterator TAG = {.plain = 0xFAFAFAFA};
	
	//Iterators are stored in the eeprom in a circular buffer
	//that allows a 1/8th of the writes on the singular address
	
	uint8_t i;
	uint8_t j;
	
	AT24CXX_Init();

	for(j=0;j<DL_EEPROM_ITERATORS;++j)	{
		const size_t idx = j*DL_ITER_CBUFFER_LEN;
		
		AT24CXX_iterator it = {.plain = 4+idx};
		uint8_t msb,lsb,pg;
		AT24CXX_iterator_to_address(it,&pg,&msb,&lsb);
		AT24CXX_WriteBlock(pg,msb,lsb,(uint8_t*)&g_eeprom_iter[j],S);
		delay_ms(5);

		
		for(i=1;i<DL_ITER_CBUFFER_LEN;++i) {
			AT24CXX_iterator it = {.plain = (4+i*S)+idx};
			uint8_t msb,lsb,pg;
			AT24CXX_iterator_to_address(it,&pg,&msb,&lsb);
			AT24CXX_WriteBlock(pg,msb,lsb,(uint8_t*)&TAG,S);
			delay_ms(5);
		}
	}
	return AC_ERROR_OK;
}

static void reset_iterators(void)
{
	g_eeprom_iter[ITERATOR_DB_BEGIN].plain  = PARTITION_DB_BEGIN;
	g_eeprom_iter[ITERATOR_DB_END].plain  = PARTITION_DB_BEGIN;

	//g_eeprom_iter[ITERATOR_LOG_BEGIN].plain  = PARTITION_LOG_BEGIN;
	//g_eeprom_iter[ITERATOR_LOG_END].plain  = PARTITION_LOG_BEGIN;
	
}

RET_ERROR_CODE DB_reset_eeprom(void)
{

	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"DB reset eeprom");

	char szBUF[4] = {'A','C','0','1'};
	
	AT24CXX_WriteBlock(0,0,0,(uint8_t*)szBUF,4);
	delay_ms(5);

	reset_iterators();

	DB_iterator_init_eeprom();


	return AC_ERROR_OK;
}

RET_ERROR_CODE DB_init( uint16_t recSize )
{
	
	SIZE_OF_DB_RECORD = recSize;
	
	for (uint8_t i=0;i<DL_EEPROM_ITERATORS;++i)
	{
		g_eeprom_iter[i].plain = 0xFFFFFFFF;
	}
		
	DB_iterator_read_from_eeprom();

	debug_string_2P(NORMAL,PSTR("DB_init"),PSTR("eeprom iterators are :"));
	
	
	bool isundef = false;

	for (uint8_t i=0;i<DL_EEPROM_ITERATORS;++i)
	{
		if(g_eeprom_iter[i].plain == 0xFFFFFFFF) isundef = true;
		AT24CXX_iterator_report(g_eeprom_iter[i]);
	}

	//if the eeprom has to be initialized
	if(isundef)
	{
			
		debug_string_2P(NORMAL,PSTR("DB_init"),PSTR("resetting iterators\r\n") );
			

		reset_iterators();
			
		DB_iterator_init_eeprom();
	}
	
	return AC_ERROR_OK;

}

RET_ERROR_CODE DB_query_startup_code(char szCode[4])
{
	AT24CXX_Init();
	AT24CXX_ReadBlock(0,0,0,(uint8_t*)szCode,4);
	//delay_ms(5);//TODO: TEST
	
	return AC_ERROR_OK;
}


void DB_iterator_moveback(DB_ITERATOR * pIter,uint16_t range)
{
	const int32_t n = range*SIZE_OF_DB_RECORD;
	const int32_t x = PARTITION_DB_BEGIN+n;
	const int32_t z = pIter->plain;

	if(z<x) {
		debug_string_2P(NORMAL,PSTR("dl_iterator_moveback") ,PSTR("iterator wraps datalogger"));
		pIter->plain = PARTITION_DB_END - (x-z);
	} else {
		pIter->plain = z - n;
	}
}

void DB_iterator_moveforward(DB_ITERATOR * const pIter,uint16_t range)
{
	const int32_t n = range*SIZE_OF_DB_RECORD;
	const int32_t x = PARTITION_DB_END-n;
	const int32_t z = pIter->plain;

	if(z>x) {
		debug_string_2P(NORMAL,PSTR("dl_iterator_moveforward") ,PSTR("iterator wraps datalogger"));
		pIter->plain = PARTITION_DB_BEGIN + (z-x);
	} else {
		pIter->plain = z + n;
	}
}

RET_ERROR_CODE DB_get_record(const DB_ITERATOR * const pIter,struct DB_RECORD * const pDS)
{

	DB_ITERATOR it = *pIter;
	uint8_t pg,msb,lsb;
	AT24CXX_iterator_to_address(it,&pg,&msb,&lsb);
	DB_iterator_moveforward(&it,1);

	uint8_t * const pBuf = (uint8_t *) pDS;
	
	const uint8_t v = it.byte[LSB_BYTE];
	//is it a plain page or a overlapping read ?
	if ((v>=SIZE_OF_DB_RECORD) || (v==0)) {
		AT24CXX_ReadBlock(pg,msb,lsb,pBuf,SIZE_OF_DB_RECORD);
		//delay_ms(1);
	} else {
		const uint8_t x = (SIZE_OF_DB_RECORD - v);
		AT24CXX_ReadBlock(pg,msb,lsb,pBuf,x);
		//delay_ms(3);//TODO: TEST
		AT24CXX_ReadBlock(it.byte[PAGE_BYTE],it.byte[MSB_BYTE],0,pBuf+x,v);
	}
	
	return AC_ERROR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////
//This function modifies DB_ITERATOR in order to point to the new END position
//
RET_ERROR_CODE DB_put_record(DB_ITERATOR * const pIter,const struct DB_RECORD * const pDS)
{

	uint8_t pg,msb,lsb;
	AT24CXX_iterator_to_address(*pIter,&pg,&msb,&lsb);
	DB_iterator_moveforward(pIter,1);

	uint8_t * const pBuf = (uint8_t *) pDS;
	
	const uint8_t v = pIter->byte[LSB_BYTE];
	//is it a plain page or a overlapping read ?
	if ((v>=SIZE_OF_DB_RECORD) || (v==0)) {
		AT24CXX_WriteBlock(pg,msb,lsb,pBuf,SIZE_OF_DB_RECORD);
		delay_ms(5);
	} else {
		const uint16_t x = (SIZE_OF_DB_RECORD - v);
		AT24CXX_WriteBlock(pg,msb,lsb,pBuf,x);
		delay_ms(5);
		AT24CXX_WriteBlock(pIter->byte[PAGE_BYTE],pIter->byte[MSB_BYTE],0,pBuf+x,v);
		delay_ms(5);
	}
	
	return 0;
}

RET_ERROR_CODE DB_mark_record( const DB_ITERATOR iter, const uint8_t flag )
{
	uint8_t pg,msb,lsb;
	const uint8_t flags = flag;
	AT24CXX_iterator_to_address(iter,&pg,&msb,&lsb);
	RET_ERROR_CODE e = AT24CXX_WriteBlock(pg,msb,lsb,&flags,1);
	delay_ms(5);
	return e;
}


//////////////////////////////////////////////////////////////////////////////////////////
//This function modifies DB_ITERATOR in order to trim the BEGIN position
//
RET_ERROR_CODE DB_append_record(const struct DB_RECORD * const pDS)
{
	uint8_t ret = DB_put_record(g_eeprom_iter+ITERATOR_DB_END,pDS);

	const uint32_t off = g_eeprom_iter[ITERATOR_DB_END].plain - g_eeprom_iter[ITERATOR_DB_BEGIN].plain ;

	if (off < SIZE_OF_DB_RECORD)
	{
		debug_string_1P(NORMAL,PSTR("Avanzo BEGIN"));
		DB_iterator_moveforward(&g_eeprom_iter[ITERATOR_DB_BEGIN],1);
	}
	
	return ret;
}

uint32_t DB_size(void)
{
	
	if (g_eeprom_iter[ITERATOR_DB_END].plain > g_eeprom_iter[ITERATOR_DB_BEGIN].plain)
	{
		return	g_eeprom_iter[ITERATOR_DB_END].plain - g_eeprom_iter[ITERATOR_DB_BEGIN].plain;
	}
	else
	{
		return	  (PARTITION_DB_END - g_eeprom_iter[ITERATOR_DB_BEGIN].plain) 
				+ (g_eeprom_iter[ITERATOR_DB_END].plain - PARTITION_DB_BEGIN);
	}
}

uint32_t DB_iterator_distance(const DB_ITERATOR * const itBeg,const DB_ITERATOR * const itEnd)
{
	
	if (itEnd->plain > itBeg->plain)
	{
		return itEnd->plain - itBeg->plain;
	}
	else
	{
		return (PARTITION_DB_END - itBeg->plain) + (itEnd->plain - PARTITION_DB_BEGIN);
	}
}

