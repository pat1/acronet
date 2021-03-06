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
#else
#error "It is mandatory to define MODULE_PUBLIC_DATATYPE for each module"
#endif

#define MODULE_ISTANCE_CHAN BOOST_PP_SEQ_ELEM(ISTANCE_NUM,MODULE_ISTANCES)
#define METHOD_NAME_ISTANCE_TRAIL BOOST_PP_CAT(_get_ist_,ISTANCE_NUM)
//#define MODULE_ISTANCE_DATA  BOOST_PP_CAT(MODULE_PUBLIC_DATATYPE,BOOST_PP_CAT(_ist_,ISTANCE_NUM))

//#define METHOD_DB_SLOT(db) BOOST_PP_CAT(BOOST_PP_CAT(BOOST_PP_CAT(db.,MODULE_NAME),_ist_),ISTANCE_NUM)



#ifdef MODULE_INTERFACE_GETDATA
#define METHOD_FUNCTION_NAME BOOST_PP_CAT(MODULE_INTERFACE_GETDATA,METHOD_NAME_ISTANCE_TRAIL)
static void METHOD_FUNCTION_NAME(DB_RECORD * const pRec)
{
	BOOST_PP_CAT(MODULE_INTERFACE_GETDATA,BOOST_PP_CAT(_ist_,ISTANCE_NUM))(&(pRec->MODULE_ISTANCE_DATA));
}
#undef METHOD_FUNCTION_NAME
#endif


#ifdef MODULE_INTERFACE_DATA2STRING
#define METHOD_FUNCTION_NAME BOOST_PP_CAT(MODULE_INTERFACE_DATA2STRING,METHOD_NAME_ISTANCE_TRAIL)
static RET_ERROR_CODE  METHOD_FUNCTION_NAME(const DB_RECORD * const pRec,char * const sz, uint16_t * const len_sz)
{
	static const __flash char prolog[] = ";"BOOST_PP_STRINGIZE(MODULE_NAME)":"BOOST_PP_STRINGIZE(ISTANCE_NUM);
	if ((*len_sz)<sizeof(prolog))
	{
		return AC_BUFFER_TOO_SMALL;
	}
	
	strcat_P(sz,prolog);
	*len_sz -= (sizeof(prolog)-1);
	
	const RET_ERROR_CODE e = MODULE_INTERFACE_DATA2STRING(&(pRec->MODULE_ISTANCE_DATA),sz+(sizeof(prolog)-1),len_sz);
	
	*len_sz += (sizeof(prolog)-1);
	
	return e;
}
#undef METHOD_FUNCTION_NAME
#endif

#ifdef MODULE_INTERFACE_DATA2STRING_RMAP
#define METHOD_FUNCTION_NAME BOOST_PP_CAT(MODULE_INTERFACE_DATA2STRING_RMAP,METHOD_NAME_ISTANCE_TRAIL)
static RET_ERROR_CODE METHOD_FUNCTION_NAME(DB_RECORD * const pRec)
{
	
}
#undef METHOD_FUNCTION_NAME
#endif

#undef ISTANCE_NUM
#undef MODULE_PRIVATE_DATA
