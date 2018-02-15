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
#include "Acronet/globals.h"


#include <asf.h>
#include "Buffer_Usart.h"




/*! \brief Initializes buffer and selects what USART module to use.
 *
 *  Initializes receive and transmit buffer and selects what USART module to use,
 *  and stores the data register empty interrupt level.
 *
 *  \param usart_data           The USART_data_t struct instance.
 *  \param usart                The USART module.
 *  \param dreIntLevel          Data register empty interrupt level.
 */
void usart_interruptdriver_initialize(USART_data_t * usart_data,
                                      USART_t * usart,
                                      USART_DREINTLVL_t dreIntLevel)
{
	usart_data->usart = usart;
	usart_data->dreIntLevel = dreIntLevel;

	usart_data->RX_Tail = 0;
	usart_data->RX_Head = 0;
	usart_data->TX_Tail = 0;
	usart_data->TX_Head = 0;
}

/*! \brief Test if there is data in the transmitter software buffer.
 *
 *  This function can be used to test if there is free space in the transmitter
 *  software buffer.
 *
 *  \param usart_data The USART_data_t struct instance.
 *
 *  \retval true      There is data in the receive buffer.
 *  \retval false     The receive buffer is empty.
 */
bool USART_TXBuffer_FreeSpace( USART_data_t * usart_data)
{
	/* Make copies to make sure that volatile access is specified. */
	uint8_t tempHead = (usart_data->TX_Head + 1) & USART_TX_BUFFER_MASK;
	uint8_t tempTail = usart_data->TX_Tail;

	/* There are data left in the buffer unless Head and Tail are equal. */
	return (tempHead != tempTail);
}



/*! \brief Put data (5-8 bit character).
 *
 *  Stores data byte in TX software buffer and enables DRE interrupt if there
 *  is free space in the TX software buffer.
 *
 *  \param usart_data The USART_data_t struct instance.
 *  \param data       The data to send.
 */
bool USART_TXBuffer_PutByte( USART_data_t * usart_data, uint8_t data)
{

	USART_data_t * const ad = usart_data;


	uint8_t tempCTRLA;
	uint8_t tempTX_Head;
	bool TXBuffer_FreeSpace;
	//USART_Buffer_t * TXbufPtr;

	//TXbufPtr = &usart_data->buffer;
	
	
	TXBuffer_FreeSpace = USART_TXBuffer_FreeSpace(ad);


	if(TXBuffer_FreeSpace)
	{
	  	tempTX_Head = ad->TX_Head;
	  	ad->TX[tempTX_Head]= data;
		/* Advance buffer head. */
		ad->TX_Head = (tempTX_Head + 1) & USART_TX_BUFFER_MASK;

		/* Enable DRE interrupt. */
		tempCTRLA = ad->usart->CTRLA;
		tempCTRLA = (tempCTRLA & ~USART_DREINTLVL_gm) | ad->dreIntLevel;
		ad->usart->CTRLA = tempCTRLA;
	}
	return TXBuffer_FreeSpace;
}






/*! \brief Get received data (5-8 bit character).
 *
 *  The function USART_RXBufferData_Available should be used before this
 *  function is used to ensure that data is available.
 *
 *  Returns data from RX software buffer.
 *
 *  \param usart_data       The USART_data_t struct instance.
 *
 *  \return         Received data.
 */
uint8_t USART_RXBuffer_GetByte(USART_data_t * ud)
{
	//USART_Buffer_t * const bufPtr = &usart_data->buffer;
	const uint8_t ans = (ud->RX[ud->RX_Tail]);

	/* Advance buffer tail. */
	ud->RX_Tail = (ud->RX_Tail + 1) & USART_RX_BUFFER_MASK;
	return ans;
}



/*! \brief RX Complete Interrupt Service Routine.
 *
 *  RX Complete Interrupt Service Routine.
 *  Stores received data in RX software buffer.
 *
 *  \param usart_data      The USART_data_t struct instance.
 */
bool USART_RXComplete( USART_data_t * usart_data)
{
	USART_data_t * const ad = usart_data;
	
	//USART_Buffer_t * const bufPtr = &usart_data->buffer;
	/* Advance buffer head. */
	uint8_t tempRX_Head = (ad->RX_Head + 1) & USART_RX_BUFFER_MASK;

	/* Check for overflow. */
	uint8_t tempRX_Tail = ad->RX_Tail;
	uint8_t data = ad->usart->DATA;

	if (tempRX_Head == tempRX_Tail) {
	  	return false;
	}else{
		if(g_log_verbosity>=VERY_VERBOSE) usart_putchar(USART_DEBUG,data);
		ad->RX[ad->RX_Head] = data;
		ad->RX_Head = tempRX_Head;
	}
	return true;
}



/*! \brief Data Register Empty Interrupt Service Routine.
 *
 *  Data Register Empty Interrupt Service Routine.
 *  Transmits one byte from TX software buffer. Disables DRE interrupt if buffer
 *  is empty. Argument is pointer to USART (USART_data_t).
 *
 *  \param usart_data      The USART_data_t struct instance.
 */
void USART_DataRegEmpty( USART_data_t * usart_data)
{
	USART_data_t * const ad = usart_data;

	//USART_Buffer_t * const bufPtr = &usart_data->buffer;

	/* Check if all data is transmitted. */
	const uint8_t tempTX_Tail = ad->TX_Tail;
	if (ad->TX_Head == tempTX_Tail){
	    /* Disable DRE interrupts. */
		uint8_t tempCTRLA = ad->usart->CTRLA;
		tempCTRLA = (tempCTRLA & ~USART_DREINTLVL_gm) | USART_DREINTLVL_OFF_gc;
		ad->usart->CTRLA = tempCTRLA;

	}else{
		/* Start transmitting. */
		const uint8_t data = ad->TX[ad->TX_Tail];
		ad->usart->DATA = data;

		/* Advance buffer tail. */
		ad->TX_Tail = (ad->TX_Tail + 1) & USART_TX_BUFFER_MASK;
	}
}
