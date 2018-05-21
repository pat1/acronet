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


#ifndef MODINST_IMPL_H_
#define MODINST_IMPL_H_


///////////////////////////////////////////////////////////////////////////////
//Computes the header file to be included
//
/*
#define BOOST_PP_ITERATION_LIMITS (0,BOOST_PP_SEQ_SIZE(MODULE_DECLARATION)-1)
#define BOOST_PP_FILENAME_1       "Acronet/datalogger/modinst/generators/mod_include_selector.h"
#include BOOST_PP_ITERATE()

#undef BOOST_PP_ITERATION_LIMITS
#undef BOOST_PP_FILENAME_1
*/

///////////////////////////////////////////////////////////////////////////////
//Computes the module enumerators
//
#include "Acronet/datalogger/modinst/generators/mod_enum_declaration.h"


///////////////////////////////////////////////////////////////////////////////
//Computes function interface stubs
//
#define BOOST_PP_ITERATION_PARAMS_1 (3, (0, BOOST_PP_SEQ_SIZE(MODULE_DECLARATION)-1, "Acronet/datalogger/modinst/generators/mod_fn_iface.h"))
#include BOOST_PP_ITERATE()

#undef BOOST_PP_ITERATION_PARAMS_1


///////////////////////////////////////////////////////////////////////////////
//Computes function interface table
//
static const __flash MODULE_INTERFACE iface_module[] = {
	////////////////////////////////////////////////////////////////////////////////////
	// DATALOGGER MODULE INTERFACE
	////////////////////////////////////////////////////////////////////////////////////
	{	NULL,
		NULL,
		NULL,
		NULL,
		dl_reset_data,
		( GETDATA )		dl_get_data,
		( DATA2STRING ) dl_Data2String,
		#ifdef RMAP_SERVICES
		NULL
		#endif	//RMAP_SERVICES
	},

#define BOOST_PP_ITERATION_LIMITS (0,BOOST_PP_SEQ_SIZE(MODULE_DECLARATION)-1)
#define BOOST_PP_FILENAME_1       "Acronet/datalogger/modinst/generators/mod_fn_table.h"
#include BOOST_PP_ITERATE()

#undef BOOST_PP_ITERATION_LIMITS
#undef BOOST_PP_FILENAME_1
};


#ifdef PERIODIC_TABLE
#error "PERIODIC_TABLE was already defined somewhere else"
#endif



#include "boost/preprocessor/slot/counter.hpp" 

static void dl_periodic_update( void )
{
	typedef void  ( * PERIODICFN  )(void);
	
	static void (* const __flash fnPeriodic[])(void) = { 
#define BOOST_PP_ITERATION_LIMITS (0,BOOST_PP_SEQ_SIZE(MODULE_DECLARATION)-1)
#define BOOST_PP_FILENAME_1       "Acronet/datalogger/modinst/generators/mod_periodic_table.h"
#include BOOST_PP_ITERATE()
#undef BOOST_PP_ITERATION_LIMITS
#undef BOOST_PP_FILENAME_1
		};

#if(BOOST_PP_COUNTER>255)
#error "This implentation assumes a max of 255 periodic functions"
#endif

	static const __flash uint8_t tableSize = BOOST_PP_COUNTER;
	
	static uint8_t idx = 0;
	
	for (uint8_t i=idx;i<tableSize;i++)
	{
		fnPeriodic[i]();
	}
	
	for (uint8_t i=0;i<idx;i++)
	{
		fnPeriodic[i]();
	}

	if ( ++idx >= tableSize )
	{
		idx = 0;
	}
	
}


#endif /* MODINST_IMPL_H_ */