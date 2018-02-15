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


#ifndef PM10QBIT_H_
#define PM10QBIT_H_

//#define PM10QBIT_PIN_RX_SIGNAL SP336_USART0_PIN_RX_SIGNAL

typedef struct {
	// remember to initialize attributes
	// PM10QBIT_STATS PM10val = {.mean = 0.0f, .samples = 0, .maxVal = 0, .minVal = 0};
	uint16_t samples;
	uint16_t mean;
	uint16_t meanZeros;
	//uint16_t maxVal;
	//uint16_t minVal;
	uint16_t zeros;
} PM10QBIT_DATA;


void PM10QBIT_init(void);
void PM10QBIT_enable(void);
void PM10QBIT_disable(void);
void PM10QBIT_get_data (PM10QBIT_DATA * const);
void PM10QBIT_reset_data(void);
bool PM10QBIT_Yield(void);

RET_ERROR_CODE PM10QBIT_Data2String(const PM10QBIT_DATA * const st,char * const sz, int16_t * len_sz);


#endif /* PM10QBIT_H_ */