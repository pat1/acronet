/*
 * nmea.h
 *
 * Created: 20/04/2018 15:47:32
 *  Author: fabio
 */ 


#ifndef NMEA_H_
#define NMEA_H_

uint8_t NMEALine_Tokenize(char * psz,char ** pNext);


#define NMEA_PREP(p1,p2,...)			p1##p2(__VA_ARGS__)

#define NMEA_LINE_ENABLE_RX(ch)					NMEA_PREP( NMEA_Line_enable_RX_CH , ch )
#define NMEA_LINE_DISABLE_RX(ch)				NMEA_PREP( NMEA_Line_disable_RX_CH , ch )
#define NMEA_LINE_ENABLE_TX(ch)					NMEA_PREP( NMEA_Line_enable_TX_CH , ch )
#define NMEA_LINE_DISABLE_TX(ch)				NMEA_PREP( NMEA_Line_disable_TX_CH , ch )
#define NMEA_LINE_RESET(ch)						NMEA_PREP( NMEA_Line_reset_CH , ch )
#define NMEA_LINE_GETCHAR(ch)					NMEA_PREP( NMEA_Line_getChar_CH	, ch )
#define NMEA_LINE_PUTSTRING(ch,psz)				NMEA_PREP( NMEA_Line_putStr_CH , ch )

#define NMEA_LINE_CHECKSUM_CHECK(ch,psz,len_sz)	NMEA_PREP( NMEA_Line_checksum_check_CH , ch , psz , len_sz )

void NMEA_Line_enable_RX_CH0(void);
void NMEA_Line_enable_RX_CH1(void);
void NMEA_Line_enable_RX_CH2(void);
void NMEA_Line_enable_RX_CH3(void);

void NMEA_Line_disable_RX_CH0(void);
void NMEA_Line_disable_RX_CH1(void);
void NMEA_Line_disable_RX_CH2(void);
void NMEA_Line_disable_RX_CH3(void);

void NMEA_Line_enable_TX_CH0(void);
void NMEA_Line_enable_TX_CH1(void);
void NMEA_Line_enable_TX_CH2(void);
void NMEA_Line_enable_TX_CH3(void);

void NMEA_Line_disable_TX_CH0(void);
void NMEA_Line_disable_TX_CH1(void);
void NMEA_Line_disable_TX_CH2(void);
void NMEA_Line_disable_TX_CH3(void);

void NMEA_Line_reset_CH0(void);
void NMEA_Line_reset_CH1(void);
void NMEA_Line_reset_CH2(void);
void NMEA_Line_reset_CH3(void);

char NMEA_Line_getChar_CH0(void);
char NMEA_Line_getChar_CH1(void);
char NMEA_Line_getChar_CH2(void);
char NMEA_Line_getChar_CH3(void);

void NMEA_Line_putStr_CH0(const char * const psz,const uint16_t maxlen);
void NMEA_Line_putStr_CH1(const char * const psz,const uint16_t maxlen);
void NMEA_Line_putStr_CH2(const char * const psz,const uint16_t maxlen);
void NMEA_Line_putStr_CH3(const char * const psz,const uint16_t maxlen);

uint8_t NMEA_Line_checksum_check_CH0(char * const psz,const uint8_t len_sz);
uint8_t NMEA_Line_checksum_check_CH1(char * const psz,const uint8_t len_sz);
uint8_t NMEA_Line_checksum_check_CH2(char * const psz,const uint8_t len_sz);
uint8_t NMEA_Line_checksum_check_CH3(char * const psz,const uint8_t len_sz);

#endif /* NMEA_H_ */