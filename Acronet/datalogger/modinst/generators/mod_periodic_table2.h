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


BOOST_PP_COMMA_IF(BOOST_PP_COUNTER) BOOST_PP_CAT(MODULE_INTERFACE_PERIODIC,BOOST_PP_CAT(_ist_,ISTANCE_NUM))
#include BOOST_PP_UPDATE_COUNTER()
