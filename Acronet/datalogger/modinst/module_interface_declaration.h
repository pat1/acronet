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

#include "boost/preprocessor.hpp"
#include "Acronet/datalogger/modinst/modconf.h"


#if !BOOST_PP_IS_ITERATING
#define BOOST_PP_ITERATION_PARAMS_1 (3, (0, BOOST_PP_SEQ_SIZE(MODULE_DECLARATION)-1, "Acronet/datalogger/modinst/generators/mod_h_file.h"))
#elif BOOST_PP_ITERATION_DEPTH() == 1
#define BOOST_PP_ITERATION_PARAMS_2 (3, (0, BOOST_PP_SEQ_SIZE(MODULE_DECLARATION)-1, "Acronet/datalogger/modinst/generators/mod_h_file.h"))
#elif BOOST_PP_ITERATION_DEPTH() == 2
#define BOOST_PP_ITERATION_PARAMS_3 (3, (0, BOOST_PP_SEQ_SIZE(MODULE_DECLARATION)-1, "Acronet/datalogger/modinst/generators/mod_h_file.h"))
#endif

#include BOOST_PP_ITERATE()

#if !BOOST_PP_IS_ITERATING
#undef BOOST_PP_ITERATION_PARAMS_1
#elif BOOST_PP_ITERATION_DEPTH() == 1
#undef BOOST_PP_ITERATION_PARAMS_2
#elif BOOST_PP_ITERATION_DEPTH() == 2
#undef BOOST_PP_ITERATION_PARAMS_3
#endif

