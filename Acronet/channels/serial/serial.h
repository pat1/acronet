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


#ifndef SERIAL_CHANNEL_H_
#define SERIAL_CHANNEL_H_



#define SERCHAN_PREP(p1,p2,...)			p1##p2(__VA_ARGS__)

#define SERCHAN_ENABLE_RX(ch)				SERCHAN_PREP( serchan_enable_RX_CH , ch )
#define SERCHAN_DISABLE_RX(ch)				SERCHAN_PREP( serchan_disable_RX_CH , ch )
#define SERCHAN_ENABLE_TX(ch)				SERCHAN_PREP( serchan_enable_TX_CH , ch )
#define SERCHAN_DISABLE_TX(ch)				SERCHAN_PREP( serchan_disable_TX_CH , ch )
#define SERCHAN_RESET(ch)					SERCHAN_PREP( serchan_reset_CH , ch )
#define SERCHAN_GETCHAR(ch)					SERCHAN_PREP( serchan_getChar_CH	, ch )
#define SERCHAN_PUTSTRING(ch,psz)			SERCHAN_PREP( serchan_putStr_CH , ch )


void serchan_enable_RX_CH0(void);
void serchan_enable_RX_CH1(void);
void serchan_enable_RX_CH2(void);
void serchan_enable_RX_CH3(void);

void serchan_disable_RX_CH0(void);
void serchan_disable_RX_CH1(void);
void serchan_disable_RX_CH2(void);
void serchan_disable_RX_CH3(void);

void serchan_enable_TX_CH0(void);
void serchan_enable_TX_CH1(void);
void serchan_enable_TX_CH2(void);
void serchan_enable_TX_CH3(void);

void serchan_disable_TX_CH0(void);
void serchan_disable_TX_CH1(void);
void serchan_disable_TX_CH2(void);
void serchan_disable_TX_CH3(void);

void serchan_reset_CH0(void);
void serchan_reset_CH1(void);
void serchan_reset_CH2(void);
void serchan_reset_CH3(void);

char serchan_getChar_CH0(void);
char serchan_getChar_CH1(void);
char serchan_getChar_CH2(void);
char serchan_getChar_CH3(void);

void serchan_putStr_CH0(const char * const psz,const uint16_t maxlen);
void serchan_putStr_CH1(const char * const psz,const uint16_t maxlen);
void serchan_putStr_CH2(const char * const psz,const uint16_t maxlen);
void serchan_putStr_CH3(const char * const psz,const uint16_t maxlen);


#endif /* SERIAL_CHANNEL_H_ */