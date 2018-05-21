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
#include "Acronet/drivers/SP336/SP336.h"
#include "Acronet/channels/NMEA/nmea.h"

#define NMEA_SENTENCE_MAX_LENGTH 90


#ifdef MOD_DIG_0
#undef MOD_DIG_0
#endif
#ifdef MOD_DIG_1
#undef MOD_DIG_1
#endif
#ifdef MOD_DIG_2
#undef MOD_DIG_2
#endif
#ifdef MOD_DIG_3
#undef MOD_DIG_3
#endif

#ifdef USES_NMEA_CHAN_0
#define MOD_DIG_0 1
#define NMEA_CHAN_0_IDX 0
#pragma message "NMEA CH#0 is ON"
#else
#pragma message "NMEA CH#0 is OFF"
#define MOD_DIG_0 0
#endif

#ifdef USES_NMEA_CHAN_1
#define MOD_DIG_1 1
#define NMEA_CHAN_1_IDX (0 + MOD_DIG_0)
#pragma message "NMEA CH#1 is ON"
#else
#pragma message "NMEA CH#1 is OFF"
#define MOD_DIG_1 0
#endif

#ifdef USES_NMEA_CHAN_2
#define MOD_DIG_2 1
#define NMEA_CHAN_2_IDX (0 + MOD_DIG_0 + MOD_DIG_1)
#pragma message "NMEA CH#2 is ON"
#else
#pragma message "NMEA CH#2 is OFF"
#define MOD_DIG_2 0
#endif

#ifdef USES_NMEA_CHAN_3
#define MOD_DIG_3 1
#define NMEA_CHAN_3_IDX (0 + MOD_DIG_0 + MOD_DIG_1 + MOD_DIG_2)
#pragma message "NMEA CH#3 is ON"
#else
#pragma message "NMEA CH#3 is OFF"
#define MOD_DIG_3 0
#endif


#define NMEA_CHANNELS (0 + MOD_DIG_0 + MOD_DIG_1 + MOD_DIG_2 + MOD_DIG_3)
#define NMEA_UART_BUF_SIZE 256

#if (NMEA_CHANNELS == 0)
#pragma message "NMEA CHANNELS 0"
#elif (NMEA_CHANNELS == 1)
#pragma message "NMEA CHANNELS 1"
#elif (NMEA_CHANNELS == 2)
#pragma message "NMEA CHANNELS 2"
#elif (NMEA_CHANNELS == 3)
#pragma message "NMEA CHANNELS 3"
#endif


static volatile char g_szNMEALine[NMEA_CHANNELS][NMEA_UART_BUF_SIZE];
static volatile uint16_t g_idxBufferNMEALine[NMEA_CHANNELS];
static volatile uint16_t g_idxProcessNMEALine[NMEA_CHANNELS];


static void NMEALine_reset(const uint8_t ch)
{
	g_szNMEALine[ch][0] = 0;
	g_idxBufferNMEALine[ch] = 0;
	g_idxProcessNMEALine[ch] = 0;
}

static char NMEALine_getChar(const uint8_t ch)
{
	
	//TODO: add critical section here ?
	
	if(g_idxBufferNMEALine[ch] == 0) return 0;
	const char c = g_szNMEALine[ch][g_idxProcessNMEALine[ch]++];
	if(g_idxProcessNMEALine[ch]==g_idxBufferNMEALine[ch]) {
		NMEALine_reset(ch);
	}

	return c;
}


static void NMEALine_addChar(const uint8_t ch,const char c)
{
	if(g_idxBufferNMEALine[ch]<NMEA_UART_BUF_SIZE) {
		g_szNMEALine[ch][g_idxBufferNMEALine[ch]++]=c;
	}
}

uint8_t NMEALine_Tokenize(char * psz,char ** pNext)
{
	uint8_t i = 0;
	char * const p = psz;
	
	do {
		const char c = p[i];
		if(c==',') {
			p[i] = 0;
			*pNext = p+(i+1);
			return i;
		}
		
		if(c==0) {
			*pNext = NULL;
			return i;
		}
	} while(++i<NMEA_SENTENCE_MAX_LENGTH);
	
	*pNext = NULL;
	return 0;
}

static uint8_t __attribute__((const)) ascii_hex(const uint8_t c)
{
	if((c<58) && (c>47)) { // 0 to 9
		return (c-48);
	} else if((c<71) && (c>64)) { // A to F
		return (c-55);
	} else if((c<103) && (c>96)) { // a to f
		return (c-87);
	}
	
	//ERROR TO HANDLE
	
	return 0xFF;
}


uint8_t NMEA_Line_checksum_check(char * const psz,const uint8_t len_sz)
{
	uint8_t ix = 0;
	uint8_t r = 0;
	if(len_sz<10) {
		return 1;
	}
	if(psz[ix]=='$') ix++;
	while (ix<len_sz)
	{
		const char c = psz[ix++];
		if(c=='*') break;
		r ^= c;
	}
	
	uint8_t cs = (ascii_hex(psz[ix])<<4) | ascii_hex(psz[ix+1]);
	if (r!=cs)
	{
		debug_string_2P(NORMAL,PSTR("NMEA"),PSTR("CHECKSUM MISMATCH"));
		return 0xFF;
	}
	return 0;
}

/*
#define CAT2(A,B) A ## B
#define CAT3(A,B,C) A ## B ## C
#define NMEA_CHAN_IDX_BUILD(idx) CAT3(NMEA_CHAN_ , idx , _IDX)

#define IFACE_TEMPLATE(CHAN_IDX)	void CAT2( NMEA_Line_reset_CH , CHAN_IDX ) (void) \
									{\
										g_szNMEALine[ NMEA_CHAN_IDX_BUILD(CHAN_IDX) ][0] = 0;\
										g_idxBufferNMEALine[ NMEA_CHAN_IDX_BUILD(CHAN_IDX) ] = 0;\
										g_idxProcessNMEALine[ NMEA_CHAN_IDX_BUILD(CHAN_IDX) ] = 0;\
									}\
									char CAT2(NMEA_Line_getChar_CH , CHAN_IDX) (void) \
									{\
										if(g_idxBufferNMEALine[ NMEA_CHAN_IDX_BUILD(CHAN_IDX) ] == 0) return 0;\
										const char c = g_szNMEALine[ NMEA_CHAN_IDX_BUILD(CHAN_IDX) ][g_idxProcessNMEALine[ NMEA_CHAN_IDX_BUILD(CHAN_IDX) ]++];\
										if(g_idxProcessNMEALine[ NMEA_CHAN_IDX_BUILD(CHAN_IDX) ]==g_idxBufferNMEALine[ NMEA_CHAN_IDX_BUILD(CHAN_IDX) ]) {\
											CAT2(NMEA_Line_reset_CH,CHAN_IDX)();\
										}\
										return c;\
									}\
									void CAT2(NMEA_Line_addChar_CH, CHAN_IDX) (const char c)\
									{\
										if(g_idxBufferNMEALine[ NMEA_CHAN_IDX_BUILD(CHAN_IDX) ] < NMEA_UART_BUF_SIZE) {\
											g_szNMEALine[ NMEA_CHAN_IDX_BUILD(CHAN_IDX) ][g_idxBufferNMEALine[ NMEA_CHAN_IDX_BUILD(CHAN_IDX) ]++]=c;\
										}\
									}
									
*/



#ifdef USES_NMEA_CHAN_0
#undef NMEA_CHAN_0_PUT
#undef NMEA_CHAN_0_ISR
#undef NMEA_CHAN_0_USART
#define NMEA_CHAN_0_PUT SP336_0_PutString
#define NMEA_CHAN_0_ISR  SP336_USART0_RX_Vect
#define NMEA_CHAN_0_USART SP336_USART0
#endif //MODBUS_CHAN_0

#ifdef USES_NMEA_CHAN_1
#undef NMEA_CHAN_1_PUT
#undef NMEA_CHAN_1_ISR
#undef NMEA_CHAN_1_USART
#define NMEA_CHAN_1_PUT SP336_1_PutBuffer
#define NMEA_CHAN_1_ISR  SP336_USART1_RX_Vect
#define NMEA_CHAN_1_USART SP336_USART1
#endif //NMEA_CHAN_1

#ifdef USES_NMEA_CHAN_2
#undef NMEA_CHAN_2_PUT
#undef NMEA_CHAN_2_ISR
#undef NMEA_CHAN_2_USART
#define NMEA_CHAN_2_PUT SP336_2_PutBuffer
#define NMEA_CHAN_2_ISR  SP336_USART2_RX_Vect
#define NMEA_CHAN_2_USART SP336_USART2
#endif //NMEA_CHAN_2


#ifdef USES_NMEA_CHAN_3
#undef NMEA_CHAN_3_PUT
#undef NMEA_CHAN_3_ISR
#undef NMEA_CHAN_3_USART
#define NMEA_CHAN_3_PUT SP336_3_PutBuffer
#define NMEA_CHAN_3_ISR  SP336_USART3_RX_Vect
#define NMEA_CHAN_3_USART SP336_USART3
#endif //NMEA_CHAN_3


#ifdef USES_NMEA_CHAN_0


void NMEA_Line_reset_CH0(void)
{
	g_szNMEALine[NMEA_CHAN_0_IDX][0] = 0;
	g_idxBufferNMEALine[NMEA_CHAN_0_IDX] = 0;
	g_idxProcessNMEALine[NMEA_CHAN_0_IDX] = 0;
}

char NMEA_Line_getChar_CH0(void)
{
	
	//TODO: add critical section here ?
	
	if(g_idxBufferNMEALine[NMEA_CHAN_0_IDX] == 0) return 0;
	const char c = g_szNMEALine[NMEA_CHAN_0_IDX][g_idxProcessNMEALine[NMEA_CHAN_0_IDX]++];
	if(g_idxProcessNMEALine[NMEA_CHAN_0_IDX]==g_idxBufferNMEALine[NMEA_CHAN_0_IDX]) {
		NMEA_Line_reset_CH0();
	}

	return c;
}


static void NMEA_Line_addChar_CH0(const char c)
{
	if(g_idxBufferNMEALine[NMEA_CHAN_0_IDX]<NMEA_UART_BUF_SIZE) {
		g_szNMEALine[NMEA_CHAN_0_IDX][g_idxBufferNMEALine[NMEA_CHAN_0_IDX]++]=c;
	}
}

void NMEA_Line_enable_RX_CH0(void)
{
	usart_rx_enable(&NMEA_CHAN_0_USART);
}

void NMEA_Line_disable_RX_CH0(void)
{
	usart_rx_disable(&NMEA_CHAN_0_USART);
}

void NMEA_Line_enable_TX_CH0(void)
{
	usart_tx_enable(&NMEA_CHAN_0_USART);
}

void NMEA_Line_disable_TX_CH0(void)
{
	usart_tx_disable(&NMEA_CHAN_0_USART);
}

void NMEA_Line_putStr_CH0(const char * const psz,const uint16_t maxlen)
{
	NMEA_CHAN_0_PUT(psz,maxlen);
}

ISR(NMEA_CHAN_0_ISR)
{
	//usart_putchar(USART_DEBUG,'.');
	NMEA_Line_addChar_CH0(NMEA_CHAN_0_USART.DATA);
}

#endif //USES_NMEA_CHAN_0

#ifdef USES_NMEA_CHAN_1

void NMEA_Line_reset_CH1(void)
{
	g_szNMEALine[NMEA_CHAN_1_IDX][0] = 0;
	g_idxBufferNMEALine[NMEA_CHAN_1_IDX] = 0;
	g_idxProcessNMEALine[NMEA_CHAN_1_IDX] = 0;
}

char NMEA_Line_getChar_CH1(void)
{
	
	//TODO: add critical section here ?
	
	if(g_idxBufferNMEALine[NMEA_CHAN_1_IDX] == 0) return 0;
	const char c = g_szNMEALine[NMEA_CHAN_1_IDX][g_idxProcessNMEALine[NMEA_CHAN_1_IDX]++];
	if(g_idxProcessNMEALine[NMEA_CHAN_1_IDX]==g_idxBufferNMEALine[NMEA_CHAN_1_IDX]) {
		NMEA_Line_reset_CH1();
	}

	return c;
}


static void NMEA_Line_addChar_CH1(const char c)
{
	if(g_idxBufferNMEALine[NMEA_CHAN_1_IDX]<NMEA_UART_BUF_SIZE) {
		g_szNMEALine[NMEA_CHAN_1_IDX][g_idxBufferNMEALine[NMEA_CHAN_1_IDX]++]=c;
	}
}

void NMEA_Line_enable_RX_CH1(void)
{
	usart_rx_enable(&NMEA_CHAN_1_USART);
}

void NMEA_Line_disable_RX_CH1(void)
{
	usart_rx_disable(&NMEA_CHAN_1_USART);
}

void NMEA_Line_enable_TX_CH1(void)
{
	usart_tx_enable(&NMEA_CHAN_1_USART);
}

void NMEA_Line_disable_TX_CH1(void)
{
	usart_tx_disable(&NMEA_CHAN_1_USART);
}

void NMEA_Line_putStr_CH1(const char * const psz,const uint16_t maxlen)
{
	NMEA_CHAN_1_PUT(psz,maxlen);
}


ISR(NMEA_CHAN_1_ISR)
{
	//usart_putchar(USART_DEBUG,'.');
	NMEA_Line_addChar_CH1(NMEA_CHAN_1_USART.DATA);
}

#endif

#ifdef USES_NMEA_CHAN_2
void NMEA_Line_reset_CH2(void)
{
	g_szNMEALine[NMEA_CHAN_2_IDX][0] = 0;
	g_idxBufferNMEALine[NMEA_CHAN_2_IDX] = 0;
	g_idxProcessNMEALine[NMEA_CHAN_2_IDX] = 0;
}

char NMEA_Line_getChar_CH2(void)
{
	
	//TODO: add critical section here ?
	
	if(g_idxBufferNMEALine[NMEA_CHAN_2_IDX] == 0) return 0;
	const char c = g_szNMEALine[NMEA_CHAN_2_IDX][g_idxProcessNMEALine[NMEA_CHAN_2_IDX]++];
	if(g_idxProcessNMEALine[NMEA_CHAN_2_IDX]==g_idxBufferNMEALine[NMEA_CHAN_2_IDX]) {
		NMEA_Line_reset_CH2();
	}

	return c;
}


static void NMEA_Line_addChar_CH2(const char c)
{
	if(g_idxBufferNMEALine[NMEA_CHAN_2_IDX]<NMEA_UART_BUF_SIZE) {
		g_szNMEALine[NMEA_CHAN_2_IDX][g_idxBufferNMEALine[NMEA_CHAN_2_IDX]++]=c;
	}
}

void NMEA_Line_enable_RX_CH2(void)
{
	usart_rx_enable(&NMEA_CHAN_2_USART);
}

void NMEA_Line_disable_RX_CH2(void)
{
	usart_rx_disable(&NMEA_CHAN_2_USART);
}

void NMEA_Line_enable_TX_CH2(void)
{
	usart_tx_enable(&NMEA_CHAN_2_USART);
}

void NMEA_Line_disable_TX_CH2(void)
{
	usart_tx_disable(&NMEA_CHAN_2_USART);
}

void NMEA_Line_putStr_CH2(const char * const psz,const uint16_t maxlen)
{
	NMEA_CHAN_2_PUT(psz,maxlen);
}


ISR(NMEA_CHAN_2_ISR)
{
	//usart_putchar(USART_DEBUG,'.');
	NMEA_Line_addChar_CH2(NMEA_CHAN_2_USART.DATA);
}

#endif

#ifdef USES_NMEA_CHAN_3
void NMEA_Line_reset_CH3(void)
{
	g_szNMEALine[NMEA_CHAN_3_IDX][0] = 0;
	g_idxBufferNMEALine[NMEA_CHAN_3_IDX] = 0;
	g_idxProcessNMEALine[NMEA_CHAN_3_IDX] = 0;
}

char NMEA_Line_getChar_CH3(void)
{
	
	//TODO: add critical section here ?
	
	if(g_idxBufferNMEALine[NMEA_CHAN_3_IDX] == 0) return 0;
	const char c = g_szNMEALine[NMEA_CHAN_3_IDX][g_idxProcessNMEALine[NMEA_CHAN_3_IDX]++];
	if(g_idxProcessNMEALine[NMEA_CHAN_3_IDX]==g_idxBufferNMEALine[NMEA_CHAN_3_IDX]) {
		NMEA_Line_reset_CH3();
	}

	return c;
}


static void NMEA_Line_addChar_CH3(const char c)
{
	if(g_idxBufferNMEALine[NMEA_CHAN_3_IDX]<NMEA_UART_BUF_SIZE) {
		g_szNMEALine[NMEA_CHAN_3_IDX][g_idxBufferNMEALine[NMEA_CHAN_3_IDX]++]=c;
	}
}

void NMEA_Line_enable_RX_CH3(void)
{
	usart_rx_enable(&NMEA_CHAN_3_USART);
}

void NMEA_Line_disable_RX_CH3(void)
{
	usart_rx_disable(&NMEA_CHAN_3_USART);
}

void NMEA_Line_enable_TX_CH3(void)
{
	usart_tx_enable(&NMEA_CHAN_3_USART);
}

void NMEA_Line_disable_TX_CH3(void)
{
	usart_tx_disable(&NMEA_CHAN_3_USART);
}

void NMEA_Line_putStr_CH3(const char * const psz,const uint16_t maxlen)
{
	NMEA_CHAN_3_PUT(psz,maxlen);
}


ISR(NMEA_CHAN_3_ISR)
{
	//usart_putchar(USART_DEBUG,'.');
	NMEA_Line_addChar_CH3(NMEA_CHAN_3_USART.DATA);
}

#endif
