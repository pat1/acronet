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


#ifndef MOGGIE_H_
#define MOGGIE_H_


//#define USES_MODBUS_CHAN_0
//#define USES_MODBUS_CHAN_1
#define USES_MODBUS_CHAN_3
//#define USES_MODBUS_CHAN_3
//#define USES_PULSE_CHAN_0

#define MODULE_DECLARATION	((MOD_ID_HD3910_MODBUS,(MODBUS_CHAN_3)(MODBUS_CHAN_3)(MODBUS_CHAN_3)))

#define T056_PER_ISTANCE_CMD ((0x07,0x04,0x00,0x04,0x00,0x02,0x30,0x6C))((0x07,0x04,0x00,0x04,0x00,0x02,0x30,0x6C))
#define HD3910_PER_ISTANCE_CMD ((0x1B,0x04,0x00,0x00,0x00,0x04,0xF3,0xF3))((0x1C,0x04,0x00,0x00,0x00,0x04,0xF2,0x44))((0x1D,0x04,0x00,0x00,0x00,0x04,0xF3,0x95))

#define SP336_MODE					SP336_MODE_HIGHZ
#define SP336_2_MODE				SP336_2_MODE_RS485_HALFDUP

//#define SP336_USART0_BPS	19200
//#define SP336_USART1_BPS	19200
//#define SP336_USART2_BPS	19200
#define SP336_USART3_BPS	19200



#define PM10QBIT_PIN_RX_SIGNAL		SP336_USART0_PIN_RX_SIGNAL

#define VP61_VOLTMETER				ADCA
#define VP61_VOLTMETER_CH			ADC_CH0
#define VP61_VOLTMETER_PIN_POS		ADCCH_POS_PIN4
#define VP61_VOLTMETER_PIN_NEG		ADCCH_POS_PIN0



#endif /* MOGGIE_H_ */

