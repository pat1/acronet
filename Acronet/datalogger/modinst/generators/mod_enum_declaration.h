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


#ifndef MOD_ENUM_DECLARATION_H_
#define MOD_ENUM_DECLARATION_H_


///////////////////////////////////////////////////////////////////////////////
//
// Module enumerator
//

#define M10(z,n,data) BOOST_PP_CAT(BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2,1,BOOST_PP_SEQ_ELEM(data,MODULE_REGISTRY)),_),n),
#define M1(r,data,elem) BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(BOOST_PP_TUPLE_ELEM(2,1,elem)),M10,BOOST_PP_TUPLE_ELEM(2,0,elem))

typedef enum {
	DL_MODULE_BEG = 0,
	DL_INTERNAL = DL_MODULE_BEG,
	BOOST_PP_SEQ_FOR_EACH(M1,_,MODULE_DECLARATION)
	DL_MODULE_END
} MODULE_ID;


#undef M1
#undef M10

#endif /* MOD_ENUM_DECLARATION_H_ */