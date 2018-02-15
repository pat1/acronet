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


#ifndef L8095N_H_
#define L8095N_H_

//#define L8095N_PUSART			SP336_USART1
//#define L8095N_PIN_TX_ENABLE	SP336_USART1_PIN_TX_ENABLE
//#define L8095N_PIN_TX_SIGNAL	SP336_USART1_PIN_TX_SIGNAL
//#define L8095N_PIN_RX_SIGNAL	SP336_USART1_PIN_RX_SIGNAL

//#define L8095N_PUSART			SP336_USART0
//#define L8095N_PIN_TX_ENABLE	SP336_USART0_PIN_TX_ENABLE
//#define L8095N_PIN_TX_SIGNAL	SP336_USART0_PIN_TX_SIGNAL
//#define L8095N_PIN_RX_SIGNAL	SP336_USART0_PIN_RX_SIGNAL

enum {		L8095N_STAT_BEG=0,
			L8095N_STAT_PRESSURE=L8095N_STAT_BEG,
			L8095N_STAT_TEMPERATURE,
			L8095N_STAT_TEMPERATURE_MAX,
			L8095N_STAT_TEMPERATURE_MIN,
			L8095N_STAT_RH,
			L8095N_STAT_DEWPOINT,
			L8095N_STAT_END};


typedef struct
{
	float		m_stat[L8095N_STAT_END];
	uint16_t	m_samples;
} L8095N_DATA;


RET_ERROR_CODE l8095n_init(void);


RET_ERROR_CODE l8095n_get_data(L8095N_DATA * const ps);
RET_ERROR_CODE l8095n_reset_data(void);
RET_ERROR_CODE l8095n_Data2String(	 const L8095N_DATA * const st
									,char * const sz
									,uint16_t * len_sz);

#ifdef RMAP_SERVICES
RET_ERROR_CODE l8095n_Data2String_RMAP(	 uint8_t * const subModule
										,const L8095N_DATA * const st
										,const uint32_t timeStamp
										,const uint16_t timeWindow
										,char * const szTopic
										,int16_t * const len_szTopic
										,char * const szMessage
										,int16_t * const len_szMessage );
#endif

void l8095n_enable(void);
void l8095n_disable(void);

bool l8095n_Yield(void);

#endif /* L8095N_H_ */