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


#ifndef CAP_PRODUCER_H_
#define CAP_PRODUCER_H_


RET_ERROR_CODE cap_schedule_check( void );

RET_ERROR_CODE CAP_init(void);
void CAP_enable(void);
void CAP_disable(void);
bool CAP_Yield(void);


RET_ERROR_CODE cap_rtips(float * res,float p1);
RET_ERROR_CODE cap_lev(float * res);

RET_ERROR_CODE cap_levbias(float * res);
RET_ERROR_CODE cap_plcoeff(float * res);

#endif /* CAP_PRODUCER_H_ */