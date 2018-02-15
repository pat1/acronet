/*
 * db_conf.h
 *
 * Created: 26/08/2016 11:15:35
 *  Author: fabio
 */ 


#ifndef DB_CONF_H_
#define DB_CONF_H_



//
//Partitions must respect the page size of 256 bytes
//
// The first page (0 to 256) is reserved

#define EXT_EEPROM_MEMORY_SIZE 524288
#define EXT_EEPROM_PAGE_SIZE 256

#define PARTITION_LOG_SIZE 1024

#define PARTITION_DB_BEGIN (EXT_EEPROM_PAGE_SIZE)
#define PARTITION_DB_END (EXT_EEPROM_MEMORY_SIZE-PARTITION_LOG_SIZE)
#define PARTITION_LOG_END (EXT_EEPROM_MEMORY_SIZE)
#define PARTITION_LOG_BEGIN (EXT_EEPROM_MEMORY_SIZE-PARTITION_LOG_SIZE)

#define PARTITION_DB_SIZE (PARTITION_DB_END - PARTITION_DB_BEGIN)


#endif /* DB_CONF_H_ */