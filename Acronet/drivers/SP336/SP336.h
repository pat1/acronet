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


#ifndef SP336_H_
#define SP336_H_




//typedef enum {	SP336_MODE_LOOPBACK,
				//SP336_MODE_RS232,
				//SP336_MODE_MIXED_HALFDUP,
				//SP336_MODE_MIXED_FULLDUP,
				//SP336_MODE_RS485_HALFDUP,
				//SP336_MODE_LOWPOWER_RX,
				//SP336_MODE_HIGHZ
//} SP336_MODE;

	

typedef void (*SP336_CALLBACK) (const char);


RET_ERROR_CODE SP336_Init	(void);
RET_ERROR_CODE SP336_Config (USART_t * const pUsart,const usart_rs232_options_t * const pConf);
void SP336_RX_Disable (USART_t * const pUsart);
void SP336_RX_Enable (USART_t * const pUsart);
RET_ERROR_CODE SP336_RegisterCallback (USART_t * const id,SP336_CALLBACK fn);

RET_ERROR_CODE SP336_PutChar (USART_t * const  id, const char c);
RET_ERROR_CODE SP336_PutString (USART_t * const id, const char * const sz);



RET_ERROR_CODE SP336_0_PutBuffer(const uint8_t * const pBuf,const uint16_t len);
RET_ERROR_CODE SP336_1_PutBuffer(const uint8_t * const pBuf,const uint16_t len);
RET_ERROR_CODE SP336_2_PutBuffer(const uint8_t * const pBuf,const uint16_t len);
RET_ERROR_CODE SP336_3_PutBuffer(const uint8_t * const pBuf,const uint16_t len);

#endif /* SP336_H_ */