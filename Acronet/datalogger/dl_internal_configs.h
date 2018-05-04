
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



//#include "Acronet/datalogger/modinst/modinst.h"


//static const __flash MODULE_INTERFACE iface_module[] = {
	//
//////////////////////////////////////////////////////////////////////////////////////
//// DATALOGGER MODULE INTERFACE
//////////////////////////////////////////////////////////////////////////////////////	
															//{	NULL,
																//dl_enable,
																//NULL,
																//NULL,
																//dl_reset_data,
																//( GETDATA )		dl_get_data,
																//( DATA2STRING ) dl_Data2String,
//#ifdef RMAP_SERVICES
																//NULL
//#endif	//RMAP_SERVICES
															//},
//
//////////////////////////////////////////////////////////////////////////////////////
//// RAINGAUGE MODULE INTERFACE
//////////////////////////////////////////////////////////////////////////////////////
//
//#ifdef SETUP_RAINGAUGE
															//{	raingauge_init,
																//NULL,
																//NULL,
																//NULL,
																//raingauge_reset_data,
																//( GETDATA )		raingauge_get_data,
																//( DATA2STRING ) raingauge_Data2String,
//#ifdef RMAP_SERVICES
																//( DATA2STRINGRMAP ) raingauge_Data2String_RMAP
//#endif //RMAP_SERVICES
															 //},
//#ifdef SETUP_RAINGAUGE_AUX
															//{	raingauge_init,
																//NULL,
																//NULL,
																//NULL,
																//raingauge_reset_data_aux,
																//( GETDATA )		raingauge_get_data_aux,
																//( DATA2STRING )	raingauge_Data2String_aux,
//#ifdef RMAP_SERVICES
																//( DATA2STRINGRMAP ) raingauge_Data2String_RMAP_aux
//#endif //RMAP_SERVICES
															//},
//#endif //SETUP_RAINGAUGE_AUX
//#endif //SETUP_RAINGAUGE
//
//////////////////////////////////////////////////////////////////////////////////////
//// LB150 MODULE INTERFACE
//////////////////////////////////////////////////////////////////////////////////////
//
//#ifdef SETUP_LB150
															//{	LB150_init,
																//LB150_enable,
																//LB150_disable,
																//LB150_Yield,
																//LB150_reset_data,
																//( GETDATA )		LB150_get_data,
																//( DATA2STRING )	LB150_Data2String,
//#ifdef RMAP_SERVICES
																//NULL
//#endif //RMAP_SERVICES
															//},
//#endif //SETUP_LB150
//
//////////////////////////////////////////////////////////////////////////////////////
//// MBXXXX MODULE INTERFACE
//////////////////////////////////////////////////////////////////////////////////////
//
//#ifdef SETUP_MBXXXX
															//{	MBXXXX_init,
																//MBXXXX_enable,
																//MBXXXX_disable,
																//MBXXXX_Yield,
																//MBXXXX_reset_data,
																//( GETDATA )		MBXXXX_get_data,
																//( DATA2STRING )	MBXXXX_Data2String,	
//#ifdef RMAP_SERVICES
																//NULL
//#endif //RMAP_SERVICES
															//},
//#endif //SETUP_MBXXXX
//
//////////////////////////////////////////////////////////////////////////////////////
//// CV7L MODULE INTERFACE
//////////////////////////////////////////////////////////////////////////////////////
//
//#ifdef SETUP_CV7L
															//{	CV7L_init,
																//CV7L_enable,
																//CV7L_disable,
																//CV7L_Yield,
																//( RESETDATA )   CV7L_reset_data,
																//( GETDATA )		CV7L_get_data,
																//( DATA2STRING )	CV7L_Data2String,	
//#ifdef RMAP_SERVICES
																//NULL
//#endif //RMAP_SERVICES
															//},
//#endif //SETUP_CV7L
//
//////////////////////////////////////////////////////////////////////////////////////
//// L8095N MODULE INTERFACE
//////////////////////////////////////////////////////////////////////////////////////
//
//#ifdef SETUP_L8095N
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
//#endif //SETUP_L8095N
//
//////////////////////////////////////////////////////////////////////////////////////
//// L8095N MODULE INTERFACE
//////////////////////////////////////////////////////////////////////////////////////
//
//#ifdef SETUP_L8095N_SEC
															//{	l8095n_sec_init,
																//l8095n_sec_enable,
																//l8095n_sec_disable,
																//l8095n_sec_Yield,
																//( RESETDATA )   l8095n_sec_reset_data,
																//( GETDATA )		l8095n_sec_get_data,
																//( DATA2STRING )	l8095n_sec_Data2String,
//#ifdef RMAP_SERVICES
																//( DATA2STRINGRMAP ) l8095n_sec_Data2String_RMAP
//#endif //RMAP_SERVICES
															//},
//#endif //SETUP_L8095N
//
//////////////////////////////////////////////////////////////////////////////////////
//// FMX167 MODULE INTERFACE
//////////////////////////////////////////////////////////////////////////////////////
//
//
//#ifdef SETUP_FMX167
															//{	fmx167_init,
																//fmx167_enable,
																//fmx167_disable,
																//fmx167_Yield,
																//fmx167_reset_data,
																//( GETDATA )		fmx167_get_data,
																//( DATA2STRING )	fmx167_Data2String,	
//#ifdef RMAP_SERVICES
																//NULL
//#endif //RMAP_SERVICES
															//},
//#endif //SETUP_FMX167
//
//////////////////////////////////////////////////////////////////////////////////////
//// VP61 MODULE INTERFACE
//////////////////////////////////////////////////////////////////////////////////////
//
//#ifdef SETUP_VP61
															//{	vp61_init,
																//vp61_enable,
																//vp61_disable,
																//NULL,
																//vp61_reset_data,
																//( GETDATA )		vp61_get_data,
																//( DATA2STRING )	vp61_Data2String,
//#ifdef RMAP_SERVICES
																//( DATA2STRINGRMAP ) vp61_Data2String_RMAP
//#endif //RMAP_SERVICES
															//},
//#endif //SETUP_VP61
//
//////////////////////////////////////////////////////////////////////////////////////
//// PM10QBIT MODULE INTERFACE
//////////////////////////////////////////////////////////////////////////////////////
//
//#ifdef SETUP_PM10QBIT
															//{	PM10QBIT_init,
																//PM10QBIT_enable,
																//PM10QBIT_disable,
																//PM10QBIT_Yield,
																//PM10QBIT_reset_data,
																//( GETDATA )		PM10QBIT_get_data,
																//( DATA2STRING )	PM10QBIT_Data2String,	
//#ifdef RMAP_SERVICES
																//NULL
//#endif //RMAP_SERVICES
															//},
//#endif //SETUP_PM10QBIT
//
//////////////////////////////////////////////////////////////////////////////////////
//// SENSIT MODULE INTERFACE
//////////////////////////////////////////////////////////////////////////////////////
//
//#ifdef SETUP_SENSIT
															//{	sens_it_init,
																//sens_it_enable,
																//sens_it_disable,
																//sens_it_Yield,
																//sens_it_reset_data,
																//( GETDATA )		sens_it_get_data,
																//( DATA2STRING )	sens_it_Data2String,	
//#ifdef RMAP_SERVICES
																//NULL
//#endif //RMAP_SERVICES
															//},
//#endif //SETUP_SENSIT
//
//////////////////////////////////////////////////////////////////////////////////////
//// CAP MODULE INTERFACE
//////////////////////////////////////////////////////////////////////////////////////
//
//#ifdef SETUP_CAP
															//{	CAP_init,
																//CAP_enable,
																//CAP_disable,
																//CAP_Yield,
																//NULL,
																//NULL,
																//NULL,
//#ifdef RMAP_SERVICES
																//NULL
//#endif //RMAP_SERVICES
															//},
//
//#endif //SETUP_CAP
//
//#ifdef SETUP_GPIO2LOG
															//{	gpio2log_init,
																//NULL,
																//NULL,
																//gpio2log_Yield,
																//NULL,
																//NULL,
																//NULL,
																//#ifdef RMAP_SERVICES
																//NULL
																//#endif //RMAP_SERVICES
															//},
//
//#endif //SETUP_GPIO2LOG
//
//#ifdef SETUP_PANEL
															//{	panel_init,
																//NULL,
																//NULL,
																//NULL,
																//panel_reset_data,
																//( GETDATA )		panel_get_data,
																//( DATA2STRING )	panel_Data2String,
																//#ifdef RMAP_SERVICES
																//NULL
																//#endif //RMAP_SERVICES
															//},
//
//#endif //SETUP_PANEL
//
//#ifdef SETUP_T023_MODBUS
															//{	t023_init,
																//NULL,
																//NULL,
																//t023_Yield,
																//t023_reset_data,
																//( GETDATA )		t023_get_data,
																//( DATA2STRING )	t023_Data2String,
//#ifdef RMAP_SERVICES
																//NULL
//#endif //RMAP_SERVICES
															//},
//#endif
//#ifdef SETUP_T026_MODBUS
															//{	t026_init,
																//NULL,
																//NULL,
																//t026_Yield,
																//t026_reset_data,
																//( GETDATA )		t026_get_data,
																//( DATA2STRING )	t026_Data2String,
//#ifdef RMAP_SERVICES
																//NULL
//#endif //RMAP_SERVICES
															//},
//#endif
//#ifdef SETUP_T056_MODBUS
															//{	t056_init,
																//NULL,
																//NULL,
																//t056_Yield,
																//t056_reset_data,
																//( GETDATA )		t056_get_data,
																//( DATA2STRING )	t056_Data2String,
//#ifdef RMAP_SERVICES
																//NULL
//#endif //RMAP_SERVICES
															//},
//#endif
//#ifdef SETUP_HD3910_MODBUS
															//{	hd3910_init,
																//NULL,
																//NULL,
																//hd3910_Yield,
																//hd3910_reset_data,
																//( GETDATA )		hd3910_get_data,
																//( DATA2STRING )	hd3910_Data2String,
//
//#ifdef RMAP_SERVICES
//NULL
//#endif //RMAP_SERVIC
															//},
//#endif
//
//};
	

static inline size_t __attribute__((const)) selectDataSource( const uint8_t idx )
{

	switch(idx) {

		case DL_INTERNAL:
		return offsetof(DB_RECORD,dl_data);
		break;
		
#ifdef SETUP_RAINGAUGE
		case RAINGAUGE1:
		return offsetof(DB_RECORD,raingauge_data[0]);  
		break;
#ifdef SETUP_RAINGAUGE_AUX
		case RAINGAUGE2:
		return offsetof(DB_RECORD,raingauge_data[1]);  
		break;
#endif
#endif
#ifdef SETUP_LB150
		case LB150:
		return offsetof(DB_RECORD,lb150_data);
		break;
#endif
#ifdef SETUP_MBXXXX
		case MBXXXX:
		return offsetof(DB_RECORD,mbXXXX_data);
		break;
#endif
#ifdef SETUP_CV7L
		case CV7L:
		return  offsetof(DB_RECORD,cv7l_data);
		break;
#endif
#ifdef SETUP_L8095N
		case L8095N:
		return offsetof(DB_RECORD,l8095n_data);
		break;
#endif
#ifdef SETUP_L8095N_SEC
		case L8095N_SEC:
		return offsetof(DB_RECORD,l8095n_sec_data);
		break;
#endif
#ifdef SETUP_VP61
		case VP61:
		return offsetof(DB_RECORD,vp61_data);
		break;
#endif
#ifdef SETUP_PM10QBIT
		case PM10QBIT:
		return  offsetof(DB_RECORD,pm10qbit_data);
		break;
#endif
#ifdef SETUP_SENSIT
		case SENSIT:
		return offsetof(DB_RECORD,sens_it_stats);
		break;
#endif
#ifdef SETUP_FMX167
		case FMX167:
		return offsetof(DB_RECORD,fmx_data);
		break;
#endif
#ifdef SETUP_PANEL
		case PANEL:
		return offsetof(DB_RECORD,panel_data);
		break;
#endif
#ifdef SETUP_T023_MODBUS
		case T023_MODBUS:
		return offsetof(DB_RECORD,t023_data);
		break;
#endif
#ifdef SETUP_T026_MODBUS
		case T026_MODBUS:
		return offsetof(DB_RECORD,t026_data);
		break;
#endif
#ifdef SETUP_T056_MODBUS
		case T056_MODBUS:
		return offsetof(DB_RECORD,t056_data);
		break;
#endif
#ifdef SETUP_HD3910_MODBUS
		case HD3910_MODBUS:
		return offsetof(DB_RECORD,hd3910_data);
		break;
#endif

		default:
		return 0xFFFF;
	}
	
	return 0xFFFF;
}

RET_ERROR_CODE DB_select(const struct DB_RECORD * const pRecord,const uint8_t fieldID, DB_FIELD * const pField)
{
	
	
	return AC_ERROR_OK;
}


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