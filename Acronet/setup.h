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



#ifndef SETUP_H_
#define SETUP_H_


//STATION_SETUP
#if defined(STATION_SETUP_BLIGNY)
#include "Acronet/utils/station_setup/bligny.h"
#elif defined(STATION_SETUP_BRANCA)
#include "Acronet/utils/station_setup/branca.h"
#elif defined(STATION_SETUP_BONINI)
#include "Acronet/utils/station_setup/bonini.h"
#elif defined(STATION_SETUP_CADORNA)
#include "Acronet/utils/station_setup/cadorna.h"
#elif defined(STATION_SETUP_CIABREA)
#include "Acronet/utils/station_setup/ciabrea.h"
#elif defined(STATION_SETUP_DEPANIS)
#include "Acronet/utils/station_setup/depanis.h"
#elif defined(STATION_SETUP_LEGINO)
#include "Acronet/utils/station_setup/legino.h"
#elif defined(STATION_SETUP_MAGLIOTTO)
#include "Acronet/utils/station_setup/magliotto.h"
#elif defined(STATION_SETUP_MARCONI)
#include "Acronet/utils/station_setup/marconi.h"
#elif defined(STATION_SETUP_MEMORIA)
#include "Acronet/utils/station_setup/memoria.h"
#elif defined(STATION_SETUP_MEMORIA_PLUS)
#include "Acronet/utils/station_setup/memoria_plus.h"
#elif defined(STATION_SETUP_RAVONE)
#include "Acronet/utils/station_setup/ravone.h"
#elif defined(STATION_SETUP_SANPIETRO)
#include "Acronet/utils/station_setup/sanpietro.h"
#elif defined(STATION_SETUP_CAPOFIUME)
#include "Acronet/utils/station_setup/capofiume.h"
#elif defined(STATION_SETUP_IDROBASE_TEST)
#include "Acronet/utils/station_setup/idrobase_test.h"
#elif defined(STATION_SETUP_HEADWIND)
#include "Acronet/utils/station_setup/headwind.h"
#elif defined(STATION_SETUP_RMAP_GHIGI)
#include "Acronet/utils/station_setup/ghigi_rmap.h"
#elif defined(STATION_SETUP_PANEL)
#include "Acronet/utils/station_setup/panel.h"
#elif defined(STATION_SETUP_MONESI)
#include "Acronet/utils/station_setup/monesi.h"
#elif defined(STATION_SETUP_REACH)
#include "Acronet/utils/station_setup/reach.h"
#elif defined(STATION_SETUP_ZEUS)
#include "Acronet/utils/station_setup/zeus.h"
#elif defined(STATION_SETUP_MOGGIE)
#include "Acronet/utils/station_setup/moggie.h"
#elif defined(STATION_SETUP_ROSSELLI)
#include "Acronet/utils/station_setup/rosselli.h"

#else 
#pragma message "No configuration defined, using defaults"

#define SETUP_RAINGAUGE
//#define SETUP_LB150
//#define SETUP_MBXXXX
//#define SETUP_PM10QBIT
//#define SETUP_L8095N
//#define SETUP_CV7L
//#define SETUP_FMX167
//#define SETUP_SENSIT
//#define SETUP_VP61

//#define SETUP_CAP_RAIN
//#define SETUP_CAP_LEVEL


#endif //STATION_SETUP

//#ifndef SP336_MODE
//#pragma message "[WARNING] SP336_MODE not defined, using defaults"
//#define SP336_MODE		SP336_MODE_HIGHZ
//#endif
//
//#ifndef SP336_2_MODE
//#pragma message "[WARNING] SP336_2_MODE not defined, using defaults"
//#define SP336_2_MODE	SP336_2_MODE_HIGHZ
//#endif

#define SP336_SELECTED_SETUP       (SP336_MODE | SP336_2_MODE)

#endif /* SETUP_H_ */