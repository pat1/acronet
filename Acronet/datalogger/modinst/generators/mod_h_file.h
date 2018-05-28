///*
 //* ACRONET Project
 //* http://www.acronet.cc
 //*
 //* Copyright ( C ) 2014 Acrotec srl
 //* All rights reserved.
 //*
 //* This software may be modified and distributed under the terms
 //* of the EUPL v.1.1 license.  See http://ec.europa.eu/idabc/eupl.html for details.
 //*/
//
//
//#pragma message"ITERATION TEST " BOOST_PP_STRINGIZE(BOOST_PP_ITERATION_DEPTH())
//#pragma message"ITERATION MODULE " BOOST_PP_STRINGIZE(MODINST_PARAM_ID)
//
//#if !BOOST_PP_IS_ITERATING
//#pragma message"ITERATION LEVEL 1"
//#elif BOOST_PP_ITERATION_DEPTH() == 1
//#include "Acronet/datalogger/modinst/generators/mod_h_file1a.h"
//#elif BOOST_PP_ITERATION_DEPTH() == 2
//#include "Acronet/datalogger/modinst/generators/mod_h_file1b.h"
//#endif
//
//

#if !BOOST_PP_IS_ITERATING
#error "This module should be in an iterarion"
#elif BOOST_PP_ITERATION_DEPTH() == 1
#define DECLARATION_ITER2 BOOST_PP_SEQ_ELEM(BOOST_PP_FRAME_ITERATION(1),MODULE_DECLARATION)
#elif BOOST_PP_ITERATION_DEPTH() == 2
#define DECLARATION_ITER2 BOOST_PP_SEQ_ELEM(BOOST_PP_FRAME_ITERATION(2),MODULE_DECLARATION)
#endif


//#define DECLARATION_ITER2 BOOST_PP_SEQ_ELEM(BOOST_PP_FRAME_ITERATION(1),MODULE_DECLARATION)
#define REGISTRY_ITER2 BOOST_PP_TUPLE_ELEM(2,0,DECLARATION_ITER2)
#define CHANNEL_SEQ    BOOST_PP_TUPLE_ELEM(2,1,DECLARATION_ITER2)

#if MODINST_PARAM_ID == REGISTRY_ITER2

#if !BOOST_PP_IS_ITERATING
#define BOOST_PP_ITERATION_PARAMS_1 (3, (0, BOOST_PP_SEQ_SIZE(CHANNEL_SEQ)-1, "Acronet/datalogger/modinst/generators/mod_h_file2.h"))
#elif BOOST_PP_ITERATION_DEPTH() == 1
#define BOOST_PP_ITERATION_PARAMS_2 (3, (0, BOOST_PP_SEQ_SIZE(CHANNEL_SEQ)-1, "Acronet/datalogger/modinst/generators/mod_h_file2.h"))
#elif BOOST_PP_ITERATION_DEPTH() == 2
#define BOOST_PP_ITERATION_PARAMS_3 (3, (0, BOOST_PP_SEQ_SIZE(CHANNEL_SEQ)-1, "Acronet/datalogger/modinst/generators/mod_h_file2.h"))
#endif
#include BOOST_PP_ITERATE()


#ifdef MODULE_INTERFACE_DATA2STRING
RET_ERROR_CODE MODULE_INTERFACE_DATA2STRING(const MODULE_PUBLIC_DATATYPE * const, char * const, size_t * const);
#endif

#ifdef MODULE_INTERFACE_DATA2STRING_RMAP
RET_ERROR_CODE MODULE_INTERFACE_DATA2STRING_RMAP( uint8_t * const, const MODULE_PUBLIC_DATATYPE * const, const uint32_t, const uint16_t, char * const, int16_t * const,char * const,int16_t * const);
#endif

#if !BOOST_PP_IS_ITERATING
#undef BOOST_PP_ITERATION_PARAMS_1
#elif BOOST_PP_ITERATION_DEPTH() == 1
#undef BOOST_PP_ITERATION_PARAMS_2
#elif BOOST_PP_ITERATION_DEPTH() == 2
#undef BOOST_PP_ITERATION_PARAMS_3
#endif

#endif

#undef CHANNEL_SEQ
#undef DECLARATION_ITER2
#undef REGISTRY_ITER2
