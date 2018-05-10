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

#include "Acronet/setup.h"
#include "Acronet/HAL/hal_interface.h"


#include <asf.h>
#include <stdio.h>
#include "progmem.h"
#include <conf_board.h>
#include <conf_usart_serial.h>


#include "Acronet/globals.h"

#include "Acronet/utils/AnyType/AnyType.h"
#include "Acronet/services/config/config.h"
#include "Acronet/drivers/AT24CXX/AT24CXX.h"
//#include "services/DB/db_conf.h"
#include "Acronet/services/DB/DB.h"
#include "Acronet/services/LOG/LOG.h"

//static AT24CXX_iterator g_log_iter_beg;
//static AT24CXX_iterator g_log_iter_end;

static AT24CXX_iterator g_log_iter_beg;
static AT24CXX_iterator g_log_iter_end;

enum {LOG_TAG_BEG=0xBB,LOG_TAG_END=0xEE,LOG_TAG_DEL=0xDD};

static uint16_t my_iterator_distance(const AT24CXX_iterator itBeg,const AT24CXX_iterator itEnd)
{
	const int32_t len = itEnd.plain - itBeg.plain;
	
	if(len<0)
	{
		return  (uint16_t) (len + LOG_EEPROM_PARTITION_END - (LOG_EEPROM_PARTITION_BEGIN+1));
	}
	
	return (uint16_t) len - 1;	
}


static uint16_t my_log_get_size(void)
{
	return my_iterator_distance(g_log_iter_beg,g_log_iter_end);	
}


static void my_iterator_next_page(AT24CXX_iterator * const it)
{
	AT24CXX_iterator a = { .plain = it->plain + 256 };

	if (a.plain > LOG_EEPROM_PARTITION_END)
	{
		//it->plain = PARTITION_LOG_BEGIN + 4;
		it->plain = LOG_EEPROM_PARTITION_BEGIN;
	} else {
		it->plain = a.plain;
	}
}

static void my_iterator_add_offset(AT24CXX_iterator * const it,uint16_t offset)
{
	AT24CXX_iterator t = { .plain = it->plain + offset};
	
	while (t.plain > LOG_EEPROM_PARTITION_END)
	{
		t.plain -= LOG_EEPROM_PARTITION_END - (LOG_EEPROM_PARTITION_BEGIN);
	}

	*it = t;
}

static void my_iterator_inc(AT24CXX_iterator * const it)
{
	AT24CXX_iterator t = { .plain = it->plain + 1};
	
	if(t.plain >= LOG_EEPROM_PARTITION_END)
	{
		//it->plain = (PARTITION_LOG_BEGIN+4);
		it->plain = (LOG_EEPROM_PARTITION_BEGIN);
	} else {
		it->plain = t.plain;
	}

}

static void my_iterator_dec(AT24CXX_iterator * const it)
{
	AT24CXX_iterator t = { .plain = it->plain - 1};
	
	//if(t.plain < (PARTITION_LOG_BEGIN+4))
	if(t.plain < (LOG_EEPROM_PARTITION_BEGIN))
	{
		it->plain = LOG_EEPROM_PARTITION_END-1;
	} else {
		it->plain = t.plain;
	}

}


RET_ERROR_CODE LOG_reset( void )
{

	const AT24CXX_iterator it = { .plain = LOG_EEPROM_PARTITION_BEGIN };

	uint8_t szCode[2] = {	LOG_TAG_BEG,
							LOG_TAG_END
						};


	RET_ERROR_CODE r = AT24CXX_WriteBlock(	it.byte[PAGE_BYTE],
											it.byte[MSB_BYTE],
											it.byte[LSB_BYTE],
											szCode,sizeof(szCode));

	delay_ms(5);

	if (AC_ERROR_OK != r)
	{
		return r;
	}


	g_log_iter_beg.plain = LOG_EEPROM_PARTITION_BEGIN  ;
	g_log_iter_end.plain = LOG_EEPROM_PARTITION_BEGIN + 1 ;

	return r;
}

//static RET_ERROR_CODE LOG_reset_test( void )
//{
//
	//AT24CXX_iterator it = { .plain = PARTITION_LOG_BEGIN };
//
	//uint8_t szCode[256];
	//
	//memset(szCode,0,sizeof(szCode));
//
//
	//RET_ERROR_CODE r = AT24CXX_WritePage(	it.byte[PAGE_BYTE],
											//it.byte[MSB_BYTE]++,
											//szCode);
//
	//delay_ms(5);
//
	//if (AC_ERROR_OK != r)
	//{
		//return r;
	//}
//
	//memset(szCode,0,sizeof(szCode));
//
	//r = AT24CXX_WritePage(	it.byte[PAGE_BYTE],
											//it.byte[MSB_BYTE]++,
											//szCode);
//
	//delay_ms(5);
//
	//if (AC_ERROR_OK != r)
	//{
		//return r;
	//}
//
//
	//r = AT24CXX_WritePage(	it.byte[PAGE_BYTE],
											//it.byte[MSB_BYTE]++,
											//szCode);
//
	//delay_ms(5);
//
	//if (AC_ERROR_OK != r)
	//{
		//return r;
	//}
//
	//szCode[250] = LOG_TAG_BEG;
	//szCode[251] = LOG_TAG_END;
//
	//r = AT24CXX_WritePage(	it.byte[PAGE_BYTE],
							//it.byte[MSB_BYTE],
							//szCode);
//
	//delay_ms(5);
//
	//g_log_iter_beg.plain = it.plain + 250 ;
	//g_log_iter_end.plain = it.plain + 251 ;
 //
 	//if (AC_ERROR_OK != r)
	//{
		//return r;
	//}
//
//
	//return r;
//}


static RET_ERROR_CODE  find_TAG(const uint8_t ui8TAG,AT24CXX_iterator * itPos)
{
	AT24CXX_iterator it = { .plain = LOG_EEPROM_PARTITION_BEGIN };		

	uint8_t buf[4];
		
	while(it.plain < LOG_EEPROM_PARTITION_END)
	{
		const RET_ERROR_CODE r = AT24CXX_ReadBlock(	it.byte[PAGE_BYTE],
													it.byte[MSB_BYTE],
													it.byte[LSB_BYTE],
													buf, sizeof(buf));
		//delay_ms(2);//TODO: TEST
													
			
		if (AC_ERROR_OK != r)
		{
			return r;
		}
			
			
		if (buf[0]==ui8TAG) {
			break;
		} else if (buf[1]==ui8TAG) {
			AT24CXX_iterator_add(&it,1);
			break;
		} else if (buf[2]==ui8TAG) {
			AT24CXX_iterator_add(&it,2);
			break;
		} else if (buf[3]==ui8TAG) {
			AT24CXX_iterator_add(&it,3);
			break;
		}
		
		AT24CXX_iterator_add(&it,4);
			
	}

	*itPos = it;
	return AC_ERROR_OK;
}

RET_ERROR_CODE LOG_init( void )
{
	AT24CXX_Init();
	
	AT24CXX_iterator it = { .plain = LOG_EEPROM_PARTITION_BEGIN };
	

	////////////////////////////////////////////////////////////
	//
	// get the location of the Begin TAG
	//
	
    RET_ERROR_CODE r = find_TAG( LOG_TAG_BEG ,&it);
    if(AC_ERROR_OK != r)
    {
	    return r;
    }
    
    g_log_iter_beg.plain = it.plain;


	////////////////////////////////////////////////////////////
	//
	// get the the location of the End TAG
	//
	
    r = find_TAG(LOG_TAG_END,&it);
	if(AC_ERROR_OK != r)
	{
		return r;
	}
	
	g_log_iter_end.plain = it.plain;
	
	debug_string_1P(NORMAL,PSTR("LOG iterators\r\n"));
	AT24CXX_iterator_report(g_log_iter_beg);
	AT24CXX_iterator_report(g_log_iter_end);
	
	if ((g_log_iter_beg.plain == LOG_EEPROM_PARTITION_END) || (g_log_iter_end.plain == LOG_EEPROM_PARTITION_END))
	{
		debug_string_1P(NORMAL,PSTR("One or more LOG Iterators are missing\r\n"
									"probably due to a forced reboot."
									" Doing a mandatory reset\r\n") );
									
		LOG_reset();
	}
	
	return AC_ERROR_OK;
}

#define SIZE_OF_LOG_BUFFER 256
static uint8_t  log_buffer[SIZE_OF_LOG_BUFFER+1]; //Space for the END TAG
static uint16_t log_buffer_index = 0;


RET_ERROR_CODE LOG_say(const char * const pszStatement)
{
	//debug_string_1P(VERBOSE,PSTR("Log says: "));
	//debug_string(VERBOSE,pszStatement,RAM_STRING);
	//debug_string_1P(VERBOSE,g_szCRLF);
	
	uint16_t i = 0;
	for (;;)
	{
		const uint16_t off = log_buffer_index + i;
		
		if (off>=SIZE_OF_LOG_BUFFER)
		{
			debug_string_1P(NORMAL,PSTR("[ERROR] LOG Buffer overflow\r\n"));
			//Disaster! TODO:manage properly
			log_buffer[SIZE_OF_LOG_BUFFER-1] = 0;
			log_buffer[SIZE_OF_LOG_BUFFER] = LOG_TAG_END;  //Dimension of the buffer has one byte left for this TAG
			break;
		}

		const char c = pszStatement[i++];
		log_buffer[off] = (uint8_t) c;
		
		if (c==0)
		{
			log_buffer[off+1] = LOG_TAG_END;  
			break;
		}
		
	}
	
	log_buffer_index += i;
	
	return AC_ERROR_OK;
}

static RET_ERROR_CODE LOG_make_it_empty(void)
{

	uint8_t ui8TAG = LOG_TAG_DEL;	
	
	RET_ERROR_CODE r = AT24CXX_WriteBlock(	g_log_iter_beg.byte[PAGE_BYTE],
											g_log_iter_beg.byte[MSB_BYTE],
											g_log_iter_beg.byte[LSB_BYTE],
											&ui8TAG, 1);

	delay_ms(5);

	if (AC_ERROR_OK != r)
	{
		return r;
	}
												
	
	my_iterator_dec(&g_log_iter_beg);
	
	ui8TAG = LOG_TAG_BEG;	
	r = AT24CXX_WriteBlock(	g_log_iter_beg.byte[PAGE_BYTE],
							g_log_iter_beg.byte[MSB_BYTE],
							g_log_iter_beg.byte[LSB_BYTE],
							&ui8TAG, 1);
	delay_ms(5);
	
	return r;
}


static RET_ERROR_CODE LOG_flush_2(	const AT24CXX_iterator * const itBeg, 
							const AT24CXX_iterator * const itEnd,
							uint8_t * pBuffer,uint16_t * pBufferSize)
{
	uint8_t pro = 256 - itBeg->byte[LSB_BYTE];
	uint8_t epi = itEnd->byte[MSB_BYTE];
//	uint16_t pages = ((itEnd->plain & 0x00FFFF00) - (itBeg->plain & 0x00FFFF00))>>8;
//	uint16_t pages = (itEnd->plain >> 8) - (itBeg->plain >> 8); <-- not safe 
	uint16_t pages = itEnd->byte[MSB_BYTE] + (((uint16_t) itEnd->byte[PAGE_BYTE])<<8) 
				   - itBeg->byte[MSB_BYTE] + (((uint16_t) itBeg->byte[PAGE_BYTE])<<8);

	AT24CXX_ReadBlock(	itBeg->byte[PAGE_BYTE],
						itBeg->byte[MSB_BYTE],
						itBeg->byte[LSB_BYTE],pBuffer,pro);
						
	uint8_t * pb = pBuffer + pro;

	AT24CXX_iterator it = {.plain = itBeg->plain + pro};
	
	
	for (uint16_t p=0;p<pages;++p)
	{
			AT24CXX_ReadPage(	it.byte[PAGE_BYTE],
								it.byte[MSB_BYTE],
								pb );
			pb += 256;
			it.plain += 256;
	}
		
	if (epi>0)
	{
		AT24CXX_ReadBlock(	itEnd->byte[PAGE_BYTE],
							itEnd->byte[MSB_BYTE],
							itEnd->byte[LSB_BYTE],pb,epi);
							
		pb += epi;

	}
	
	*pBufferSize = pb - pBuffer;
	return AC_ERROR_OK;
}

RET_ERROR_CODE LOG_flush(uint8_t * const pBuffer,uint16_t * pBufferSize)
{
	AT24CXX_Init();
	
	static const AT24CXX_iterator dummy = { .plain = LOG_EEPROM_PARTITION_BEGIN };		
	
	AT24CXX_iterator it[2][2] = { { g_log_iter_beg, g_log_iter_end } ,
								  { dummy , g_log_iter_end } };
	uint8_t idx=1;
									  
	if(it[0][1].plain<g_log_iter_beg.plain)
	{
		it[0][1].plain = LOG_EEPROM_PARTITION_END;
		idx = 2;
	}
	
	uint16_t offset = 0;
	uint16_t size = *pBufferSize;
	
	while( idx-- > 0 ) {
		LOG_flush_2(&it[idx][0],&it[idx+1][1],pBuffer+offset,&size);
		offset += size;
		size = *pBufferSize - offset;
	};
	
	*pBufferSize = size;
	
	return LOG_make_it_empty();
}

static RET_ERROR_CODE LOG_process_buffer_2(const AT24CXX_iterator itBeg,uint8_t * const pBuffer, uint32_t len)
{
	AT24CXX_Init();
	
	const AT24CXX_iterator itEnd = {.plain = itBeg.plain + len};

	const uint16_t pro = min(256-itBeg.byte[LSB_BYTE],len);

	const uint16_t pages = itEnd.byte[MSB_BYTE] + (((uint16_t) itEnd.byte[PAGE_BYTE])<<8)
			    	     -(itBeg.byte[MSB_BYTE] + (((uint16_t) itBeg.byte[PAGE_BYTE])<<8));

	AT24CXX_WriteBlock(	itBeg.byte[PAGE_BYTE],
						itBeg.byte[MSB_BYTE],
						itBeg.byte[LSB_BYTE],pBuffer,pro);
						
	delay_ms(5);
	
	uint8_t * pb = pBuffer + pro;

	AT24CXX_iterator it = {.plain = itBeg.plain + pro};
	
	
	for (uint16_t p=2;p<pages;++p)
	{
		AT24CXX_WritePage(	it.byte[PAGE_BYTE],
							it.byte[MSB_BYTE],
							pb );
		delay_ms(5);
							
		pb += 256;
		it.plain += 256;
	}
	
	const uint16_t epi =itEnd.byte[LSB_BYTE];
	if ((pages>0) && (epi>0))
	{
		
		AT24CXX_WriteBlock(	it.byte[PAGE_BYTE],
							it.byte[MSB_BYTE],
							it.byte[LSB_BYTE],pb,epi);
		delay_ms(5);

	}
	
	return AC_ERROR_OK;
}


RET_ERROR_CODE LOG_process_buffer(void)
{
	
	if (log_buffer_index==0)
	{
		return AC_NOTHING_TO_DO;
	}
	
	uint32_t freespace[2];
	
	if(g_log_iter_end.plain<g_log_iter_beg.plain)
	{
		freespace[0] = g_log_iter_beg.plain - g_log_iter_end.plain - 1;
		freespace[1] = 0;
	} else {
		freespace[0] = LOG_EEPROM_PARTITION_END - g_log_iter_end.plain;
		freespace[1] = g_log_iter_beg.plain - LOG_EEPROM_PARTITION_BEGIN ;
	}
	
	const uint16_t d = log_buffer_index + 1; //In this way we take the END TAG 
	
	if (freespace[0]>=d)	
	{
		LOG_process_buffer_2(g_log_iter_end,log_buffer,d);
	} else {
		if(freespace[0]>0) 
		{
			LOG_process_buffer_2(g_log_iter_end,log_buffer,freespace[0]);	
		}
		if (freespace[1]>0)
		{
			const AT24CXX_iterator it = {.plain = LOG_EEPROM_PARTITION_BEGIN };				
			const uint16_t v = min(d-freespace[0],freespace[1]);
			uint8_t * const p = log_buffer + freespace[0];
			LOG_process_buffer_2(it,p,v);
		}
	} 
	
	my_iterator_add_offset(&g_log_iter_end,log_buffer_index);
	
	log_buffer_index = 0;
	
	return AC_ERROR_OK;
}

RET_ERROR_CODE LOG_get_length(uint32_t * const pLen)
{
	if (pLen==NULL)
	{
		return AC_BAD_PARAMETER;
	}
	
	const uint16_t len = my_log_get_size();
	
	*pLen = len;
	return AC_ERROR_OK;
}

RET_ERROR_CODE LOG_lock(LOG_LOCK_PARAMS * const pPara)
{

	AT24CXX_Init();
	
	pPara->it.plain = g_log_iter_beg.plain;
	
	return AT24CXX_ReadPage(	g_log_iter_beg.byte[PAGE_BYTE],
								g_log_iter_beg.byte[MSB_BYTE],
								pPara->buffer);

	delay_ms(2);
}

RET_ERROR_CODE LOG_unlock( LOG_LOCK_PARAMS * const pPara )
{
	
	uint8_t bb = LOG_TAG_DEL;

	RET_ERROR_CODE r = AT24CXX_WriteBlock(	g_log_iter_beg.byte[PAGE_BYTE],
											g_log_iter_beg.byte[MSB_BYTE],
											g_log_iter_beg.byte[LSB_BYTE],&bb,1);
	delay_ms(5);

	if (AC_ERROR_OK != r)
	{
		return r;
	}

	g_log_iter_beg = pPara->it;
	 
	bb = LOG_TAG_BEG;
	r = AT24CXX_WriteBlock(	g_log_iter_beg.byte[PAGE_BYTE],
							g_log_iter_beg.byte[MSB_BYTE],
							g_log_iter_beg.byte[LSB_BYTE],&bb,1);
	delay_ms(5);

	
	return r;
}


RET_ERROR_CODE LOG_get_next_string( LOG_LOCK_PARAMS * const pPara, char * szDest, uint16_t * szLen )
{
	const uint16_t ce = min(*szLen,my_iterator_distance(pPara->it,g_log_iter_end));
	
	if(ce==0) {
		return AC_NOTHING_TO_DO;
	}
	
	AT24CXX_iterator * const pi = &pPara->it;
	uint8_t  bb = pPara->it.byte[LSB_BYTE];
	uint8_t * const pb = pPara->buffer;
	uint16_t l=0;
	
	while(l<ce) {

		const uint8_t c = pb[bb++];

		if(bb == 0) {
			my_iterator_next_page(pi);
			
			AT24CXX_ReadPage(	pi->byte[PAGE_BYTE],
								pi->byte[MSB_BYTE],
								pb);
			delay_ms(2);
		}

		if (c==LOG_TAG_BEG) {
			continue;			
		}
		
		szDest[l++] = c;
		
		if (c==0) {
			break;
		}
	}

	*szLen = l;
	pi->byte[LSB_BYTE] = bb;

	if (pb[bb]==LOG_TAG_END)
	{
		my_iterator_dec(pi);
	}
	
	
	return AC_ERROR_OK;
	
}
