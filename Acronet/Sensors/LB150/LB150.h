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


#ifndef LB150_H_
#define LB150_H_

/*
#define LB150_PUSART SP336_USART1
#define LB150_PIN_TX_ENABLE SP336_USART1_PIN_TX_ENABLE
#define LB150_PIN_TX_SIGNAL	SP336_USART1_PIN_TX_SIGNAL
#define LB150_PIN_RX_SIGNAL SP336_USART1_PIN_RX_SIGNAL
*/

//#define LB150_PUSART SP336_USART0
//#define LB150_PIN_TX_ENABLE SP336_USART0_PIN_TX_ENABLE
//#define LB150_PIN_TX_SIGNAL	SP336_USART0_PIN_TX_SIGNAL
//#define LB150_PIN_RX_SIGNAL SP336_USART0_PIN_RX_SIGNAL

enum {	LB150_STAT_BEG=0,
		LB150_STAT_PRESSURE=LB150_STAT_BEG,
		LB150_STAT_TEMPERATURE,
		LB150_STAT_TEMPERATURE_MAX,
		LB150_STAT_TEMPERATURE_MIN,
		LB150_STAT_RH,
		LB150_STAT_DEWPOINT,
		LB150_STAT_WINDIR,
		LB150_STAT_WINDIR_GUST,
		LB150_STAT_WINSPEED,
		LB150_STAT_WINSPEED_GUST,
#ifdef LB150_ENABLE_GPS
		LB150_STAT_LATITUDE,
		LB150_STAT_LONGITUDE,
#endif	//	LB150_ENABLE_GPS
		LB150_STAT_END};


typedef struct
{
	float		m_stat[LB150_STAT_END];
	uint16_t	m_samples;
} LB150_DATA;


RET_ERROR_CODE LB150_init(void);


RET_ERROR_CODE LB150_get_data(LB150_DATA * const ps);
RET_ERROR_CODE LB150_reset_data(void);
RET_ERROR_CODE LB150_Data2String(const LB150_DATA * const st,char * const sz, int16_t * len_sz);
void LB150_enable(void);
void LB150_disable(void);

bool LB150_Yield(void);

#endif /* LB150_H_ */