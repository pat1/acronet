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


#define DECLARATION_ITER BOOST_PP_SEQ_ELEM(BOOST_PP_FRAME_ITERATION(1),MODULE_DECLARATION)
#define REGISTRY_ITER BOOST_PP_TUPLE_ELEM(2,0,DECLARATION_ITER)

#if MODINST_PARAM_ID == REGISTRY_ITER

#define MODULE_ISTANCES BOOST_PP_TUPLE_ELEM(2,1,DECLARATION_ITER)
#define BOOST_PP_ITERATION_PARAMS_2 (3, (0, BOOST_PP_SEQ_SIZE(MODULE_ISTANCES)-1, MODULE_CHANNEL_GLUE_FILE))
#include BOOST_PP_ITERATE()

#undef BOOST_PP_ITERATION_PARAMS_2
#undef MODULE_ISTANCES
#endif

#undef REGISTRY_ITER
#undef DECLARATION_ITER


