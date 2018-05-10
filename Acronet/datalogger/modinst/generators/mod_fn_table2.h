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

															//{	l8095n_init,
															//l8095n_enable,
															//l8095n_disable,
															//l8095n_Yield,
															//( RESETDATA )   l8095n_reset_data,
															//( GETDATA )		l8095n_get_data,
															//( DATA2STRING )	l8095n_Data2String,
															//#ifdef RMAP_SERVICES
															//( DATA2STRINGRMAP ) l8095n_Data2String_RMAP
															//#endif //RMAP_SERVICES
															//},


#define MODULE_NUM BOOST_PP_ITERATION()


#undef MODULE_PRIVATE_DATA


#define METHOD_FUNCTION(MNAME) BOOST_PP_CAT(MNAME,BOOST_PP_CAT(_ist_,MODULE_NUM))

	{
#ifdef MODULE_INTERFACE_INIT
		(INITMODULE) METHOD_FUNCTION(MODULE_INTERFACE_INIT),
#else
		NULL,
#endif

#ifdef MODULE_INTERFACE_ENABLE
		(ENABLEMODULE) METHOD_FUNCTION(MODULE_INTERFACE_ENABLE),
#else
		NULL,
#endif

#ifdef MODULE_INTERFACE_DISABLE
		(DISABLEMODULE) METHOD_FUNCTION(MODULE_INTERFACE_DISABLE),
#else
		NULL,
#endif

#ifdef MODULE_INTERFACE_YIELD
		(YIELDMODULE) METHOD_FUNCTION(MODULE_INTERFACE_YIELD),
#else
		NULL,
#endif

#ifdef MODULE_INTERFACE_RESET
		(RESETDATA)METHOD_FUNCTION(MODULE_INTERFACE_RESET),
#else
		NULL,
#endif


#ifdef MODULE_INTERFACE_GETDATA
		(GETDATA)METHOD_FUNCTION(MODULE_INTERFACE_GETDATA),
#else
		NULL,
#endif


#ifdef MODULE_INTERFACE_DATA2STRING
		(DATA2STRING)METHOD_FUNCTION(MODULE_INTERFACE_DATA2STRING),
#else
		NULL,
#endif

#ifdef RMAP_SERVICES
#ifdef MODULE_INTERFACE_DATA2STRING_RMAP
		(DATA2STRINGRMAP)METHOD_FUNCTION(MODULE_INTERFACE_DATA2STRING_RMAP)
#else
		NULL
#endif
#endif
	},