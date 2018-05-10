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


#ifndef MODCONF_H_
#define MODCONF_H_

//#define MOD_ID_DATALOGGER 0
#define MOD_ID_PANEL 0
#define MOD_ID_RAINGAUGE 1
#define MOD_ID_CV7L 2
#define MOD_ID_MBXXXX 3
#define MOD_ID_VP61 4
#define MOD_ID_L8095N 5
#define MOD_ID_LB150 6
#define MOD_ID_PM10QBIT 7
#define MOD_ID_SENSIT 8
#define MOD_ID_FMX167 9
#define MOD_ID_GPIO2LOG 10
#define MOD_ID_T023_MODBUS 11
#define MOD_ID_T026_MODBUS 12
#define MOD_ID_T056_MODBUS 13
#define MOD_ID_HD3910_MODBUS 14

#define MODULE_REGISTRY ((MOD_ID_PANEL,PANEL,"Acronet/Sensors/PANEL/panel.h"))\
						((MOD_ID_RAINGAUGE,RAINGAUGE,"Acronet/Sensors/raingauge/pulse_raingauge.h"))\
						((MOD_ID_CV7L,CV7L,"Acronet/Sensors/CV7L/CV7L.h"))\
						((MOD_ID_MBXXXX,MBXXXX,"Acronet/Sensors/MBXXXX/MBXXXX.h"))\
						((MOD_ID_VP61,VP61,"Acronet/Sensors/VP61/vp61.h"))\
						((MOD_ID_L8095N,L8095N,"Acronet/Sensors/L8095-N/L8095-N.h"))\
						((MOD_ID_LB150,LB150,"Acronet/Sensors/LB150/LB150.h"))\
						((MOD_ID_PM10QBIT,PM10QBIT,"Acronet/Sensors/PM10Qbit/PM10Qbit.h"))\
						((MOD_ID_SENSIT,SENSIT,"Acronet/Sensors/SENS-IT/sens-it.h"))\
						((MOD_ID_FMX167,FMX167,"Acronet/Sensors/FMX167/fmx167.h"))\
						((MOD_ID_GPIO2LOG,GPIO2LOG,"Acronet/Sensors/GPIO2LOG/gpio2log.h"))\
						((MOD_ID_T023_MODBUS,T023,"Acronet/Sensors/SIAP_MICROS/t023/t023.h"))\
						((MOD_ID_T026_MODBUS,T026,"Acronet/Sensors/SIAP_MICROS/t026/t026.h"))\
						((MOD_ID_T056_MODBUS,T056,"Acronet/Sensors/SIAP_MICROS/t056/t056.h"))\
						((MOD_ID_HD3910_MODBUS,HD3910,"Acronet/Sensors/DELTAOHM/HD3910/hd3910.h"))
							

#ifdef RMAP_SERVICES
#define MODULE_SIZE_OF_INTERFACE 8
#else
#define MODULE_SIZE_OF_INTERFACE 7
#endif


#endif /* MODCONF_H_ */