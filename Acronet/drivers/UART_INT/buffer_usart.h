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

#ifndef BUFFER_USART_H_
#define BUFFER_USART_H_

#define USART_RX_BUFFER_SIZE 16
#define USART_TX_BUFFER_SIZE 2
#define USART_RX_BUFFER_MASK ( USART_RX_BUFFER_SIZE - 1 )
#define USART_TX_BUFFER_MASK ( USART_TX_BUFFER_SIZE - 1 )



typedef struct Usart_and_buffer
{
	uint8_t RX[USART_RX_BUFFER_SIZE];
	uint8_t TX[USART_TX_BUFFER_SIZE];

	uint8_t RX_Head;
	uint8_t RX_Tail;

	uint8_t TX_Head;
	uint8_t TX_Tail;

	volatile USART_t * usart;
	USART_DREINTLVL_t dreIntLevel;
	
} USART_data_t;



/* Functions for interrupt driven driver. */
void usart_interruptdriver_initialize(USART_data_t * const usart_data,USART_t * const usart,USART_DREINTLVL_t dreIntLevel );


bool USART_TXBuffer_FreeSpace( USART_data_t * const usart_data);
bool USART_TXBuffer_PutByte( USART_data_t * const usart_data, uint8_t data);
uint8_t USART_RXBuffer_GetByte(USART_data_t * const usart_data);
bool USART_RXComplete(USART_data_t * const usart_data);
void USART_DataRegEmpty(USART_data_t * const usart_data);

/*! \brief Test if there is data in the receive software buffer.
 *
 *  This function can be used to test if there is data in the receive software
 *  buffer.
 *
 *  \param usart_data         The USART_data_t struct instance
 *
 *  \retval true      There is data in the receive buffer.
 *  \retval false     The receive buffer is empty.
 */
static inline bool USART_RXBufferData_Available(const USART_data_t * const usart_data)
{
	/* Make copies to make sure that volatile access is specified. */
	const uint8_t tempHead = usart_data->RX_Head;
	const uint8_t tempTail = usart_data->RX_Tail;

	/* There are data left in the buffer unless Head and Tail are equal. */
	return (tempHead != tempTail);
}

static inline void usart_buffer_flush(USART_data_t * const usart_data)
{
	usart_data->RX_Tail = 0;
	usart_data->RX_Head = 0;
	usart_data->TX_Tail = 0;
	usart_data->TX_Head = 0;
}


#endif /* BUFFER_USART_H_ */