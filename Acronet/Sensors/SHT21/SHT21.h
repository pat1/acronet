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


#ifndef SHT21_H_
#define SHT21_H_

typedef union {
	int16_t wval;
	int8_t bval[2];
} SHTVAL;


int SHT21_Init(void);

status_code_t SHT21_TriggerReadTemp_hold(SHTVAL * val);
status_code_t SHT21_TriggerReadRH_hold(SHTVAL * val);

status_code_t SHT21_TriggerReadTemp_noHold(SHTVAL * val);
status_code_t SHT21_TriggerReadRH_noHold(SHTVAL * val);




#endif /* SHT21_H_ */