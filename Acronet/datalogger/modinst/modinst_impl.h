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
//Computes the module DB record type, it also include the module header file
//
#include "Acronet/services/DB/DBRecord.h"

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
#define BOOST_PP_ITERATION_LIMITS (0,BOOST_PP_SEQ_SIZE(MODULE_DECLARATION)-1)
#define BOOST_PP_FILENAME_1       "Acronet/datalogger/modinst/generators/mod_fn_table.h"
#include BOOST_PP_ITERATE()

#undef BOOST_PP_ITERATION_LIMITS
#undef BOOST_PP_FILENAME_1
};

#endif /* MODINST_IMPL_H_ */