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


#ifndef MODINST_H_
#define MODINST_H_

#include "boost/preprocessor.hpp"

#define DECLA_INCLUDE0(path) \#include path
#define DECLA_INCLUDE(a,d,e) DECLA_INCLUDE0(BOOST_PP_TUPLE_ELEM(2,0,e))


BOOST_PP_SEQ_FOR_EACH(DECLA_INCLUDE,_,MODULE_DECLARATION)


#define DECLA(z,n,c) c##n



#define DECLA_RECORD(z,n,c) c##n

typedef struct DB_RECORD {

	uint8_t flags;
	DL_INTERNAL_DATA dl_data;

#ifdef SETUP_RAINGAUGE
	BOOST_PP_ENUM( SETUP_RAINGAUGE, DECLA_RECORD, (RAINGAUGE_DATA raingauge_data) )
#endif

#ifdef SETUP_CV7L
	BOOST_PP_ENUM( SETUP_CV7L, DECLA_RECORD, (CV7L_DATA cv7l_data) )
#endif

#ifdef SETUP_LB150
	BOOST_PP_ENUM( SETUP_LB150, DECLA_RECORD, (LB150_DATA lb150_data) )
#endif

#ifdef SETUP_MBXXXX
	BOOST_PP_ENUM( SETUP_MBXXXX, DECLA_RECORD, (MBXXXX_DATA mbXXXX_data) )
#endif

#ifdef SETUP_VP61
	BOOST_PP_ENUM( SETUP_VP61, DECLA_RECORD, (VP61_DATA vp61_data) )
#endif

#ifdef SETUP_L8095N
	BOOST_PP_ENUM( SETUP_L8095N, DECLA_RECORD, (L8095N_DATA l8095n_data) )
#endif
#ifdef SETUP_PM10QBIT
	BOOST_PP_ENUM( SETUP_PM10QBIT, DECLA_RECORD, (PM10QBIT_DATA	pm10qbit_data) )
#endif
#ifdef SETUP_SENSIT
	BOOST_PP_ENUM( SETUP_SENSIT, DECLA_RECORD, (SENSIT_DATA sens_it_stats) )
#endif
#ifdef SETUP_FMX167
	BOOST_PP_ENUM( SETUP_FMX167, DECLA_RECORD, (FMX167_DATA fmx_data) )
#endif
#ifdef SETUP_PANEL
	BOOST_PP_ENUM( SETUP_PANEL, DECLA_RECORD, (PANEL_DATA panel_data) )
#endif
#ifdef SETUP_T023_MODBUS
	BOOST_PP_ENUM( SETUP_T023_MODBUS, DECLA_RECORD, (T023_DATA t023_data) )
#endif
#ifdef SETUP_T026_MODBUS
	BOOST_PP_ENUM( SETUP_T026_MODBUS, DECLA_RECORD, (T026_DATA t026_data) )
#endif
#ifdef SETUP_T056_MODBUS
	BOOST_PP_ENUM( SETUP_T056_MODBUS, DECLA_RECORD, (T056_DATA t056_data) )
#endif
#ifdef SETUP_HD3910_MODBUS
	BOOST_PP_ENUM( SETUP_T056_MODBUS, DECLA_RECORD, (HD3910_DATA hd3910_data) )
#endif
} __attribute__((packed)) DB_RECORD;



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
	#ifdef SETUP_T026_MODBUS
	{	t026_init,
		NULL,
		NULL,
		t026_Yield,
		t026_reset_data,
		(TODO)		t026_get_data,
		( DATA2STRING )	t026_Data2String,
		#ifdef RMAP_SERVICES
		NULL
		#endif //RMAP_SERVICES
	},
	#endif
	#ifdef SETUP_T056_MODBUS
	{	t056_init,
		NULL,
		NULL,
		t056_Yield,
		t056_reset_data,
		( GETDATA )		t056_get_data,
		( DATA2STRING )	t056_Data2String,
		#ifdef RMAP_SERVICES
		NULL
		#endif //RMAP_SERVICES
	},
	#endif
	#ifdef SETUP_HD3910_MODBUS
	{	hd3910_init,
		NULL,
		NULL,
		hd3910_Yield,
		hd3910_reset_data,
		( GETDATA )		hd3910_get_data,
		( DATA2STRING )	hd3910_Data2String,

		#ifdef RMAP_SERVICES
		NULL
		#endif //RMAP_SERVIC
	},
	#endif

};


#endif /* MODINST_H_ */