
#ifndef DL_INTERNAL_CONFIGS_H_
#define DL_INTERNAL_CONFIGS_H_

//
//#if defined (SETUP_CAP_RAIN) || defined (SETUP_CAP_LEVEL)
//#include "Acronet/services/CAP/cap_common.h"
//#include "Acronet/services/CAP/cap_producer.h"
//#include "Acronet/services/CAP/cap_consumer.h"
//#endif
//
//
//

#include "boost/preprocessor.hpp"
#include "modinst/modconf.h"

///////////////////////////////////////////////////////////////////////////////
//Computes the module DB record type, it also include the module header file
//
#include "Acronet/services/DB/DBRecord.h"


static void dl_reset_data(void);
static void dl_get_data(DB_RECORD * const);
static RET_ERROR_CODE dl_Data2String(  const DB_RECORD * const,char * const,size_t * );


/* Module interface */
typedef RET_ERROR_CODE ( * INITMODULE    )(void);
typedef void           ( * ENABLEMODULE  )(void);
typedef void           ( * DISABLEMODULE )(void);
typedef bool           ( * YIELDMODULE   )(void);
typedef void           ( * RESETDATA     )(void);
typedef void           ( * GETDATA       )(DB_RECORD * const st);
typedef RET_ERROR_CODE ( * DATA2STRING   )(  const DB_RECORD * const st
											,char * const sz
											,size_t * len_sz       );

#ifdef RMAP_SERVICES
typedef RET_ERROR_CODE ( * DATA2STRINGRMAP   )(  uint8_t * const subModule
												,const void * const st
												,const uint32_t timeStamp
												,const uint16_t timeWindow
												,char * const szTopic
												,int16_t * const len_szTopic
												,char * const szMessage
												,int16_t * const len_szMessage );
#endif


//typedef RET_ERROR_CODE ( * SANITYCHECK   )(void);


typedef struct {
	INITMODULE		pFN_Init;
	ENABLEMODULE	pFN_Enable;
	DISABLEMODULE	pFN_Disable;
	YIELDMODULE     pFN_Yield;
	RESETDATA		pFN_ResetData;
	GETDATA			pFN_GetData;
	DATA2STRING		pFN_Data2String;
#ifdef RMAP_SERVICES
	DATA2STRINGRMAP pFN_Data2StringRMAP;
#endif
}  MODULE_INTERFACE;



#include "modinst/modinst_impl.h"






/*******************************************************************/
//	Introspection Configuration table
/*******************************************************************/
#if defined(SETUP_CAP_LEVEL) || defined(SETUP_CAP_RAIN)

const __flash CAP_INTROSPECTION g_cap_introspection[4] = {
	{"RTIPS",1,(void *) cap_rtips },
	{"PLCOEFF",0,(void *) cap_plcoeff},
	{"LEV",0,(void *) cap_lev},
	{"LEVBIAS",0,(void *) cap_levbias}
};
#endif




#endif /* DL_INTERNAL_CONFIGS_H_ */