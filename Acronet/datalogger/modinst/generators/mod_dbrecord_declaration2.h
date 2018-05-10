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



#define ISTANCE_NUM BOOST_PP_FRAME_ITERATION(2)

#undef MODULE_ISTANCE_DATA

#ifdef MODULE_PUBLIC_DATATYPE
#define MODULE_ISTANCE_DATA  BOOST_PP_CAT(MODULE_PUBLIC_DATATYPE,BOOST_PP_CAT(_ist_,ISTANCE_NUM))
MODULE_PUBLIC_DATATYPE MODULE_ISTANCE_DATA;
#else
#error "It is mandatory to define MODULE_PUBLIC_DATATYPE for each module"
#endif

