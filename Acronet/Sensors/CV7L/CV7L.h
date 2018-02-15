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


#ifndef CV7L_H_
#define CV7L_H_

/*
#define CV7L_PUSART SP336_USART1
#define CV7L_PIN_TX_ENABLE SP336_USART1_PIN_TX_ENABLE
#define CV7L_PIN_TX_SIGNAL	SP336_USART1_PIN_TX_SIGNAL
#define CV7L_PIN_RX_SIGNAL SP336_USART1_PIN_RX_SIGNAL
*/

//#define CV7L_PUSART SP336_USART1
//#define CV7L_PIN_TX_ENABLE SP336_USART1_PIN_TX_ENABLE
//#define CV7L_PIN_TX_SIGNAL	SP336_USART1_PIN_TX_SIGNAL
//#define CV7L_PIN_RX_SIGNAL SP336_USART1_PIN_RX_SIGNAL

enum {	CV7L_STAT_BEG=0,
		CV7L_STAT_TEMPERATURE = CV7L_STAT_BEG,
		CV7L_STAT_TEMPERATURE_MAX,
		CV7L_STAT_TEMPERATURE_MIN,
		CV7L_STAT_WINDIR,
		CV7L_STAT_WINSPEED,
		CV7L_STAT_WINSPEED_GUST,			
		CV7L_STAT_END};


typedef struct
{
	float		m_stat[CV7L_STAT_END];
	uint16_t	m_samples;
} CV7L_DATA;


RET_ERROR_CODE CV7L_init(void);


RET_ERROR_CODE CV7L_get_data(CV7L_DATA * const ps);
RET_ERROR_CODE CV7L_reset_data(void);
RET_ERROR_CODE CV7L_Data2String(const CV7L_DATA * const st,char * const sz, uint16_t * len_sz);
void CV7L_enable(void);
void CV7L_disable(void);

bool CV7L_Yield(void);

#endif /* CV7L_H_ */