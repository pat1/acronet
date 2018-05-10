/*
 * DBRecord.h
 *
 * Created: 08/05/2018 13:44:59
 *  Author: fabio
 */ 


#ifndef DBRECORD_H_
#define DBRECORD_H_


///////////////////////////////////////////////////////////////////////////////
//Computes the header file to be included
//
#define BOOST_PP_ITERATION_LIMITS (0,BOOST_PP_SEQ_SIZE(MODULE_DECLARATION)-1)
#define BOOST_PP_FILENAME_1       "Acronet/datalogger/modinst/generators/mod_include_selector.h"
#include BOOST_PP_ITERATE()
#undef BOOST_PP_ITERATION_LIMITS
#undef BOOST_PP_FILENAME_1


///////////////////////////////////////////////////////////////////////////////
//Computes DB_RECORD structure
//

typedef struct DB_RECORD {
	uint8_t flags;
	uint32_t data_timestamp;

#define BOOST_PP_ITERATION_LIMITS (0,BOOST_PP_SEQ_SIZE(MODULE_DECLARATION)-1)
#define BOOST_PP_FILENAME_1       "Acronet/datalogger/modinst/generators/mod_dbrecord_declaration.h"
#include BOOST_PP_ITERATE()
#undef BOOST_PP_ITERATION_LIMITS
#undef BOOST_PP_FILENAME_1

} __attribute__((packed)) DB_RECORD;


#endif /* DBRECORD_H_ */