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


#include <asf.h>
#include "Acronet/globals.h"
#include "cbuffer_usart.h"





/*! \brief RX Complete Interrupt Service Routine.
 *
 *  RX Complete Interrupt Service Routine.
 *  Stores received data in RX software buffer.
 *
 *  \param usart_data      The USART_data_t struct instance.
 */
bool USART_RX_CBuffer_Complete(USART_data_t * usart_data)
{
	USART_data_t * const ad = usart_data;

	//USART_Buffer_t * const bufPtr = &usart_data->buffer;
	/* Advance buffer head. */
	const uint8_t idx = ad->RX_Head;
	const uint8_t tempRX_Head = (idx + 1) & USART_RX_BUFFER_MASK;

	/* Check for overflow. */

	if (tempRX_Head == ad->RX_Tail) {
		ad->RX_Tail = (tempRX_Head + 1) & USART_RX_BUFFER_MASK;
	}
	
	const uint8_t data = ad->usart->DATA;
	ad->RX[idx] = data;
	ad->RX_Head = tempRX_Head;

//	if(g_log_verbosity>=VERY_VERBOSE) {
	if(0) {
		if((data>31) && (data<127)) {
			usart_putchar(USART_DEBUG, data);
		} else {
			static const __flash char decoder[] = "0123456789ABCDEF ";
			usart_putchar(USART_DEBUG, '[');
			const uint8_t u = (data & 0x0F);
			const uint8_t d = (data >> 8);
			char c = decoder[d];
			usart_putchar(USART_DEBUG, c);
			c = decoder[u];
			usart_putchar(USART_DEBUG, c);
			usart_putchar(USART_DEBUG, ']');
		}
	}
	//usart_putchar(USART_DEBUG, data);


	return true;
}

