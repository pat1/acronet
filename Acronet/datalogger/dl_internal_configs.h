
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


/* Module interface */
typedef RET_ERROR_CODE ( * INITMODULE    )(void);
typedef void           ( * ENABLEMODULE  )(void);
typedef void           ( * DISABLEMODULE )(void);
typedef bool           ( * YIELDMODULE   )(void);
typedef void           ( * RESETDATA     )(void);
typedef void           ( * GETDATA       )(void * const st);
typedef RET_ERROR_CODE ( * DATA2STRING   )(  const void * const st
											,char * const sz
											,int16_t * len_sz       );

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

#include "boost/preprocessor.hpp"
#include "modinst/modconf.h"
#include "modinst/modinst_impl.h"


	

/*
RET_ERROR_CODE DB_select(const struct DB_RECORD * const pRecord,const uint8_t fieldID, DB_FIELD * const pField)
{
	
	
	return AC_ERROR_OK;
}
*/

/*******************************************************************/
//	ADC Configuration table
/*******************************************************************/

#include "Acronet/drivers/ADC/ADC_Manager.h"


const __flash ADC_MAN_SETUP g_tbl_ADC_MAN_SETUP[] = {
	
	{ 'A',0,0,0,0,NULL,NULL,"demo1" },
	{ 'B',0,0,0,0,0,"demo2" },
	{ 'A',0,0,0,0,0,"demo3" }
	
};


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


static void dl_periodic_update( void )
{
	typedef void  ( * PERIODICFN  )(void);
	
	static void (* const __flash fnPeriodic[])(void) = {
									#ifdef SETUP_VP61
										vp61_periodic,
									#endif
									#ifdef SETUP_T023_MODBUS
										t023_periodic,
									#endif
									#ifdef SETUP_T026_MODBUS
										t026_periodic,
									#endif
									#ifdef SETUP_T056_MODBUS
										t056_periodic,
									#endif
									#ifdef SETUP_HD3910_MODBUS
										hd3910_periodic,
									#endif
									#ifdef SETUP_GPIO2LOG
										gpio2log_periodic,
									#endif
									};

	static const __flash uint8_t tableSize = sizeof(fnPeriodic) / sizeof(PERIODICFN);
									
	static uint8_t idx = 0;
	
	for (uint8_t i=idx;i<tableSize;i++)
	{
		fnPeriodic[i]();
	}
										
	for (uint8_t i=0;i<idx;i++)
	{
		fnPeriodic[i]();
	}

	if ( ++idx >= tableSize )
	{
		idx = 0;
	}
										
}


#endif /* DL_INTERNAL_CONFIGS_H_ */