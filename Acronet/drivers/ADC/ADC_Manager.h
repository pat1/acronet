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


#ifndef ADC_MANAGER_H_
#define ADC_MANAGER_H_

typedef struct {
	
	char adc;
	
	uint8_t channel;
	enum adc_sign sign;
	enum adc_resolution resolution;
	enum adc_reference  reference;
	
	void (*cb_fnGetValue)(const uint16_t reg);
	void (*cb_fnPreRead)(void);
	void (*cb_fnPostRead)(void);
	
	char szBrief[16];
} ADC_MAN_SETUP;


RET_ERROR_CODE ADC_MAN_Init(const ADC_MAN_SETUP * const pTable,uint8_t tbSize);
RET_ERROR_CODE ADC_MAN_Enable(void);
RET_ERROR_CODE ADC_MAN_Disable(void);





#endif /* ADC_MANAGER_H_ */