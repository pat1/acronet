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


#ifndef EXT_EEPROM_H_
#define EXT_EEPROM_H_

//
//Partitions must respect the page size of 256 bytes
//
// The first page (0 to 256) is reserved

#define EXT_EEPROM_MEMORY_SIZE 524288
#define EXT_EEPROM_PAGE_SIZE 256

#define LOG_EEPROM_PARTITION_SIZE 2048
//#define PARTITION_LOG_SIZE 8192

//#define PARTITION_DB_BEGIN (EXT_EEPROM_PAGE_SIZE)
//#define PARTITION_DB_END (EXT_EEPROM_MEMORY_SIZE-PARTITION_LOG_SIZE)
//#define PARTITION_LOG_END (EXT_EEPROM_MEMORY_SIZE)
//#define PARTITION_LOG_BEGIN (EXT_EEPROM_MEMORY_SIZE-PARTITION_LOG_SIZE)


#define LOG_EEPROM_PARTITION_BEGIN (EXT_EEPROM_PAGE_SIZE)
#define LOG_EEPROM_PARTITION_END (LOG_EEPROM_PARTITION_BEGIN+LOG_EEPROM_PARTITION_SIZE)
#define DB_EEPROM_PARTITION_BEGIN (LOG_EEPROM_PARTITION_END)
#define DB_EEPROM_PARTITION_END (EXT_EEPROM_MEMORY_SIZE)


#define PARTITION_DB_SIZE (PARTITION_DB_END - PARTITION_DB_BEGIN)




#endif /* EXT_EEPROM_H_ */