
#ifndef DL_INTERNAL_CONFIGS_H_
#define DL_INTERNAL_CONFIGS_H_

#ifdef SETUP_PANEL
#include "Acronet/Sensors/PANEL/panel.h"
#endif

#ifdef SETUP_RAINGAUGE
#include "Acronet/Sensors/raingauge/pulse_raingauge.h"
#endif
#ifdef SETUP_CV7L
#include "Acronet/Sensors/CV7L/CV7L.h"
#endif
#ifdef SETUP_MBXXXX
#include "Acronet/Sensors/MBXXXX/MBXXXX.h"
#endif
#ifdef SETUP_VP61
#include "Acronet/Sensors/VP61/vp61.h"
#endif
#ifdef SETUP_L8095N
#include "Acronet/Sensors/L8095-N/L8095-N.h"
#endif
#ifdef SETUP_L8095N_SEC
#include "Acronet/Sensors/L8095-N-SEC/L8095-N-SEC.h"
#endif
#ifdef SETUP_LB150
#include "Acronet/Sensors/LB150/LB150.h"
#endif
#ifdef SETUP_PM10QBIT
#include "Acronet/Sensors/PM10Qbit/PM10Qbit.h"
#endif
#ifdef SETUP_SENSIT
#include "Acronet/Sensors/SENS-IT/sens-it.h"
#endif
#ifdef SETUP_FMX167
#include "Acronet/Sensors/FMX167/fmx167.h" 
#endif
#ifdef SETUP_GPIO2LOG
#include "Acronet/Sensors/GPIO2LOG/gpio2log.h"
#endif
#ifdef SETUP_T023_MODBUS
#include "Acronet/Sensors/SIAP_MICROS/t023/t023.h"
#endif

#if defined (SETUP_CAP_RAIN) || defined (SETUP_CAP_LEVEL)
#include "Acronet/services/CAP/cap_common.h"
#include "Acronet/services/CAP/cap_producer.h"
#include "Acronet/services/CAP/cap_consumer.h"
#endif



typedef enum {
	DL_MODULE_BEG = 0,
	DL_INTERNAL = DL_MODULE_BEG,
#ifdef SETUP_RAINGAUGE
	RAINGAUGE1,
#ifdef SETUP_RAINGAUGE_AUX
	RAINGAUGE2,
#endif
#endif
#ifdef	SETUP_CV7L
	CV7L,
#endif
#ifdef SETUP_MBXXXX
	MBXXXX,
#endif
#ifdef SETUP_VP61
	VP61,
#endif
#ifdef SETUP_L8095N
	L8095N,
#endif
#ifdef SETUP_L8095N_SEC
	L8095N_SEC,
#endif
#ifdef	SETUP_LB150
	LB150,
#endif
#ifdef	SETUP_PM10QBIT
	PM10QBIT,
#endif
#ifdef SETUP_SENSIT
	SENSIT,
#endif
#ifdef SETUP_CAP
	CAP_PRODUCER,
#endif
#ifdef SETUP_FMX167
	FMX167,
#endif
#ifdef SETUP_GPIO2LOG
	GPIO2LOG,
#endif
#ifdef SETUP_PANEL
	PANEL,
#endif
#ifdef SETUP_T023_MODBUS
	T023_MODBUS,
#endif
	DL_MODULE_END
} MODULE_ID;


typedef struct DB_RECORD {

	uint8_t flags;
	DL_INTERNAL_DATA dl_data;

#ifdef SETUP_RAINGAUGE
#ifdef SETUP_RAINGAUGE_AUX
RAINGAUGE_DATA raingauge_data[2];
#else
RAINGAUGE_DATA raingauge_data[1];
#endif
#endif

#ifdef SETUP_CV7L
	CV7L_DATA	cv7l_data;
#endif

#ifdef SETUP_LB150
	LB150_DATA	lb150_data;
#endif

#ifdef SETUP_MBXXXX
	MBXXXX_DATA	mbXXXX_data;
#endif

#ifdef SETUP_VP61
	VP61_DATA	vp61_data;
#endif

#ifdef SETUP_L8095N
	L8095N_DATA	l8095n_data;
#endif
#ifdef SETUP_L8095N_SEC
	L8095N_DATA	l8095n_sec_data;
#endif
#ifdef SETUP_PM10QBIT
	PM10QBIT_DATA	pm10qbit_data;
#endif
#ifdef SETUP_SENSIT
	SENSIT_STATS	sens_it_stats;
#endif
#ifdef SETUP_FMX167
	FMX167_DATA fmx_data;
#endif
#ifdef SETUP_PANEL
	PANEL_DATA panel_data;
#endif
#ifdef SETUP_T023_MODBUS
	T023_DATA t023_data;
#endif
} __attribute__((packed)) DB_RECORD;

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




static const __flash MODULE_INTERFACE iface_module[] = {
	
////////////////////////////////////////////////////////////////////////////////////
// DATALOGGER MODULE INTERFACE
////////////////////////////////////////////////////////////////////////////////////	
															{	NULL,
																dl_enable,
																NULL,
																NULL,
																dl_reset_data,
																( GETDATA )		dl_get_data,
																( DATA2STRING ) dl_Data2String,
#ifdef RMAP_SERVICES
																NULL
#endif	//RMAP_SERVICES
															},

////////////////////////////////////////////////////////////////////////////////////
// RAINGAUGE MODULE INTERFACE
////////////////////////////////////////////////////////////////////////////////////

#ifdef SETUP_RAINGAUGE
															{	raingauge_init,
																NULL,
																NULL,
																NULL,
																raingauge_reset_data,
																( GETDATA )		raingauge_get_data,
																( DATA2STRING ) raingauge_Data2String,
#ifdef RMAP_SERVICES
																( DATA2STRINGRMAP ) raingauge_Data2String_RMAP
#endif //RMAP_SERVICES
															 },
#ifdef SETUP_RAINGAUGE_AUX
															{	raingauge_init,
																NULL,
																NULL,
																NULL,
																raingauge_reset_data_aux,
																( GETDATA )		raingauge_get_data_aux,
																( DATA2STRING )	raingauge_Data2String_aux,
#ifdef RMAP_SERVICES
																( DATA2STRINGRMAP ) raingauge_Data2String_RMAP_aux
#endif //RMAP_SERVICES
															},
#endif //SETUP_RAINGAUGE_AUX
#endif //SETUP_RAINGAUGE

////////////////////////////////////////////////////////////////////////////////////
// LB150 MODULE INTERFACE
////////////////////////////////////////////////////////////////////////////////////

#ifdef SETUP_LB150
															{	LB150_init,
																LB150_enable,
																LB150_disable,
																LB150_Yield,
																LB150_reset_data,
																( GETDATA )		LB150_get_data,
																( DATA2STRING )	LB150_Data2String,
#ifdef RMAP_SERVICES
																NULL
#endif //RMAP_SERVICES
															},
#endif //SETUP_LB150

////////////////////////////////////////////////////////////////////////////////////
// MBXXXX MODULE INTERFACE
////////////////////////////////////////////////////////////////////////////////////

#ifdef SETUP_MBXXXX
															{	MBXXXX_init,
																MBXXXX_enable,
																MBXXXX_disable,
																MBXXXX_Yield,
																MBXXXX_reset_data,
																( GETDATA )		MBXXXX_get_data,
																( DATA2STRING )	MBXXXX_Data2String,	
#ifdef RMAP_SERVICES
																NULL
#endif //RMAP_SERVICES
															},
#endif //SETUP_MBXXXX

////////////////////////////////////////////////////////////////////////////////////
// CV7L MODULE INTERFACE
////////////////////////////////////////////////////////////////////////////////////

#ifdef SETUP_CV7L
															{	CV7L_init,
																CV7L_enable,
																CV7L_disable,
																CV7L_Yield,
																( RESETDATA )   CV7L_reset_data,
																( GETDATA )		CV7L_get_data,
																( DATA2STRING )	CV7L_Data2String,	
#ifdef RMAP_SERVICES
																NULL
#endif //RMAP_SERVICES
															},
#endif //SETUP_CV7L

////////////////////////////////////////////////////////////////////////////////////
// L8095N MODULE INTERFACE
////////////////////////////////////////////////////////////////////////////////////

#ifdef SETUP_L8095N
															{	l8095n_init,
																l8095n_enable,
																l8095n_disable,
																l8095n_Yield,
																( RESETDATA )   l8095n_reset_data,
																( GETDATA )		l8095n_get_data,
																( DATA2STRING )	l8095n_Data2String,	
#ifdef RMAP_SERVICES
																( DATA2STRINGRMAP ) l8095n_Data2String_RMAP
#endif //RMAP_SERVICES
															},
#endif //SETUP_L8095N

////////////////////////////////////////////////////////////////////////////////////
// L8095N MODULE INTERFACE
////////////////////////////////////////////////////////////////////////////////////

#ifdef SETUP_L8095N_SEC
															{	l8095n_sec_init,
																l8095n_sec_enable,
																l8095n_sec_disable,
																l8095n_sec_Yield,
																( RESETDATA )   l8095n_sec_reset_data,
																( GETDATA )		l8095n_sec_get_data,
																( DATA2STRING )	l8095n_sec_Data2String,
#ifdef RMAP_SERVICES
																( DATA2STRINGRMAP ) l8095n_sec_Data2String_RMAP
#endif //RMAP_SERVICES
															},
#endif //SETUP_L8095N

////////////////////////////////////////////////////////////////////////////////////
// FMX167 MODULE INTERFACE
////////////////////////////////////////////////////////////////////////////////////


#ifdef SETUP_FMX167
															{	fmx167_init,
																fmx167_enable,
																fmx167_disable,
																fmx167_Yield,
																fmx167_reset_data,
																( GETDATA )		fmx167_get_data,
																( DATA2STRING )	fmx167_Data2String,	
#ifdef RMAP_SERVICES
																NULL
#endif //RMAP_SERVICES
															},
#endif //SETUP_FMX167

////////////////////////////////////////////////////////////////////////////////////
// VP61 MODULE INTERFACE
////////////////////////////////////////////////////////////////////////////////////

#ifdef SETUP_VP61
															{	vp61_init,
																vp61_enable,
																vp61_disable,
																NULL,
																vp61_reset_data,
																( GETDATA )		vp61_get_data,
																( DATA2STRING )	vp61_Data2String,
#ifdef RMAP_SERVICES
																( DATA2STRINGRMAP ) vp61_Data2String_RMAP
#endif //RMAP_SERVICES
															},
#endif //SETUP_VP61

////////////////////////////////////////////////////////////////////////////////////
// PM10QBIT MODULE INTERFACE
////////////////////////////////////////////////////////////////////////////////////

#ifdef SETUP_PM10QBIT
															{	PM10QBIT_init,
																PM10QBIT_enable,
																PM10QBIT_disable,
																PM10QBIT_Yield,
																PM10QBIT_reset_data,
																( GETDATA )		PM10QBIT_get_data,
																( DATA2STRING )	PM10QBIT_Data2String,	
#ifdef RMAP_SERVICES
																NULL
#endif //RMAP_SERVICES
															},
#endif //SETUP_PM10QBIT

////////////////////////////////////////////////////////////////////////////////////
// SENSIT MODULE INTERFACE
////////////////////////////////////////////////////////////////////////////////////

#ifdef SETUP_SENSIT
															{	sens_it_init,
																sens_it_enable,
																sens_it_disable,
																sens_it_Yield,
																sens_it_reset_data,
																( GETDATA )		sens_it_get_data,
																( DATA2STRING )	sens_it_Data2String,	
#ifdef RMAP_SERVICES
																NULL
#endif //RMAP_SERVICES
															},
#endif //SETUP_SENSIT

////////////////////////////////////////////////////////////////////////////////////
// CAP MODULE INTERFACE
////////////////////////////////////////////////////////////////////////////////////

#ifdef SETUP_CAP
															{	CAP_init,
																CAP_enable,
																CAP_disable,
																CAP_Yield,
																NULL,
																NULL,
																NULL,
#ifdef RMAP_SERVICES
																NULL
#endif //RMAP_SERVICES
															},

#endif //SETUP_CAP

#ifdef SETUP_GPIO2LOG
															{	gpio2log_init,
																NULL,
																NULL,
																gpio2log_Yield,
																NULL,
																NULL,
																NULL,
																#ifdef RMAP_SERVICES
																NULL
																#endif //RMAP_SERVICES
															},

#endif //SETUP_GPIO2LOG

#ifdef SETUP_PANEL
															{	panel_init,
																NULL,
																NULL,
																NULL,
																panel_reset_data,
																( GETDATA )		panel_get_data,
																( DATA2STRING )	panel_Data2String,
																#ifdef RMAP_SERVICES
																NULL
																#endif //RMAP_SERVICES
															},

#endif //SETUP_PANEL

#ifdef SETUP_T023_MODBUS
															{	t023_init,
																NULL,
																NULL,
																t023_Yield,
																t023_reset_data,
																( GETDATA )		t023_get_data,
																( DATA2STRING )	t023_Data2String,
#ifdef RMAP_SERVICES
																NULL
#endif //RMAP_SERVICES
															},
#endif
};
	

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
#ifdef SETUP_VP61
	vp61_periodic();
#endif
#ifdef SETUP_T023_MODBUS
	t023_periodic();
#endif
}


#endif /* DL_INTERNAL_CONFIGS_H_ */