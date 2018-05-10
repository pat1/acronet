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

#ifndef MODINST_PARAM_ID
#error "This file requires MODINST_PARAM_ID"
#endif

#ifndef MODULE_CHANNEL_GLUE_FILE
#error "This file requires a defined MODULE_CHANNEL_GLUE_FILE"
#endif

#include "boost/preprocessor.hpp"
#include "Acronet/datalogger/modinst/modconf.h"

//#define DEC_FILTER(s, data, elem) BOOST_PP_EQUAL(data,BOOST_PP_TUPLE_ELEM(2,0,elem))
//#define RESTRICTED_DECLARATION BOOST_PP_SEQ_FILTER(DEC_FILTER,MODINST_PARAM_ID,MODULE_DECLARATION)

#define BOOST_PP_ITERATION_PARAMS_1 (3, (0, BOOST_PP_SEQ_SIZE(MODULE_DECLARATION)-1, "Acronet/datalogger/modinst/generators/mod_glue.h"))

#include BOOST_PP_ITERATE()

#undef DEC_FILTER
#undef RESTRICTED_DECLARATION
#undef BOOST_PP_ITERATION_PARAMS_1
