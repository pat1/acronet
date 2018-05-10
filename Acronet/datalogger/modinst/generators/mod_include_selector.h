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


#define REGISTRY_ITER BOOST_PP_TUPLE_ELEM(2,0,BOOST_PP_SEQ_ELEM(BOOST_PP_ITERATION(),MODULE_DECLARATION))
#define MODULE_HEADER_FILE BOOST_PP_TUPLE_ELEM(3,2,BOOST_PP_SEQ_ELEM(REGISTRY_ITER, MODULE_REGISTRY))

#undef MODULE_PUBLIC_DATATYPE

#undef MODULE_CHANNEL_GLUE_FILE
#undef MODULE_INTERFACE_INIT
#undef MODULE_INTERFACE_ENABLE
#undef MODULE_INTERFACE_DISABLE
#undef MODULE_INTERFACE_YIELD
#undef MODULE_INTERFACE_RESET
#undef MODULE_INTERFACE_GETDATA
#undef MODULE_INTERFACE_DATA2STRING
#undef MODULE_INTERFACE_PERIODIC


#include MODULE_HEADER_FILE

#undef MODULE_HEADER_FILE
#undef REGISTRY_ITER


