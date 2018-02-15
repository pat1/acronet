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



#ifndef DATALOGGER_H_
#define DATALOGGER_H_

extern volatile uint8_t dl_cycle_lock;

void dl_run(void);
RET_ERROR_CODE dl_init(void);

void dl_dump_db(void);
void dl_dump_db2(void);

#endif /* DATALOGGER_H_ */