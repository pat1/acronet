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


#ifndef ZEUS_H_
#define ZEUS_H_


//#define USES_NMEA_CHAN_0
#define USES_NMEA_CHAN_1
#define USES_MODBUS_CHAN_2
//#define USES_MODBUS_CHAN_3
//#define USES_PULSE_CHAN_0

#define MODULE_DECLARATION	((MOD_ID_CV7L,(NMEA_CHAN_1)))\
							((MOD_ID_T026_MODBUS,(MODBUS_CHAN_2)))\
							((MOD_ID_T056_MODBUS,(MODBUS_CHAN_2)))

#define HD3910_PER_ISTANCE_CMD ((0x07,0x04,0x00,0x04,0x00,0x02,0x30,0x6C))((0x07,0x04,0x00,0x04,0x00,0x02,0x30,0x6C))

#define T056_PER_ISTANCE_CMD ((0x07,0x04,0x00,0x04,0x00,0x02,0x30,0x6C))
#define T023_PER_ISTANCE_CMD ((0x15,0x04,0x00,0x00,0x00,0x04,0xF2,0xDD))
#define T026_PER_ISTANCE_CMD ((0x03,0x04,0x00,0x00,0x00,0x04,0xF0,0x2B))

#define SP336_MODE					SP336_MODE_RS485_FULLDUP
#define SP336_2_MODE				SP336_2_MODE_RS485_HALFDUP

//#define SP336_USART0_BPS	19200
#define SP336_USART1_BPS	4800
#define SP336_USART2_BPS	9600
//#define SP336_USART3_BPS	9600



#define PM10QBIT_PIN_RX_SIGNAL		SP336_USART0_PIN_RX_SIGNAL

#define VP61_VOLTMETER				ADCA
#define VP61_VOLTMETER_CH			ADC_CH0
#define VP61_VOLTMETER_PIN_POS		ADCCH_POS_PIN4
#define VP61_VOLTMETER_PIN_NEG		ADCCH_POS_PIN0



#endif /* ZEUS_H_ */