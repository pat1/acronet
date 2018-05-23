/*
 * ACRONET Project
 * http://www.acronet.cc
 *
 * Copyright ( C ) 2014 Acrotec srl
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the EUPL v.1.1 license.  See http://ec.europa.eu/idabc/eupl.html for details.
 *
 * 
 * 
 */

#include "Acronet/setup.h"
#include "Acronet/HAL/hal_interface.h"

#include <asf.h>
#include <stdio.h>
#include "progmem.h"
#include <conf_board.h>
#include <conf_usart_serial.h>

#include "config/conf_usart_serial.h"

#include "Acronet/globals.h"
#include "Acronet/services/config/config.h"
#include "Acronet/drivers/SIM/sim900.h"

#include "Acronet/drivers/UART_INT/cbuffer_usart.h"
#include "Acronet/drivers/StatusLED/status_led.h"
#include "Acronet/drivers/AT24CXX/AT24CXX.h"

#include "Acronet/drivers/PCAL9535A/PCAL9535A.h"
#include "Acronet/drivers/Voltmeter/voltmeter.h"
#include "Acronet/drivers/PowerSwitch/powerswitch.h"
#include "Acronet/drivers/SP336/SP336.h"

#include "Acronet/datalogger/datalogger.h"
//#include "utils/AnyType.h"

#include "Acronet/utils/MQTT/MQTTPacket.h"
#include "Acronet/utils/MQTT/MQTTConnect.h"
#include "Acronet/utils/MQTT/MQTTPublish.h"


#include "Acronet/services/DB/DB.h"
#include "Acronet/services/LOG/LOG.h"

#include "Acronet/services/fw_update/fw_update.h"

#include "Acronet/services/CAP/cap_common.h"
#include "Acronet/services/CAP/cap_producer.h"
#include "Acronet/services/CAP/cap_consumer.h"
#include "Acronet/services/taskman/taskman.h"

#define DL_SEND_FLAGS_ONCE		0x01
#define DL_SEND_FLAGS_CONNECTED 0x02

typedef struct DL_SEND_PARAMS{
	AT24CXX_iterator iter_send;
	uint32_t dt_end;
	uint8_t  send_flags;
	
} DL_SEND_PARAMS;



static RET_ERROR_CODE dl_MQTTconnData_init(char * const heap,uint16_t lenHeap);
static RET_ERROR_CODE dl_MQTT_tcp_open(uint16_t * const TCPBufLen);
static RET_ERROR_CODE dl_MQTT_data_transfer_prepare(void);
static RET_ERROR_CODE dl_MQTT_send( const uint8_t tcp_cid, char * const TCPBuf, const uint16_t lenTCPBuf,const uint16_t numMQTTmessages, uint16_t * const pQOSACK );
static RET_ERROR_CODE dl_MQTT_sendFromDatastore( const uint8_t dataStoreID );
static RET_ERROR_CODE send_log_with_MQTT( MQTTPacket_connectData * const pData ,const uint16_t MSGMAXLEN);
static RET_ERROR_CODE send_data_with_MQTT( DL_SEND_PARAMS * const pPara, MQTTPacket_connectData * const pData, const uint16_t MSGMAXLEN);


#define DL_T0 1350923400

enum {DATASTORE_LOG = 0,DATASTORE_DB_RT = 1,DATASTORE_DB = 2};
//#define DATASTORE_LOG   ((uint8_t) 0)
//#define DATASTORE_DB_RT ((uint8_t) 1)
//#define DATASTORE_DB    ((uint8_t) 2)




bool dl_flush_buffers(void);

//#ifdef RMAP_SERVICES
//
//static RET_ERROR_CODE dl_Data2String(	  const DL_INTERNAL_DATA * const dt
										//, const uint32_t timeStamp
										//, const uint16_t timeWindow
										//, char * const sz
										//, int16_t * len_sz )
//{
	//return AC_ERROR_OK;
//}
//
//
//#endif


static void dl_enable(void)
{
	char szBUF[128];	
	szBUF[0] = 0;
	strcat_P(szBUF,PSTR("BUILD VERSION : "));
	strcat_P(szBUF,g_szGIT_TAG);
	LOG_say(szBUF);
	szBUF[0] = 0;
	strcat_P(szBUF,PSTR("FW CONFIGURATION : "));
	strcat_P(szBUF,g_szPRJ_TAG);
	LOG_say(szBUF);
	
}

#include "dl_internal_configs.h"


#ifdef RMAP_SERVICES

typedef struct DL_RMAP_SEND_PARAMS{
	DB_RECORD ds;
	DB_ITERATOR iter_send;
	uint32_t dt_end;
	uint8_t  send_flags;
	
} DL_RMAP_SEND_PARAMS;

static RET_ERROR_CODE send_data_with_RMAP( DL_RMAP_SEND_PARAMS * const pPara, MQTTPacket_connectData * const pData, const uint16_t MSGMAXLEN);
#endif


//TASKS
volatile uint8_t task_make_statistics = TASK_STOP;

volatile uint8_t task_status_server_cmd_check = TASK_STOP;
volatile uint8_t task_status_send_data_prepare_RT = TASK_STOP;
volatile uint8_t task_status_send_data_RT = TASK_STOP;
volatile uint8_t task_status_store_data = TASK_STOP;
volatile uint8_t task_status_sync_time = TASK_STOP;
//volatile uint8_t task_status_time_tick = TASK_STOP;

volatile uint8_t task_status_send_data_prepare = TASK_STOP;
volatile uint8_t task_status_send_data = TASK_STOP;

volatile uint8_t task_status_send_log = TASK_STOP;


#ifdef ENABLE_CAP
volatile uint8_t task_status_CAP_evaluate = TASK_STOP;
volatile uint8_t task_status_CAP_send = TASK_STOP;
#endif

#ifdef RMAP_SERVICES
volatile uint8_t task_status_send_data_RMAP = TASK_STOP;
#endif

//EVENTS
volatile uint8_t dl_cycle_lock = true;
//volatile uint8_t dl_event_terminate_send = false;



static DL_SEND_PARAMS dl_send_params_RT;
static DL_SEND_PARAMS dl_send_params;

#ifdef RMAP_SERVICES
static DL_RMAP_SEND_PARAMS dl_send_params_RMAP;
#endif

#define POINTER_OK true
#define POINTER_INVALID false


//Datalogger scheduler time accumulators
static volatile int32_t g_acc_task_make_statistics = 0;
static volatile int32_t g_acc_task_sync = 0;
static volatile int32_t g_acc_task_snapshot = 0;
static volatile int32_t g_acc_task_store = 0;
static volatile int32_t g_acc_task_send_RT = 280;



typedef struct DL_TIMINGS {
	uint32_t task_store;
	uint32_t task_send_rt;
	uint32_t task_sync;
} DL_TIMINGS;

static DL_TIMINGS g_timing;

//Datalogger timer interval is set as 5 seconds
static uint32_t g_timespan = 5;

static uint32_t g_timePrev = DL_T0;

//static RET_ERROR_CODE send_data_with_get(DL_SEND_PARAMS * const pPara);
static RET_ERROR_CODE send_data_with_post(DL_SEND_PARAMS * const pPara);

static void dl_test_now(void);

static uint8_t  dl_snapshot_make(const uint32_t ts);
static void		dl_snapshot_init(const uint32_t ts);

static RET_ERROR_CODE dl_task_sync_time( void );


static void dl_get_data(DB_RECORD * const ps)
{
	
}

static void dl_reset_data(void)
{
	
}

static RET_ERROR_CODE dl_Data2String(    const DB_RECORD * const st ,char * const sz , size_t * len_sz )
{
	
	#ifdef MQTT_AS_PRIMARY
	size_t len = snprintf_P(	sz,*len_sz ,PSTR("TIME=%lu") ,st->data_timestamp );
	#else
	size_t len = snprintf_P(	sz,*len_sz ,PSTR("#TIME=%lu") ,st->data_timestamp );
	#endif
	
	
	const RET_ERROR_CODE e = (len < *len_sz) ? AC_ERROR_OK : AC_BUFFER_OVERFLOW;
	*len_sz = len;
	return e;

}


//static void dl_test_psw(void)
//{
	////static uint8_t v = 0;
	////hal_psw_set((v++) & 0b00000111);
	////if (v == 16)	{ v = 0;}
//}
//
//////////////////////////////////////////////////////////////////////////
////
//// This function is used solely during the initialization
//// phase of the datalogger [this one is used by the alarm version]
////
//static void dl_startup_sync(uint32_t timeNow)
//{
	////unlock the spin cycle of the process manager
	//dl_cycle_lock = false;
//}

////////////////////////////////////////////////////////////////////////
//
// This function is used solely during the initialization
// phase of the datalogger [this one is used by the overflow version]
//
static void dl_startup_sync2(uint32_t timeNow)
{
	hal_status_led_toggle();

	//unlock the spin cycle of the process manager
	if(timeNow >= g_timespan)
		dl_cycle_lock = false;
}


static void dl_tick(uint32_t timeNow)
{
//	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"DATALOGGER_TICK");

//	hal_rtc_set_alarm(timeNow+g_timespan);
	wdt_reset();

	const int32_t deltaT = (timeNow - g_timePrev);
	g_timePrev = timeNow;

	g_acc_task_make_statistics += deltaT;
	if((TASK_STOP==task_make_statistics) && (g_acc_task_make_statistics>=60)) {
		g_acc_task_make_statistics = 0;
		task_make_statistics=TASK_READY;
	}

	
	g_acc_task_sync += deltaT;
	if((TASK_STOP==task_status_sync_time) && (g_acc_task_sync>=g_timing.task_sync)) {
		g_acc_task_sync = 0;
		task_status_sync_time=TASK_READY;
	}
	
	g_acc_task_snapshot += deltaT;
	if (g_acc_task_snapshot>=g_timing.task_store)
	{
		g_acc_task_snapshot = 0;//timeNow % 60; //keep the task scheduled at the minute
		//Snapshot of the memory that should be archived by the datalogger
		//The slow EEPROM writing procedure goes in the main loop
		dl_snapshot_make(timeNow);
	}

	g_acc_task_store += deltaT;
	if((TASK_STOP==task_status_sync_time) && (g_acc_task_store>=g_timing.task_store)) {
		g_acc_task_store = 0;
		task_status_store_data=TASK_READY;
	}

	g_acc_task_send_RT += deltaT;
	if((TASK_STOP==task_status_send_data_RT) && (g_acc_task_send_RT>=g_timing.task_send_rt))
	{
		g_acc_task_send_RT = 0;
		task_status_send_data_prepare_RT=TASK_READY;
	}
/*
	g_acc_task_cap_eval += deltaT;
	if((TASK_STOP==task_status_CAP_evaluate) && (g_acc_task_send_RT>=g_interv_task_send_RT))
	{
		g_acc_task_send_RT = 0;
		task_status_send_data_prepare_RT=TASK_READY;
	}
*/
	//If any of the tasks is getting unscheduled for too long
	//means that that task is hanged somewhere...
	//we may decide to handle this case doing a system reset


	if(g_acc_task_send_RT>(3*g_timing.task_send_rt)) {
		wdt_reset_mcu();
	}

	
	status_led_toggle();
	
	//unlock the spin cycle of the process manager
	dl_cycle_lock = false;

}


#include "Acronet/drivers/StatusLED/status_led.h"


RET_ERROR_CODE dl_init( void )
{

	char szBUF[128];

//	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"DL_INIT");


/*********************************************************************************/
// Why did we restart ? In the first 4 bytes of the external memory we may find 
// AC01 = Normal reboot
// UPDA = reboot after successful firmware update
// FAIL = reboot after an unsuccessfully update
// FWDW = reboot has occurred while downloading a firmware 
/*********************************************************************************/

	//hal_psw_init();

	DB_query_startup_code(szBUF);
	
	debug_string_1P(NORMAL,PSTR("\r\n\r\nIn the next 3 seconds, hit a key to erase DB\r\n"));

	uint16_t d = 3000;
	while(--d) {
		if(usart_rx_is_complete(USART_DEBUG)) {
			szBUF[0] = 'K';
			szBUF[1] = 'E';
			szBUF[2] = 'Y';
			szBUF[3] = '*';
			break;
		}
		delay_ms(1);
	}


	//if the eeprom has to be initialized
	if((szBUF[0]!='A') || (szBUF[1]!='C') || (szBUF[2]!='0') || (szBUF[3]!='1'))
	{
		szBUF[4] = 0;
		debug_string_1P(NORMAL,PSTR("GOT DB signature : "));
		debug_string(NORMAL,szBUF,RAM_STRING);
		//datalogger_reset_eeprom();
		DB_reset_eeprom();
		LOG_reset();
			
		//if((szBUF[0]=='U') && (szBUF[1]=='P') && (szBUF[2]=='D') && (szBUF[3]=='A'))
		//{
			//Restart after reprogramming
				
			wdt_set_timeout_period(WDT_TIMEOUT_PERIOD_8CLK);
			wdt_enable();
			wdt_reset_mcu();
			while(1) {delay_ms(200);status_led_toggle();}

		//}

	}


/*********************************************************************************/
// Check which version of the configuration file is present
// in case upgrade it 
/*********************************************************************************/
	

	if (AC_ERROR_OK != cfg_check())
	{
		if(true == cfg_old_check())
		{

			cfg_upgrade_from_old();	
			wdt_enable();
			wdt_reset_mcu();
			while(1) {delay_ms(200);status_led_toggle();}
		}
		
		debug_string_1P(NORMAL,PSTR("\r\n\r\n[ERROR] Configuration is missing, check the internal eeprom image\r\n"));
		return AC_ERROR_GENERIC;
	}


/*********************************************************************************/
// Get configuration values from the config file
/*********************************************************************************/

	CFG_ITEM_ADDRESS f;

	cfg_find_item(CFG_TAG_DATALOGGER_AWS_ID,&f);
	cfg_get_item_file(f,szBUF,sizeof(szBUF));
	
	debug_string(NORMAL,PSTR("\r\n\e[42;30mSTATION ID : "),PGM_STRING);
	debug_string(NORMAL,szBUF,RAM_STRING);

	//debug_string_P(NORMAL,PSTR("\r\nBUILD: " __TIMESTAMP__ ));


	cfg_find_item(CFG_TAG_DATALOGGER_TIMING,&f);
	cfg_get_item_file(f,&g_timing,sizeof(g_timing));

	sprintf_P(szBUF,PSTR("\e[49;39m\r\nDL_TASK_STORE_DT: %lu\r\n"),g_timing.task_store);
	debug_string(NORMAL,szBUF,false);
	sprintf_P(szBUF,PSTR("DL_TASK_SEND_DT: %lu\r\n"),g_timing.task_send_rt);
	debug_string(NORMAL,szBUF,false);
	sprintf_P(szBUF,PSTR("DL_TASK_SYNC_DT: %lu\r\n"),g_timing.task_sync);
	debug_string(NORMAL,szBUF,false);
	sprintf_P(szBUF,PSTR("DL_TIME_DT: %lu\r\n"),g_timespan);
	debug_string(NORMAL,szBUF,false);
	const uint32_t lll = sizeof(DB_RECORD);
	sprintf_P(szBUF,PSTR("sizeof(DL_LOG_ITEM) : %lu\r\n"),lll);
	debug_string(NORMAL,szBUF,false);


/*********************************************************************************/
// In case of reboot all the data that should be sent may be sent earlier
/*********************************************************************************/


	if(g_timing.task_send_rt>60) {
		g_acc_task_send_RT = g_timing.task_send_rt - 20;
	} else {
		g_acc_task_send_RT = 0;
	}


/*********************************************************************************/
// Configure UART port for SIM900 connection
/*********************************************************************************/

	const usart_rs232_options_t USART_SERIAL_OPTIONS = {
		.baudrate = USART_GPRS_BAUDRATE,
		.charlength = USART_CHAR_LENGTH,
		.paritytype = USART_PARITY,
		.stopbits = USART_STOP_BIT
	};

	usart_serial_init(USART_GPRS, &USART_SERIAL_OPTIONS);
	

/*********************************************************************************/
// Configure datalogger memory iterators
/*********************************************************************************/

	DB_init(sizeof(DB_RECORD));
	LOG_init();

	//AT24CXX_iterator b = { .plain = PARTITION_LOG_BEGIN };
	//AT24CXX_iterator e = { .plain = PARTITION_LOG_END };
	//DB_dump(b,e);
//
	//usart_rx_enable(USART_DEBUG);
	//usart_getchar(USART_DEBUG);

/*********************************************************************************/
// Configure the ADC Manager
/*********************************************************************************/

	//ADC_MAN_Init(g_tbl_ADC_MAN_SETUP,sizeof(g_tbl_ADC_MAN_SETUP)/sizeof(g_tbl_ADC_MAN_SETUP[0]));

/*********************************************************************************/
// Configure Introspection for CAP
/*********************************************************************************/

	//CAP_init(g_cap_introspection,sizeof(g_cap_introspection)/sizeof(g_cap_introspection[0]));

/*********************************************************************************/
//	SP336 Init
/*********************************************************************************/

	SP336_Init();

/*********************************************************************************/
//	Module Init 
/*********************************************************************************/

	//VBusMon_check();

	for(MODULE_ID im=DL_MODULE_BEG;im<DL_MODULE_END;++im)
	{
		INITMODULE fn_Init = iface_module[im].pFN_Init;

		if (NULL==fn_Init)
		{
			continue;
		}

		fn_Init();
	}

	dl_test_now();


/**************************/
//Connection to Acronetwork
/**************************/

	hal_rtc_set_time(DL_T0);


	uint8_t ii=0;
	while(1) {
		if(++ii==10) {
			wdt_set_timeout_period(WDT_TIMEOUT_PERIOD_8CLK);
			wdt_enable();
			wdt_reset_mcu();
			delay_ms(1000);
		}
		if(AC_ERROR_OK==sim900_init()) {
			break;
		}
	}

	
	//	g_is_GPRS_module_OK = true;
	ii=0;
	while(1) {
		statusled_blink(1);
		if(AC_ERROR_OK==sim900_GPRS_check_line())
		{
			break;
		}

		delay_ms(5000);
		
		if(++ii>24) {
			wdt_set_timeout_period(WDT_TIMEOUT_PERIOD_8CLK);
			wdt_enable();
			wdt_reset_mcu();
			delay_ms(1000);
		}
	}

/**************************************/
//TODO: only if needed during debug
/**************************************/
//	dl_test_now();


	ii=0;
	const uint32_t t=g_timespan;
	g_timespan = 0;

	
	while(1) {
		if(++ii==10) {
			wdt_set_timeout_period(WDT_TIMEOUT_PERIOD_8CLK);
			wdt_enable();
			wdt_reset_mcu();
			delay_ms(1000);
		}
		if( AC_ERROR_OK == dl_task_sync_time() ) {
			//we got the time sync and we stop asking it...
			break;
		}
		delay_ms(1000);
	}

	const uint32_t rtc = g_timespan;
	
	if (g_timespan==0)
	{
		debug_string_1P(NORMAL,PSTR("Unable to sync with the server\r\n"));
		delay_ms(1000);
		powerSwitch_toggle();
	}

	//If the synctime succeded we should have a syncronization
	//time that is the value that the synctime is passing to us
	//in the g_timespan variable
	hal_rtc_set_period_cb(dl_startup_sync2);
//	hal_rtc_set_alarm(g_timespan);

	debug_string_1P(NORMAL,PSTR("\r\nSYNC set at "));
	ultoa(g_timespan,szBUF,10);
	debug_string(NORMAL,szBUF,RAM_STRING);
	debug_string_1P(NORMAL,PSTR("\r\n\r\n"));


	uint8_t i = 5;
	while (i-- != 0)
	{
		sprintf_P(szBUF,PSTR("%lu\r\n"),hal_rtc_get_time());
		debug_string(NORMAL,szBUF,RAM_STRING);
		delay_ms(1200);
	}

	dl_cycle_lock = true;

	while(dl_cycle_lock) {
		nop();
	}

	g_timespan = t;


/**********************************/
//Configure Snapshot and Watchdog
/**********************************/
	g_timePrev = rtc;
	dl_snapshot_init(rtc);

	hal_rtc_set_period_cb(dl_tick);


	wdt_set_timeout_period(WDT_TIMEOUT_PERIOD_8KCLK);
	wdt_enable();


/**************************/
//Module Enable
/**************************/

	for(MODULE_ID im=DL_MODULE_BEG;im<DL_MODULE_END;++im)
	{
		ENABLEMODULE fn_Enable = iface_module[im].pFN_Enable;

		if (NULL==fn_Enable)
		{
			continue;
		}

		fn_Enable();
	}

	
	return AC_ERROR_OK;

}

static RET_ERROR_CODE dl_task_sync_time( void )
{
	//TODO: consider a better time sync mechanism

	task_status_sync_time = TASK_RUNNING;
	
//	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"DL_TASK_SYNC_TIME");

	//if(!g_is_GPRS_module_OK) {
	//debug_string(NORMAL,PSTR("(datalogger_sync_time) the GPRS status flag is not OK, this task will finish here\r\n"),true);
	//err = 1;
	//goto quit_synctime_task;
	//}

	RET_ERROR_CODE err = sim900_bearer_simple_open();
	if(err!=AC_ERROR_OK) {
		goto quit_synctime_task;
	}

	char buf[128];
	//cfg_get_service_url_time(buf,128);
	
	CFG_ITEM_ADDRESS f;
	
	cfg_find_item(CFG_TAG_DATALOGGER_TIME_URL,&f);
	cfg_get_item_file(f,buf,128);

	uint16_t l = 30;
	err = sim900_http_get(buf,RAM_STRING,buf,&l,false);//TODO: il terzo parametro (isBinary) deve essere definito

	
	if(err!=AC_ERROR_OK) {
		goto quit_synctime_task;
	}

	if (l>sizeof(buf))
	{
		debug_string_1P(NORMAL,PSTR("[WARNING] buffer length value was wrong, aborting procedure\r\n"));
		err = AC_ERROR_GENERIC;
		goto quit_synctime_task;
	}


	const uint32_t tOLD = g_timePrev;
	const uint32_t tAct = hal_rtc_get_time();

	const uint32_t tNEW = strtoul(&buf[0],NULL,10);

	if (tOLD>tNEW) {
		g_timePrev = tNEW;
	}
	
	if (0==g_timespan)
	{
		hal_rtc_set_time(tNEW);

		uint32_t ts = 0;
		uint8_t i=0, j=0;

		for( ; (i<l) && (j<1); ++i) {
			if(buf[i]==';') j++;
		}
				
		if(j!=1) {
			debug_string(NORMAL,buf,RAM_STRING);
			debug_string_1P(NORMAL,PSTR("\r\n[WARNING] unable to get the sync , going with defaults\r\n"));
			} else {
			ts = atoi(&buf[i]);
		}
				
				
// 		debug_string_P(NORMAL,PSTR("SYNC is "));
// 		itoa(ts,buf,10);
// 		debug_string(NORMAL,buf,RAM_STRING);
// 		debug_string_P(NORMAL,PSTR(" seconds from now\r\n"));
				
		g_timespan = tNEW + ts;
	} else {
		const int32_t tdiff = tNEW-tAct;

		if( (tdiff > 1) || (tdiff < -1) ) {
			if(wdt_is_enabled()) {
				wdt_disable();
				hal_rtc_set_time(tNEW);
				wdt_enable();
			} else {
				hal_rtc_set_time(tNEW);
			}
		}
		//g_acc_task_snapshot += tdiff;
	}

	


// 	debug_string_P(VERBOSE,PSTR("(DL_synctime) service returned: "));
// 	debug_string  (VERBOSE,&buf[0],RAM_STRING);
// 	debug_string_P(VERBOSE,szCRLF);

	struct calendar_date adate;

	calendar_timestamp_to_date(tAct,&adate);
	snprintf_P(buf,sizeof(buf),PSTR("%02d/%02d/%d %02d:%02d:%02d\r\n"),adate.date+1,adate.month+1,adate.year,adate.hour,adate.minute,adate.second);
	debug_string(NORMAL,PSTR("(DL_synctime) Old date was: "),PGM_STRING);
	debug_string(NORMAL,buf,RAM_STRING);

	calendar_timestamp_to_date(tNEW,&adate);
	snprintf_P(buf,sizeof(buf),PSTR("%02d/%02d/%d %02d:%02d:%02d\r\n"),adate.date+1,adate.month+1,adate.year,adate.hour,adate.minute,adate.second);
	debug_string(NORMAL,PSTR("(DL_synctime) New date is: "),PGM_STRING);
	debug_string(NORMAL,buf,RAM_STRING);

	wdt_reset();


quit_synctime_task:

	sim900_http_close();
	sim900_bearer_simple_release();
	

	task_status_sync_time = TASK_STOP;
	return err;
}

//static uint8_t g_sendverb = NORMAL;

static RET_ERROR_CODE dl_VerifyDataXferFeasibility(void)
{
	RET_ERROR_CODE e = AT24CXX_Init();
	if(AC_ERROR_OK!=e)
	{
		debug_string_2P(NORMAL,PSTR("dl_VerifyDataXferFeasibility"),PSTR("AT24CXX init went wrong"));
		return e;
	}

	e = sim900_GPRS_check_line();
	if(AC_ERROR_OK!=e) {
		debug_string_2P(NORMAL,PSTR("dl_VerifyDataXferFeasibility"),PSTR("GPRS line is not OK"));
		return e;
	}
	
	return AC_ERROR_OK;
	
}

static RET_ERROR_CODE dl_HTTP_data_transfer_prepare(void)
{

	RET_ERROR_CODE e = dl_VerifyDataXferFeasibility();
	if(AC_ERROR_OK!=e)
	{
		debug_string_2P(NORMAL,PSTR("dl_HTTP_data_transfer_prepare"),PSTR("Unable to get data xfer feasibility"));
		return e;
	}

	return sim900_bearer_simple_open();
}

static RET_ERROR_CODE dl_MQTT_data_transfer_prepare(void)
{
	RET_ERROR_CODE e = dl_VerifyDataXferFeasibility();
	if(AC_ERROR_OK!=e)
	{
		////IF ERROR COMES FROM THE SIM900
		//if( (e>=AC_SIM900_ERROR_BEGIN_PLACEHOLDER) && (e<AC_SIM900_ERROR_END_PLACEHOLDER) )
		//{
			//sim900_power_off();
		//}
		debug_string_2P(NORMAL,PSTR("dl_HTTP_data_transfer_prepare"),PSTR("Unable to get data xfer feasibility"));
		return e;
	}

	return AC_ERROR_OK;
	
}

static RET_ERROR_CODE dl_task_send_data_prepare( void )
{
	task_status_send_data_prepare = TASK_RUNNING;
//	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"DL_TASK_SEND_DATA_PREPARE");

#ifdef MQTT_AS_PRIMARY
	const RET_ERROR_CODE r = dl_MQTT_data_transfer_prepare();
#else
	const RET_ERROR_CODE r = dl_HTTP_data_transfer_prepare();
#endif

	if(r==AC_ERROR_OK) {
		//g_sendverb = NORMAL;
		DB_iterator_get_end(&dl_send_params.iter_send);
		dl_send_params.dt_end = hal_rtc_get_time() - 86400;
		dl_send_params.send_flags = 0; //!connected & !sendOnce
	}

	task_status_send_data_prepare = TASK_STOP;
	return r;
}


static RET_ERROR_CODE dl_task_send_data_prepare_RT( void )
{
	task_status_send_data_prepare_RT = TASK_RUNNING;
//	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"DL_TASK_SEND_DATA_PREPARE_RT");

#ifdef MQTT_AS_PRIMARY
	const RET_ERROR_CODE r = dl_MQTT_data_transfer_prepare();
#else
	const RET_ERROR_CODE r = dl_HTTP_data_transfer_prepare();
#endif
	
	if(r==AC_ERROR_OK) {
		//g_sendverb = VERBOSE;
		DB_iterator_get_end(&dl_send_params_RT.iter_send);
		dl_send_params_RT.dt_end = hal_rtc_get_time() - 86400;
		dl_send_params_RT.send_flags = 0x01; //!connected & sendOnce
		//dl_send_params_RT.send_flags = 0x00;//!connected & !sendOnce

#ifdef RMAP_SERVICES
		DB_iterator_get_end(&dl_send_params_RMAP.iter_send);
		dl_send_params_RMAP.dt_end = hal_rtc_get_time() - 86400;
		dl_send_params_RMAP.send_flags = 0x01;//!connected & sendOnce
#endif //RMAP_SERVICES
	}

	task_status_send_data_prepare_RT = TASK_STOP;
	return r;
}




static uint8_t dl_compute_skip_length(const AT24CXX_iterator * const itBeg,const AT24CXX_iterator * const itEnd)
{
	uint32_t r = DB_iterator_distance(itBeg,itEnd) / sizeof(DB_RECORD);
	
	if(r>128) {
		return 128;	
	}
		
	//Taken from: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
	r |= r >> 1;
	r |= r >> 2;
	r |= r >> 4;
	r |= r >> 8;
	r |= r >> 16;
	r = r - (r>>1);
		
	return min(max(r,1),128);
}

enum DL_EEPROM_SEARCH_MODE {LEFT_BOUND,RIGHT_BOUND,EXACT_MATCH};
static RET_ERROR_CODE dl_search_record_by_date(uint32_t dt,enum DL_EEPROM_SEARCH_MODE search_mode,AT24CXX_iterator * pIter)
{
//	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"DL_SEARCH_RECORD_BY_DATE");

	char szBuf[128];

	RET_ERROR_CODE err = AC_ERROR_OK;

	if(pIter==NULL) {
		debug_string_2P( NORMAL
						,PSTR("dl_search_record_by_date") 
						,PSTR("parameter pIter is NULL") );
						
		err = AC_BAD_PARAMETER;
		goto quit_search;
	}
	
	DB_ITERATOR itBeg, itEnd;
	
	DB_iterator_get_begin(&itBeg);
	DB_iterator_get_end(&itEnd);
	
	//Datalogger is empty
	if(itBeg.plain==itEnd.plain) {
		debug_string_2P( NORMAL
						,PSTR("dl_search_record_by_date") 
						,PSTR("Datalogger is empty. Nothing to search") );
						
		err = AC_NOTHING_TO_DO;
		goto quit_search;
	}

	err = AT24CXX_Init();
	if(AC_ERROR_OK!= err)
	{
		debug_string_2P( NORMAL
						,PSTR("dl_search_record_by_date")
						,PSTR("AT24CXX init went wrong, aborting procedure") );
		goto quit_search;
	}

	//first check the last record
	DB_RECORD ds;
	AT24CXX_iterator it;
	DB_iterator_get_end(&it);

	
	const uint8_t s = dl_compute_skip_length(&itBeg,&itEnd);

	sprintf_P(szBuf,PSTR("SKIP LENGTH %d \r\n"),s);

	debug_string(NORMAL,szBuf,RAM_STRING);
	
	DB_iterator_moveback(&it,1);

	uint8_t reg_skip = s;
	uint8_t reg_dir  = 0; //0 = backward, 1= forward
	while(1){
		

		if(0!=DB_get_record(&it,&ds))
		{
			err = AC_ERROR_GENERIC;
			goto quit_search;
		}

		//if(g_log_verbosity>=VERBOSE) {
			//snprintf_P(  szBuf
						//,sizeof(szBuf)
						//,PSTR("Search called with dt=%lu found %lu\r\n"),dt,ds.dl_data.data_timestamp);
						//
			//debug_string(NORMAL,szBuf,RAM_STRING);
		//}
		
		if(reg_dir == 0) { //if we are searching backward
			if(ds.data_timestamp<dt) {
				//debug_string_1P(VERBOSE,PSTR("Going from backward to forward\r\n"));
				reg_skip >>= 1;
				reg_dir = 1;
			} else if(ds.data_timestamp==dt){
				break;
			}
		} else {//if we are searching forward
			if(ds.data_timestamp==dt){
				break;
			} else if(ds.data_timestamp>dt){
				//debug_string_1P(VERBOSE,PSTR("Going from forward to backward\r\n"));
				reg_skip >>= 1;
				reg_dir = 0;
			}
		}

		
		if(reg_skip==0) break;
		
		//Calculate the next search position
		if(reg_dir == 0) { //if we are going to search backward
			if (it.plain == itBeg.plain)
			{
				reg_dir = 1;
				continue;
			}
			uint8_t sr = min(reg_skip,dl_compute_skip_length(&itBeg,&it));
			DB_iterator_moveback(&it,sr);
		} else { //if we are going to search forward
			uint8_t sr = min(reg_skip,dl_compute_skip_length(&it,&itEnd));
			if (it.plain == itEnd.plain)
			{
				reg_dir = 0;
				continue;
			}

			DB_iterator_moveforward(&it,sr);
		}
	}

	//if(ds.dl_stats.data_timestamp<dt) {
		//if(search_mode==LEFT_BOUND){
			//err = AC_ERROR_GENERIC;
			//goto quit_search;
			//} else if(search_mode==EXACT_MATCH) {
			//err = AC_ERROR_GENERIC;
			//goto quit_search;
		//}
		////else if RIGHT bound we have a match
		//pIter->plain = it.plain;
		//} else if(ds.dl_stats.data_timestamp==dt){
		//pIter->plain = it.plain;
		//}else if(ds.dl_stats.data_timestamp>dt){
		//if(search_mode==RIGHT_BOUND){
			//err = AC_ERROR_GENERIC;
			//goto quit_search;
			//} else if(search_mode==EXACT_MATCH) {
			//err = AC_ERROR_GENERIC;
			//goto quit_search;
		//}
		////else if LEFT bound we have a match
		//pIter->plain = it.plain;
	//}

	pIter->plain = it.plain;

	if(g_log_verbosity>=NORMAL) {
		debug_string_1P(NORMAL,PSTR("\tRecord found "));
		AT24CXX_iterator_report(it);
		snprintf_P(szBuf,sizeof(szBuf),PSTR("\r\n\tRecord  dt=%lu\r\n"),ds.data_timestamp);
		debug_string(NORMAL,szBuf,RAM_STRING);
	}

	
	quit_search:

	return err;
}

static RET_ERROR_CODE dl_cmd_get_by_date_range(uint32_t dtBeg,uint32_t dtEnd)
{
//	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"DL_CMD_GET_BY_DATE_RANGE");

	AT24CXX_iterator itEnd = {0};
	RET_ERROR_CODE err = dl_search_record_by_date(max(dtBeg,dtEnd) ,RIGHT_BOUND,&itEnd);

	if(err == AC_ERROR_OK) {
		dl_send_params.iter_send = itEnd;
		dl_send_params.dt_end = min(dtBeg,dtEnd);
		dl_send_params.send_flags = 0x00;//!connected & !sendOnce
		task_status_send_data = TASK_READY;
		goto quit_cmd_get_by_date_range;
	}

	//We didn't find the requested record


	//////////////////////////////////////////////////////////////////////////
	quit_cmd_get_by_date_range:

	return err;
}


static void datalogger_stop(void)
{
	wdt_reset();
	wdt_disable();
	
	/**************************/
	//Module Disable
	/**************************/

	for(MODULE_ID im=DL_MODULE_BEG;im<DL_MODULE_END;++im)
	{
		DISABLEMODULE fn_Disable = iface_module[im].pFN_Disable;

		if (NULL==fn_Disable)
		{
			continue;
		}

		fn_Disable();
	}

	
	hal_rtc_set_callback(NULL);
	
}

static RET_ERROR_CODE dl_cmd_fw_update(const char * pPara)
{
	datalogger_stop();
	fw_update_init(pPara);
	fw_update_run(pPara,MODE_FIRMWARE_UPDATE);
	
	
	
	while(1) {delay_ms(100);status_led_toggle();}
	return 0;
}

static RET_ERROR_CODE dl_cmd_cfg_update(const char * pPara)
{
	datalogger_stop();
	fw_update_init(pPara);
	fw_update_run(pPara,MODE_CFG_UPDATE);
	
	
	
	while(1) {delay_ms(100);status_led_toggle();}
	return 0;
}

static RET_ERROR_CODE dl_cmd_psw_update(const char * pPara)
{
	uint8_t a = 0;
	for(uint8_t idx=0;idx<4;++idx)
	{
		const char c = pPara[idx];
		if(c==0) goto invalid_psw;
		if (c!='0')
		{
			a |= (((uint8_t)1)<<(3-idx));
		} 
	}
	
	char szBuf[32];
	int v = a;
	sprintf_P(szBuf,PSTR("\t->\t%d\r\n\r\n"),v);
	
	debug_string_1P(NORMAL,PSTR("\r\n\r\nPSW_UPDATE: ---> "));
	debug_string(NORMAL,pPara,RAM_STRING);
	debug_string(NORMAL,szBuf,RAM_STRING);
	
	hal_psw_set(a);

#ifdef SETUP_PANEL
	PANEL_DATA d = {.status = a};
	panel_set_data(&d);
#endif

invalid_psw:
	return AC_ERROR_OK;
}

//This procedure parses the command string from the server and
//will call the internal procedure to accomplish
static RET_ERROR_CODE dl_task_cmd_check_2(char * const pBuf,uint16_t lenBuf)
{
//	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"DL_TASK_CMD_CHECK_2");

	uint16_t err = AC_ERROR_OK;

	if(0==strncasecmp_P(pBuf,PSTR("OK"),2)) {
		debug_string_1P(VERBOSE,PSTR("\tgot OK\r\n"));
		
	} else if(0==strncasecmp_P(pBuf,PSTR("GET_BY_DATE_RANGE:"),18)) {

		uint32_t dtBeg , dtEnd;
		
		char * psz;
		dtBeg = strtoul(pBuf+18,&psz,0);
		dtEnd = strtoul(psz+1,NULL,0);

		//if(g_log_verbosity>=VERBOSE) {
			//debug_string_1P(VERBOSE,PSTR("\tGET_BY_DATE_RANGE parameters "));
			//char szDbg[80];
			//snprintf_P(szDbg,sizeof(szDbg),PSTR("dtBeg:%lu\tdtEnd:%lu\r\n"),dtBeg,dtEnd);
			//debug_string(VERBOSE,szDbg,RAM_STRING);
		//}
		
		err = dl_cmd_get_by_date_range(dtBeg,dtEnd);
		
	} else if(0==strncasecmp_P(pBuf,PSTR("FWUPDATE,"),9)){

		//debug_string_1P(VERBOSE,PSTR("\tFW_UPDATE parameters "));
		pBuf[lenBuf]=0;
		err = dl_cmd_fw_update(pBuf+9);

	} else if(0==strncasecmp_P(pBuf,PSTR("CFG_UPDATE,"),11)){

		//debug_string_1P(VERBOSE,PSTR("\tCFG_UPDATE parameters "));
		pBuf[lenBuf]=0;
		err = dl_cmd_cfg_update(pBuf+11);

	} else if(0==strncasecmp_P(pBuf,PSTR("PSW_UPDATE:"),11)){

		//debug_string_1P(VERBOSE,PSTR("\tPSW_UPDATE parameters "));
		pBuf[lenBuf]=0;
		err = dl_cmd_psw_update(pBuf+11);

	} else 	if(0==strncasecmp_P(pBuf,PSTR("KO"),2)){
		debug_string_1P(VERBOSE,PSTR("\t[WARNING] Server returned a KO message\r\n"));
	} else {
		debug_string_1P(VERBOSE,PSTR("\t[WARNING] Server returned an unknown statement\r\n"));
	}
	
	return err;
}

static RET_ERROR_CODE dl_task_cmd_check_1(void)
{
//	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"DL_TASK_CMD_CHECK_1");

	const uint16_t BUFFER_SIZE = 256;
	char szBuf[BUFFER_SIZE];

	CFG_ITEM_ADDRESS f;
	cfg_find_item(CFG_TAG_DATALOGGER_SEND_URL,&f);
	cfg_get_item_file(f,szBuf,64);
	uint8_t l1 = strnlen(szBuf,64)-1;

	snprintf_P(szBuf+l1,BUFFER_SIZE-l1,PSTR("/CMD_CHK?AWSID="));
	l1 += strnlen(szBuf+l1,64);

	cfg_find_item(CFG_TAG_DATALOGGER_AWS_ID,&f);
	cfg_get_item_file(f,szBuf+l1,64);

	//cfg_get_aws_id(szBuf+l1,64);
	l1 += strnlen(szBuf+l1,64);

	snprintf_P(szBuf+l1,BUFFER_SIZE-l1,PSTR("&API_VER=1"));

	char retBuf[64];
	uint16_t lenRetBuf=60;
	RET_ERROR_CODE err = sim900_http_get(szBuf,false,retBuf,&lenRetBuf,false); //TODO: il terzo parametro (isBinary) deve essere definito
	sim900_http_close();

	if(err!=AC_ERROR_OK) {
		return err;
	}

	return dl_task_cmd_check_2(retBuf,lenRetBuf);
}

static RET_ERROR_CODE dl_task_cmd_check(void)
{
	task_status_server_cmd_check = TASK_RUNNING;

//	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"DL_TASK_CMD_CHECK");

	RET_ERROR_CODE err = sim900_bearer_simple_open();

	if(err==AC_ERROR_OK) {
		err = dl_task_cmd_check_1();
		sim900_bearer_simple_release();
	}

	quit_server_cmd_check:

	task_status_server_cmd_check = TASK_STOP;
	return err;
}

static RET_ERROR_CODE dl_MQTT_sendFromDatastore_2( const uint8_t dataStoreID )
{
//	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"DL_TASK_SENDFROMDATASTORE_2");

	const uint16_t lenData = sizeof(MQTTPacket_connectData)+256;
	char data[lenData];
	
	RET_ERROR_CODE e = dl_MQTTconnData_init(data,lenData);
	if ( AC_ERROR_OK != e)	{	return e;  }

	uint16_t TCPBufLen = 0;

	e = dl_MQTT_tcp_open(&TCPBufLen);
	if ( AC_ERROR_OK != e)	{	return e;  }

	MQTTPacket_connectData * const pConnData = (MQTTPacket_connectData * ) data;

	if(DATASTORE_LOG == dataStoreID) {
		e = send_log_with_MQTT(pConnData,TCPBufLen);
		sim900_tcp_close(0); //TODO: define a tcp client identifier
	} else if(DATASTORE_DB_RT == dataStoreID) {
		e = send_data_with_MQTT(&dl_send_params_RT,pConnData,TCPBufLen);
		sim900_tcp_close(3); //TODO: define a tcp client identifier
	} else if(DATASTORE_DB == dataStoreID) {
		e = send_data_with_MQTT(&dl_send_params,pConnData,TCPBufLen);
		sim900_tcp_close(3); //TODO: define a tcp client identifier
	}


	return e;
	
}


static RET_ERROR_CODE dl_MQTT_sendFromDatastore( const uint8_t dataStoreID )
{
	RET_ERROR_CODE e = dl_VerifyDataXferFeasibility();

	if(e==AC_ERROR_OK) {
		e = dl_MQTT_sendFromDatastore_2(dataStoreID);
	}
	
	return e;
}


#ifdef RMAP_SERVICES

static RET_ERROR_CODE dl_RMAP_tcp_open(uint16_t * const TCPBufLen)
{
	char szAPN[64];
	char szServerAddress[128];
	char szServerPort[8];

	RET_ERROR_CODE e =sim900_get_APN_by_operator(szAPN,sizeof(szAPN));

	if ( AC_ERROR_OK != e)
	{
		debug_string_1P(NORMAL,PSTR("Cannot get APN from eeprom"));
		return e;
	}

	e = sim900_tcp_init(1,szAPN,RAM_STRING);//TODO: client ID should be better defined 
	if ( AC_ERROR_OK != e)
	{
		debug_string_1P(NORMAL,PSTR("Cannot set APN"));
		return e;
	}

	CFG_ITEM_ADDRESS f;
	e = cfg_find_item(CFG_TAG_DATALOGGER_RMAP_SERVER,&f);
	if ( AC_ERROR_OK != e)	{	return e;  }

	e = cfg_get_item_file(f,szServerAddress,sizeof(szServerAddress));
	if ( AC_ERROR_OK != e)	{	return e;  }

	e = cfg_find_item(CFG_TAG_DATALOGGER_RMAP_SPORT,&f);
	if ( AC_ERROR_OK != e)	{	return e;  }

	e = cfg_get_item_file(f,szServerPort,sizeof(szServerPort));
	if ( AC_ERROR_OK != e)	{	return e;  }

	e = sim900_tcp_connect(1,szServerAddress,RAM_STRING,szServerPort,RAM_STRING);
	if ( AC_ERROR_OK != e)	{	return e;  }

	//e = sim900_tcp_get_buffer_len(1,TCPBufLen);
	//if ( AC_ERROR_OK != e )
	//{
		//debug_string_1P(NORMAL,PSTR("Cannot get TCP buffer size\r\n"));
		//return e;
	//}


	return AC_ERROR_OK;
}


static RET_ERROR_CODE dl_RMAP_connData_init(char * const heap,uint16_t lenHeap)
{
	static const __flash MQTTPacket_connectData data_init = MQTTPacket_connectData_initializer;
	
	MQTTPacket_connectData * pData = heap;
	
	memcpy_P(pData,&data_init,sizeof(MQTTPacket_connectData));
	
	char * szClientID = heap+sizeof(MQTTPacket_connectData)+1;
	char * szUsername = szClientID + 32;
	char * szPassword = szUsername + 32;

	memset(szClientID,0,96);

	CFG_ITEM_ADDRESS f;
	RET_ERROR_CODE e = cfg_find_item(CFG_TAG_DATALOGGER_AWS_ID,&f);
	if ( AC_ERROR_OK != e)	{	
		debug_string_1P(NORMAL,PSTR("[ERRROR] RMAP connData unable to get AWS_ID index\r\n"));
		return e;  
	}

	static const __flash char exid[]={'R','M','_'};
	szClientID[0]=exid[0];
	szClientID[1]=exid[1];
	szClientID[2]=exid[2];
	
	e = cfg_get_item_file(f,szClientID+3,28);
	if ( AC_ERROR_OK != e)	{
		debug_string_1P(NORMAL,PSTR("[ERRROR] RMAP connData unable to get AWS_ID value\r\n"));
		return e;
	}

	e = cfg_find_item(CFG_TAG_DATALOGGER_RMAP_USERNAME,&f);
	if ( AC_ERROR_OK != e)	{	
		debug_string_1P(NORMAL,PSTR("[ERRROR] RMAP connData unable to get username index\r\n"));
		return e;
	}
	
	e = cfg_get_item_file(f,szUsername,32);
	if ( AC_ERROR_OK != e)	{	
		debug_string_1P(NORMAL,PSTR("[ERRROR] RMAP connData unable to get username value\r\n"));
		return e;
	}

	e = cfg_find_item(CFG_TAG_DATALOGGER_RMAP_PASSWORD,&f);
	if ( AC_ERROR_OK != e)	{	
		debug_string_1P(NORMAL,PSTR("[ERRROR] RMAP connData unable to get password index\r\n"));
		return e;
	}
	
	e = cfg_get_item_file(f,szPassword,32);
	if ( AC_ERROR_OK != e)	{	
		debug_string_1P(NORMAL,PSTR("[ERRROR] RMAP connData unable to get password value\r\n"));
		return e;
	}

	
	pData->clientID.cstring = szClientID;
	pData->keepAliveInterval = 20;
	pData->cleansession = 1;
	pData->username.cstring = szUsername;
	pData->password.cstring = szPassword;

	
	return AC_ERROR_OK;
}


static RET_ERROR_CODE dl_RMAP_send_data( void )
{
//	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"DL_RMAP_SEND_DATA");

	const uint16_t lenData = sizeof(MQTTPacket_connectData)+256;
	char data[lenData];
	
	RET_ERROR_CODE e = dl_RMAP_connData_init(data,lenData);
	if ( AC_ERROR_OK != e)	{	
		debug_string_1P(NORMAL,PSTR("[ERRROR] RMAP connData returns error\r\n"));
		return e;  
	}

	uint16_t TCPBufLen = 0;

	if( !(dl_send_params_RMAP.send_flags & DL_SEND_FLAGS_CONNECTED) ) {
		e = dl_RMAP_tcp_open(&TCPBufLen);
		if ( AC_ERROR_OK != e)	{	
			debug_string_1P(NORMAL,PSTR("[ERRROR] RMAP tcp_open returns error\r\n"));
			return e;  
		}
	}

	e = sim900_tcp_get_buffer_len(1,TCPBufLen);
	if ( AC_ERROR_OK != e )
	{
		debug_string_1P(NORMAL,PSTR("Cannot get TCP buffer size\r\n"));
		return e;
	}


	MQTTPacket_connectData * const pConnData = (MQTTPacket_connectData * ) data;

	e = send_data_with_RMAP(&dl_send_params_RMAP,pConnData,TCPBufLen);

	if(AC_TASK_STILL_PENDING!=e)
	{
		sim900_tcp_close(1);//TODO: define a TCP identifier
	}


	return e;
	
}



RET_ERROR_CODE dl_RMAP_task_send_data( void )
{
	task_status_send_data_RMAP = TASK_RUNNING;
//	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"DL_TASK_SEND_DATA_RMAP");

	RET_ERROR_CODE err = dl_RMAP_send_data();//send_data_with_RMAP(&dl_send_params_RT);

	if(err==AC_TASK_STILL_PENDING) {
		task_status_send_data_RMAP = TASK_READY;
		err = AC_ERROR_OK;
	} else {
		task_status_send_data_RMAP  = TASK_STOP;
		task_status_server_cmd_check = TASK_READY;
	}

	return err;
}

#endif


static RET_ERROR_CODE dl_task_send_data_RT( void )
{
	task_status_send_data_RT = TASK_RUNNING;
//	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"DL_TASK_SEND_DATA_RT");

#ifdef MQTT_AS_PRIMARY
	RET_ERROR_CODE err = dl_MQTT_sendFromDatastore(DATASTORE_DB_RT);
#else
	RET_ERROR_CODE err = send_data_with_post(&dl_send_params_RT);
#endif	
	
	if(err==AC_TASK_STILL_PENDING) {
		task_status_send_data_RT = TASK_READY;
		err = AC_ERROR_OK;
	} else {

#ifndef MQTT_AS_PRIMARY
		sim900_bearer_simple_release();
#endif
		task_status_send_data_RT = TASK_STOP;

#ifdef RMAP_SERVICES
		task_status_send_data_RMAP = TASK_READY;
#else		
		task_status_server_cmd_check = TASK_READY;
#endif		

		task_status_send_log = TASK_READY;
	}

	return err;
}


static RET_ERROR_CODE dl_task_send_data(void)
{
	task_status_send_data = TASK_RUNNING;
//	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"DL_TASK_SEND_DATA");

#ifdef MQTT_AS_PRIMARY
	RET_ERROR_CODE err = dl_MQTT_sendFromDatastore(DATASTORE_DB_RT);
#else
	RET_ERROR_CODE err = send_data_with_post(&dl_send_params);
#endif

	if(err==AC_TASK_STILL_PENDING) {
		task_status_send_data = TASK_READY;
		err = AC_ERROR_OK;
	} else {
#ifndef MQTT_AS_PRIMARY
		sim900_bearer_simple_release();
#endif
		task_status_send_data = TASK_STOP;
	}

	return err;
}


//static RET_ERROR_CODE send_data_with_get( DL_SEND_PARAMS * const pPara )
//{
//
	//RET_ERROR_CODE err = 0;
//
	//#ifdef SETUP_RAINGAUGE_AUX
	//const uint16_t BUFFER_SIZE = 256;
	//#else
	//const uint16_t BUFFER_SIZE = 256;
	//#endif
//
	//char szBuf[BUFFER_SIZE];
	//
	//DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"SEND_DATA_WITH_GET");
//
	////Datalogger is empty
	//DB_Iterator itBeg;
	//DB_iterator_get_begin(&itBeg);
	//if(pPara->iter_send.plain==itBeg.plain) {
		//debug_string(NORMAL,PSTR("\tNothing left to send, quit the procedure\r\n"),true);
		//err = 0;
		//goto quit_senddata_task;
	//}
//
	//DL_LOG_ITEM ds;
	//DB_Iterator it;
	//
	////calculating the reverse iterator
	//const uint32_t x = LOG_FIRST_VALID_ADDRESS+sizeof(DL_LOG_ITEM);
	//const uint32_t z = pPara->iter_send.plain;
	//if(z<x) {
		//debug_string(NORMAL,PSTR("\titerator wraps datalogger\r\n"),true);
		//it.plain = LOG_LAST_VALID_ADDRESS - (x-z);
		//} else {
		//it.plain = z - sizeof(DL_LOG_ITEM);
	//}
//
	//if(NORMAL==g_sendverb) {
		//debug_string(g_sendverb,PSTR("\tsending record "),true);
		//AT24CXX_iterator_report(it);
	//}
	//uint8_t pg,msb,lsb;
	//AT24CXX_iterator_to_address(&it,&pg,&msb,&lsb);
	//uint8_t * const pBuf = (uint8_t *) &ds;
	//const uint8_t v = pPara->iter_send.byte[LSB_BYTE];
	////is it a plain page or a overlapping read ?
	//if ((v>=sizeof(DL_LOG_ITEM)) || (v==0)) {
		//AT24CXX_ReadBlock(pg,msb,lsb,pBuf,sizeof(DL_LOG_ITEM));
		//} else {
		//const uint8_t z = (sizeof(DL_LOG_ITEM) - v);
		//AT24CXX_ReadBlock(pg,msb,lsb,pBuf,z);
		//delay_ms(5);
		//AT24CXX_ReadBlock(pPara->iter_send.byte[PAGE_BYTE],pPara->iter_send.byte[MSB_BYTE],0,pBuf+z,v);
	//}
//
	//const uint32_t min_date_boundary = pPara->dt_end;//  dl_send_params_RT.dt_end;
//
	//if(NORMAL<=g_sendverb) {
		//snprintf_P(szBuf,sizeof(szBuf),PSTR("\r\n\tSending record with timestamp %lu\r\n"),ds.dl_data.data_timestamp);
		//debug_string(NORMAL,szBuf,false);
	//}
//
	////Record flag==0 means that is unsent
	////if the record is older than t and already sent
	////the procedure will end
	//if (ds.dl_data.data_timestamp<min_date_boundary)
	//{
		//debug_string(NORMAL,PSTR("\tMinimum date boundary reached\r\n"),true);
		////sim900_GPRS_close();
//
		//goto quit_senddata_task;
	//}
//
	//if ((pPara->send_once)&&(ds.flags!=0))
	//{
		//debug_string(g_sendverb,PSTR("\tskips an already sent record\r\n"),true);
		//goto skip1_senddata_task;
	//}
//
	//CFG_ITEM_ADDRESS f;
	//cfg_find_item(DATALOGGER_CFG_SE_URL,&f);
	//cfg_get_item_file(f,szBuf,64);
//
	//uint8_t l1 = strnlen(szBuf,64);
	//szBuf[l1  ] = 'A';
	//szBuf[l1+1] = 'W';
	//szBuf[l1+2] = 'S';
	//szBuf[l1+3] = 'I';
	//szBuf[l1+4] = 'D';
	//szBuf[l1+5] = '=';
	//l1+=6;
//
	//cfg_find_item(DATALOGGER_CFG_AWS_ID,&f);
	//cfg_get_item_file(f,szBuf+l1,64);
//
	//l1 += strnlen(szBuf+l1,64);
	//
	//
	//for(MODULE_ID im=DL_MODULE_BEG;im<DL_MODULE_END;++im)
	//{
		//DATA2STRING fn_stat2string = iface_module[im].pFN_Data2String;
//
		//void * pStat = selectDataSource(im,&ds);
		//if ((pStat==NULL) || (fn_stat2string==NULL))
		//{
			//debug_string_P(NORMAL,PSTR("[WARNING] send with post stats module unknown, skip\r\n"));
			//continue;
		//}
		//uint16_t lb = BUFFER_SIZE - l1;
		//if(AC_ERROR_OK == fn_stat2string(pStat,szBuf+l1,&lb)) {
			//l1 += lb;
		//} else {
			//debug_string_P(NORMAL,PSTR("[WARNING] send with get buffer overflow aborting\r\n"));
			//goto quit_senddata_task;
		//}
	//}
//
//
	//char retBuf[32];
	//uint16_t lenRetBuf=30;
	////const uint8_t e = 0;
	//err = sim900_http_get(szBuf,false,retBuf,&lenRetBuf,false); //TODO: il terzo parametro (isBinary) deve essere definito
	//sim900_http_close();
	////	debug_string(NORMAL,szBuf,false);
	////	debug_string(NORMAL,szCRLF,true);
	//if(err!=AC_ERROR_OK) {
		//goto quit_senddata_task;
	//}
//
//
	////Flag the record as sent
	//ds.flags = 1;
	//
	////q points to ds.flags field, so only one byte will be written
	//const AT24CXX_iterator q = {.plain = it.plain + ((&ds.flags)-((uint8_t*)&ds))};
	//AT24CXX_iterator_to_address(&q,&pg,&msb,&lsb);
	//AT24CXX_WriteBlock(pg,msb,lsb,(uint8_t*)&ds.flags,1);
	//delay_ms(5);
//
	//skip1_senddata_task:
//
	//DB_iterator_get_begin(&itBeg);
	//if (pPara->iter_send.plain == itBeg.plain)
	//{
		//goto quit_senddata_task;
	//}
//
	//pPara->iter_send = it;
//
	//debug_string(g_sendverb,PSTR("\trequesting another iteration\r\n"),PGM_STRING);
	//g_sendverb = g_log_verbosity;
	//return AC_TASK_STILL_PENDING;
	//
//
	//quit_senddata_task:
//
	//pPara->iter_send.plain = 0;
	//return err;
//}


static RET_ERROR_CODE dl_MQTTconnData_init(char * const heap,uint16_t lenHeap)
{
	static const __flash MQTTPacket_connectData data_init = MQTTPacket_connectData_initializer;
	
	MQTTPacket_connectData * pData = heap;
	
	memcpy_P(pData,&data_init,sizeof(MQTTPacket_connectData));
	
	char * szClientID = heap+sizeof(MQTTPacket_connectData)+1;
	char * szUsername = szClientID + 32;
	char * szPassword = szUsername + 32;

	memset(szClientID,0,96);

	CFG_ITEM_ADDRESS f;
	RET_ERROR_CODE e = cfg_find_item(CFG_TAG_DATALOGGER_AWS_ID,&f);
	if ( AC_ERROR_OK != e)	{	return e;  }

	e = cfg_get_item_file(f,szClientID,32);
	if ( AC_ERROR_OK != e)	{	return e;  }

	e = cfg_find_item(CFG_TAG_DATALOGGER_ID_USERNAME,&f);
	if ( AC_ERROR_OK != e)	{	return e;  }
	
	e = cfg_get_item_file(f,szUsername,32);
	if ( AC_ERROR_OK != e)	{	return e;  }

	e = cfg_find_item(CFG_TAG_DATALOGGER_ID_PASSWORD,&f);
	if ( AC_ERROR_OK != e)	{	return e;  }
	
	e = cfg_get_item_file(f,szPassword,32);
	if ( AC_ERROR_OK != e)	{	return e;  }

	
	pData->clientID.cstring = szClientID;
	pData->keepAliveInterval = 20;
	pData->cleansession = 1;
	pData->username.cstring = szUsername;
	pData->password.cstring = szPassword;

	
	return AC_ERROR_OK;
}

static RET_ERROR_CODE dl_MQTT_tcp_open(uint16_t * const TCPBufLen)
{
	char szAPN[64];
	char szServerAddress[128];
	char szServerPort[8];

	RET_ERROR_CODE e =sim900_get_APN_by_operator(szAPN,sizeof(szAPN));

	if ( AC_ERROR_OK != e)
	{
		debug_string_1P(NORMAL,PSTR("Cannot get APN from eeprom"));
		return e;
	}

	e = sim900_tcp_init(0,szAPN,RAM_STRING); //TODO: client ID should be better defined 
	if ( AC_ERROR_OK != e)
	{
		debug_string_1P(NORMAL,PSTR("Cannot set APN"));
		return e;
	}

	CFG_ITEM_ADDRESS f;
	e = cfg_find_item(CFG_TAG_DATALOGGER_LOG_ADDRESS,&f);
	if ( AC_ERROR_OK != e)	{	return e;  }

	e = cfg_get_item_file(f,szServerAddress,sizeof(szServerAddress));
	if ( AC_ERROR_OK != e)	{	return e;  }

	e = cfg_find_item(CFG_TAG_DATALOGGER_LOG_PORT,&f);
	if ( AC_ERROR_OK != e)	{	return e;  }

	e = cfg_get_item_file(f,szServerPort,sizeof(szServerPort));
	if ( AC_ERROR_OK != e)	{	return e;  }

	e = sim900_tcp_connect(0,szServerAddress,RAM_STRING,szServerPort,RAM_STRING);
	if ( AC_ERROR_OK != e)	{	return e;  }

	e = sim900_tcp_get_buffer_len(0,TCPBufLen);
	if ( AC_ERROR_OK != e )
	{
		debug_string_1P(NORMAL,PSTR("Cannot get TCP buffer size\r\n"));
		return e;
	}


	return AC_ERROR_OK;
}



RET_ERROR_CODE dl_MQTT_send( const uint8_t tcp_cid, char * const TCPBuf, const uint16_t lenTCPBuf,const uint16_t numMQTTmessages, uint16_t * const pQOSACK )
{
	//We should receive the acknowledge from both the connection
	//and the publish messages
	//MQTT says that the first message received will be a connection
	//acknowledge followed by the other publish message
	//These messages are all fixed header type with a size of 4 bytes
	 
	uint16_t retLen = (4*numMQTTmessages);
	char szRet[retLen];

	RET_ERROR_CODE e = sim900_tcp_send(tcp_cid,TCPBuf,RAM_STRING,lenTCPBuf,szRet,&retLen);
	if(AC_SIM900_TIMEOUT == e) {
		return e;
		
	} else if ( AC_ERROR_OK != e)	{	return e;  }

	char szBuf[128];

	for(uint16_t i = 0;i<retLen;i+=4)
	{
		
		if (( (szRet[i] & 0xF0) == 0b01000000) && (szRet[i+1] == 0b00000010))
		{//PUBACK message
			int n1 = (((uint16_t) szRet[i+2])<<8) + (uint8_t) szRet[i+3];
			sprintf_P(szBuf,PSTR("MQTT PUBACK -> OK  --- packet ID: %d\r\n"),n1);
			debug_string(NORMAL,szBuf,RAM_STRING);
			
			if (NULL != pQOSACK) 
			{
				(*pQOSACK)++;
			}
			
		} else if (( (szRet[i] & 0xF0) == 0b00100000) && (szRet[i+1] == 0b00000010)) {//CONNACK message
			debug_string_1P(NORMAL,PSTR("MQTT CONNACK -> OK\r\n"));
			
		} else if(((szRet[i] & 0xF0) == 0b11100000) && (szRet[i+1]==0)) {
			debug_string_1P(NORMAL,PSTR("MQTT CONN CLOSE\r\n"));
			
			
		} else { //UNKNOWN and not expected message
			const int n1 = szRet[i];
			const int n2 = szRet[i+1];
			const int n3 = szRet[i+2];
			const int n4 = szRet[i+3];
			sprintf_P(szBuf,PSTR("MQTT MSG ACK -> MISMATCH got %d:%d:%d:%d\r\n"),n1,n2,n3,n4);
			debug_string(NORMAL,szBuf,RAM_STRING);
		}
	}

	
	return AC_ERROR_OK;
}

static void mem_dump(char * pMem,uint16_t len)
{
	char szBuf[128];

	debug_string(NORMAL,PSTR("Memory dump image BEGIN\r\n\r\n\r\n\r\n"),PGM_STRING);

	static const __flash char szSEP1[] = " %3d";
	static const __flash char szSEP2[] = ",%3d";
	

	uint16_t idx = 0;
	while(idx<len)
	{
		snprintf_P(szBuf,sizeof(szBuf),PSTR("\r\n%04d\t"),idx);
		char * const psz = szBuf + 7;

		const char * pSEP = szSEP1;
		uint8_t i=0;
		for (;i<12;++i)
		{
			if ((idx+i) >= len)	{ break; }

			const uint8_t c = pMem[idx+i];
			sprintf_P(psz+(4*i),pSEP,c);
			if((c>31) && (c<127)) {
				psz[52+i] = c;
			} else {
				psz[52+i] = '.';
			}
			psz[53+i] = 0;
			pSEP = szSEP2;


		}
		idx += i;
		for (i*=4;i<52;++i)
		{
			psz[i] = 32;
		}

		//psz[68] = 0;
		
		debug_string(NORMAL,szBuf,RAM_STRING);
	}
	debug_string(NORMAL,PSTR("\r\n\r\n\r\n\r\nMemory dump image END\r\n"),PGM_STRING);

}


static RET_ERROR_CODE send_log_with_MQTT( MQTTPacket_connectData * const pData ,const uint16_t MSGMAXLEN)
{
	//The szBuf will contain all the MQTT messages
	//including the CONNECT and the DISCONNECT
	//Since the DISCONNECT message is a fixed length
	//and its size is 2 bytes we will allocate extra
	//2 bytes to be sure that we will not run out of
	//buffer
	char szBuf[MSGMAXLEN];
	const int buflen = MSGMAXLEN - 2;

	
	sprintf_P(szBuf,PSTR("TCP buffer size = %d"),MSGMAXLEN);
	debug_string(NORMAL,szBuf,RAM_STRING);

	int len = MQTTSerialize_connect(szBuf, buflen, pData);

	MQTTString topicString = MQTTString_initializer;

	char szTopic[128];
	char szName[32] = {0};
	
	CFG_ITEM_ADDRESS f;
	if ( AC_ERROR_OK != cfg_find_item(CFG_TAG_DATALOGGER_AWS_PRETTYNAME,&f)) {	
		strncat(szName,pData->clientID.cstring,sizeof(szName));
	} else if(AC_ERROR_OK != cfg_get_item_file(f,szName,sizeof(szName))) {
		strncat(szName,pData->clientID.cstring,sizeof(szName));
	}


	snprintf_P(szTopic,sizeof(szTopic),PSTR("ACRONET/STATION/%s/LOG"),szName);
	topicString.cstring = szTopic;
	
	LOG_LOCK_PARAMS lp;
	LOG_lock(&lp);

	AT24CXX_iterator iii = lp.it;
	
	char szPayload[256];
	uint16_t count = 1; //Consider the CONNECT message
	while(len<buflen) {
		
		uint16_t lenPayload = sizeof(szPayload);
		const RET_ERROR_CODE r = LOG_get_next_string(&lp,szPayload,&lenPayload);
		if(AC_ERROR_OK != r) {
			break;
		}
		
		const int l = MQTTSerialize_publish( szBuf + len,		//pointer to the buffer
											 buflen - len,		//length of the buffer
											 0,					//dup
											 1,					//QOS
											 0,					//retained
											 count,				//packet ID
											 topicString,		//topic
											 szPayload,			//MSG payload
											 lenPayload);		//MSG payload len
		if (l<0) {
			debug_string_2P(	NORMAL,
								PSTR("send_log_with_MQTT"),
								PSTR("the buffer is not big enough. Sending in chunks.") );
			uint8_t vv = g_log_verbosity;
			g_log_verbosity = VERY_VERBOSE;
			lp.it = iii;
			//mem_dump(szBuf,len);
			const RET_ERROR_CODE err = dl_MQTT_send(0,szBuf,len, count, NULL); 
			g_log_verbosity = vv;
			if (AC_ERROR_OK!=err)
			{
				return err;
			}
			count = 0;
			len = 0;
			continue;
		}
		
		iii = lp.it;
		len += l;
		count++;
	}

	LOG_unlock(&lp);

	len += MQTTSerialize_disconnect(szBuf+ len, MSGMAXLEN - len); //Overhead of 2 bytes given in the declaration 

	uint8_t vv = g_log_verbosity;
	g_log_verbosity = VERY_VERBOSE;
	lp.it = iii;
	//mem_dump(szBuf,len);
	const RET_ERROR_CODE err = dl_MQTT_send(0,szBuf,len, count, NULL);
	g_log_verbosity = vv;

	return err; 
}






static RET_ERROR_CODE dl_task_send_log( void )
{
	task_status_send_log = TASK_RUNNING;
//	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"DL_TASK_SEND_LOG");

	RET_ERROR_CODE e = dl_MQTT_sendFromDatastore(DATASTORE_LOG);

	task_status_send_log = TASK_STOP;

	if(AC_SIM900_RESOURCE_UNAVAILABLE == e) { //TODO: this error code should not be handled here
		task_status_send_log = TASK_READY;
		return AC_ERROR_OK;
	}
	
	return e;
}


RET_ERROR_CODE send_data_with_post( DL_SEND_PARAMS * const pPara )
{

	RET_ERROR_CODE err = 0;

#ifdef SETUP_RAINGAUGE_AUX
	static const int16_t BUFFER_SIZE = 3584;
#else
	static const int16_t BUFFER_SIZE = 3584;
#endif


//	DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"SEND_DATA_WITH_POST");

	char szBuf[BUFFER_SIZE];
	static const uint8_t QUEUE_LEN = 16;
	AT24CXX_iterator q[QUEUE_LEN];
	uint8_t    recflag[QUEUE_LEN];

	memset(q,0,sizeof(AT24CXX_iterator)*QUEUE_LEN);
	memset(recflag,0,sizeof(uint8_t)*QUEUE_LEN);

	DB_ITERATOR itBeg;
	DB_iterator_get_begin(&itBeg);

	//Datalogger is empty
	if(pPara->iter_send.plain==itBeg.plain) {
		debug_string_1P(NORMAL,PSTR("*** Nothing left to send, quit the procedure ***"));
		err = AC_ERROR_OK;
		goto quit_senddatapost_task;
	}

	DB_RECORD ds;
	
	CFG_ITEM_ADDRESS f;
	cfg_find_item(CFG_TAG_DATALOGGER_SEND_URL,&f);
	cfg_get_item_file(f,szBuf,128);

	const size_t lz = strnlen(szBuf,128);
	
	szBuf[lz-1] = '2';
	szBuf[lz] = 0;
	
	const size_t post_data_start = lz+1;
	size_t post_data_end = post_data_start;

	szBuf[post_data_end  ] = 'A';
	szBuf[post_data_end+1] = 'W';
	szBuf[post_data_end+2] = 'S';
	szBuf[post_data_end+3] = 'I';
	szBuf[post_data_end+4] = 'D';
	szBuf[post_data_end+5] = '=';
	post_data_end+=6;

	cfg_find_item(CFG_TAG_DATALOGGER_AWS_ID,&f);
	cfg_get_item_file(f,szBuf+post_data_end,MAX_AWS_NAME_LENGTH);

	post_data_end += strnlen(szBuf+post_data_end,MAX_AWS_NAME_LENGTH);
	
	const uint32_t now = hal_rtc_get_time();
	
	post_data_end += snprintf_P(szBuf+post_data_end,max(0,BUFFER_SIZE-post_data_end),PSTR("&TID=%lu"),now);
//	post_data_end += strnlen(szBuf+post_data_end,64);

	const uint32_t min_date_boundary = pPara->dt_end;//  dl_send_params_RT.dt_end;
	const uint8_t send_once = (pPara->send_flags & DL_SEND_FLAGS_ONCE);

	AT24CXX_iterator it = pPara->iter_send;

	uint8_t queue_iter=0;
	while(queue_iter<QUEUE_LEN) {
		DB_iterator_get_begin(&itBeg);

		if(it.plain==itBeg.plain) {
			debug_string_1P(NORMAL,PSTR("*** Begin of memory reached ***"));
			break;
		}

		DB_iterator_moveback(&it,1);
		DB_get_record(&it,&ds);
		
		if (ds.data_timestamp<min_date_boundary)
		{
			debug_string_1P(NORMAL,PSTR("*** Minimum date reached ***"));
			break;
		}

		if ( (send_once) && ( (ds.flags & 0x01) != 0))
		{
			//debug_string(g_sendverb,PSTR("\tflag sent is set, skip\r\n"),true);
			continue;
		}

		const size_t pp = post_data_end;
		
		for(MODULE_ID  im=DL_MODULE_BEG;im<DL_MODULE_END;++im)
		{
			DATA2STRING fn_data2string = iface_module[im].pFN_Data2String;

			//const size_t off = selectDataSource(im);
			//if ((off==0xFFFF) || (fn_stat2string==NULL))
			if (fn_data2string==NULL)
			{
				debug_string_1P(NORMAL,PSTR("[WARNING] Data2String is null, skip"));
				continue;
			}
			size_t lb = BUFFER_SIZE - post_data_end;
			//if(AC_ERROR_OK == fn_stat2string( off+(uint8_t*)&ds , szBuf+post_data_end , &lb)) {
			if(AC_ERROR_OK == fn_data2string( &ds,szBuf+post_data_end , &lb)) {
				post_data_end += lb;
			} else {
				debug_string_1P(NORMAL,PSTR("[WARNING] send with post buffer overflow skipping records"));
				szBuf[pp] = 0;
				post_data_end = pp;
				break;
			}
		}

		
		if( (ds.flags & 0x01) == 0 ) {
			//const AT24CXX_iterator tmp = {.plain = it.plain + ((&ds.flags)-((uint8_t*)&ds))};
			q[queue_iter] = it;
			recflag[queue_iter] = ds.flags | 0x01;
		}
		
		queue_iter++;
	}
	
	if(queue_iter==0) goto quit_senddatapost_task;
	
	pPara->iter_send = it;


	err = sim900_http_post(szBuf,RAM_STRING,szBuf+post_data_start,post_data_end-post_data_start,RAM_STRING);

	if(err!=AC_ERROR_OK) {
		sim900_http_close();
		goto quit_senddatapost_task;
	}

	char szRet[32];
	uint16_t len_szRet=sizeof(szRet);
	sim900_http_read(szRet,&len_szRet,false); //TODO: il terzo parametro (isBinary) deve essere definito
	szRet[sizeof(szRet)-1] = 0;
	debug_string(NORMAL,PSTR("\tHTTP read got : "),PGM_STRING);
	debug_string(NORMAL,szRet,RAM_STRING);
	debug_string(NORMAL,g_szCRLF,PGM_STRING);

	sim900_http_close();


	//Flag the record as sent
	for(queue_iter=0;queue_iter<QUEUE_LEN;queue_iter++) {
		if(q[queue_iter].plain == 0) continue;
		DB_mark_record(q[queue_iter], recflag[queue_iter]);
	}

	//Check if there are other records to send
	DB_iterator_get_begin(&itBeg);
	if ((ds.data_timestamp<min_date_boundary) || (pPara->iter_send.plain == itBeg.plain))
	{
		goto quit_senddatapost_task;
	}

	//debug_string_1P(VERBOSE,PSTR("*** requesting another iteration ***"));
	//g_sendverb = NORMAL;
	return AC_TASK_STILL_PENDING; //the procedure didn't send all the requested data
	

quit_senddatapost_task:
	//g_sendverb = g_log_verbosity;

	pPara->iter_send.plain = 0;
	return err;
}


///////////////////////////////////////////////////////////////////////////////
//
//	 Send Data with MQTT
//
///////////////////////////////////////////////////////////////////////////////


RET_ERROR_CODE send_data_with_MQTT(   DL_SEND_PARAMS * const pPara
									, MQTTPacket_connectData * const pData
									, const uint16_t MSGMAXLEN )
{

	RET_ERROR_CODE err = 0;

	static const int16_t BUFFER_SIZE = 3584;
	static const int16_t PAYLOAD_SIZE = 512;

	//DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"SEND_DATA_WITH_MQTT");

	char szBuf[BUFFER_SIZE];
	char szPayLoad[PAYLOAD_SIZE];
	
	static const uint8_t QUEUE_LEN = 16;
	AT24CXX_iterator q[QUEUE_LEN];
	uint8_t    recflag[QUEUE_LEN];

	memset(q,0,sizeof(AT24CXX_iterator)*QUEUE_LEN);

	int len = MQTTSerialize_connect(szBuf, BUFFER_SIZE, pData);

	MQTTString topicString = MQTTString_initializer;

	char szTopic[128];
	char szName[32] = {0};
	
	CFG_ITEM_ADDRESS f;
	if ( AC_ERROR_OK != cfg_find_item(CFG_TAG_DATALOGGER_AWS_PRETTYNAME,&f)) {
		strncat(szName,pData->clientID.cstring,sizeof(szName));
	} else if(AC_ERROR_OK != cfg_get_item_file(f,szName,sizeof(szName))) {
		strncat(szName,pData->clientID.cstring,sizeof(szName));
	}


	snprintf_P(szTopic,sizeof(szTopic),PSTR("ACRONET/STATION/%s/DATA"),szName);
	topicString.cstring = szTopic;


	DB_ITERATOR itBeg;
	DB_iterator_get_begin(&itBeg);

	//Datalogger is empty
	if(pPara->iter_send.plain==itBeg.plain) {
		debug_string_1P(NORMAL,PSTR("*** Nothing left to send, quit the procedure ***"));
		err = AC_ERROR_OK;
		goto quit_senddatamqtt_task;
	}


	
	//const uint32_t now = hal_rtc_get_time();
	
	int lenPayload = 0;//snprintf_P(szPayLoad,sizeof(szPayLoad),PSTR("TID=%lu"),now);

	const uint32_t min_date_boundary = pPara->dt_end;//  dl_send_params_RT.dt_end;
	const uint8_t send_once = (pPara->send_flags & DL_SEND_FLAGS_ONCE);

	AT24CXX_iterator it = pPara->iter_send;

	DB_RECORD ds;

	uint8_t queue_iter=0;
	while(queue_iter<QUEUE_LEN) {
		DB_iterator_get_begin(&itBeg);

		if(it.plain==itBeg.plain) {
			debug_string_1P(NORMAL,PSTR("*** Begin of memory reached ***"));
			break;
		}

		DB_iterator_moveback(&it,1);
		DB_get_record(&it,&ds);
		
		if (ds.data_timestamp<min_date_boundary)
		{
			debug_string_1P(NORMAL,PSTR("*** Minimum date reached ***"));
			break;
		}

		if ( (send_once) &&  ((ds.flags & 0x01) != 0) )
		{
			//debug_string(g_sendverb,PSTR("\tflag sent is set, skip\r\n"),true);
			continue;
		}

		const int16_t pp = lenPayload;
		
		for(MODULE_ID  im=DL_MODULE_BEG;im<DL_MODULE_END;++im)
		{
			DATA2STRING fn_data2string = iface_module[im].pFN_Data2String;

			//const size_t off = selectDataSource(im);

			//if ((off==0xFFFF) || (fn_stat2string==NULL))
			if (fn_data2string==NULL)
			{
				debug_string_1P(NORMAL,PSTR("[WARNING] Data2String null method, skip"));
				continue;
			}
			int16_t lb = PAYLOAD_SIZE - lenPayload;
			if(AC_ERROR_OK == fn_data2string( &ds,szPayLoad+lenPayload,&lb)) {
				lenPayload += lb;
			} else {
				debug_string_1P(NORMAL,PSTR("[WARNING] send with MQTT buffer overflow skipping records"));
				szPayLoad[pp] = 0;
				lenPayload = pp;
				break;
			}
		}

		const int l = MQTTSerialize_publish(	szBuf + len,		//pointer to the buffer
												BUFFER_SIZE - len,	//length of the buffer
												0,					//dup
												1,					//QOS
												0,					//retained
												queue_iter,			//packet ID
												topicString,		//topic
												szPayLoad,			//MSG payload
												lenPayload);		//MSG payload len
		if (l<0) {
			debug_string_2P(	NORMAL,
								PSTR("send_data_with_MQTT"),
								PSTR("provided buffer is not big enough. Discarding message.") );
			break;
		}

		len += l;
		
		if((ds.flags & 0x01) == 0) {
			//const AT24CXX_iterator tmp = {.plain = it.plain + ((&ds.flags)-((uint8_t*)&ds))};
			q[queue_iter] = it;
			recflag[queue_iter] = ds.flags | 0x01;
		}
		
		szPayLoad[0] = 0;
		lenPayload = 0;
		queue_iter++;
	}
	
	if(queue_iter==0) goto quit_senddatamqtt_task;
	
	pPara->iter_send = it;

	len += MQTTSerialize_disconnect(szBuf+ len, BUFFER_SIZE - len +2); //Overhead of 2 bytes given in the declaration

	//We add one to the number of the messages because we count 
	dl_MQTT_send(3,szBuf,len, queue_iter + 1, NULL); //TODO: gestire eventuale codice di errore
	


	//Flag the record as sent
	for(queue_iter=0;queue_iter<QUEUE_LEN;queue_iter++) {
		if(q[queue_iter].plain == 0) continue;
		DB_mark_record(q[queue_iter], recflag[queue_iter]);
	}

	//Check if there are other records to send
	DB_iterator_get_begin(&itBeg);
	if ((ds.data_timestamp<min_date_boundary) || (pPara->iter_send.plain == itBeg.plain))
	{
		goto quit_senddatamqtt_task;
	}

	//debug_string_1P(VERBOSE,PSTR("*** requesting another iteration ***"));
	//g_sendverb = NORMAL;
	return AC_TASK_STILL_PENDING; //the procedure didn't send all the requested data
	

quit_senddatamqtt_task:
	sim900_tcp_close(3);
	//g_sendverb = g_log_verbosity;

	pPara->iter_send.plain = 0;
	return err;
}



#ifdef RMAP_SERVICES
///////////////////////////////////////////////////////////////////////////////
//
//	 Send Data with RMAP-MQTT
//
///////////////////////////////////////////////////////////////////////////////


static RET_ERROR_CODE send_data_with_RMAP_serialize(	 DB_RECORD * const pDS
//														,char * const szTopic
//														,const uint16_t lenSZTopic
														,char * const szBuf
														,uint16_t * const pBufLen
														,uint8_t * pNumRMAPmessages )
{
	MQTTString topicString = MQTTString_initializer;
	uint16_t len = 0;
	const uint16_t BUFFER_SIZE = *pBufLen;

	char szTopic[128];
	CFG_ITEM_ADDRESS f;


	if ( AC_ERROR_OK != cfg_find_item(CFG_TAG_DATALOGGER_RMAP_TOPIC,&f)) {
		strncpy_P(szTopic,PSTR(""),sizeof(szTopic));
	} else if(AC_ERROR_OK != cfg_get_item_file(f,szTopic,sizeof(szTopic))) {
		strncpy_P(szTopic,PSTR(""),sizeof(szTopic));
	}

	const uint16_t lenTopicPreamble = strnlen(szTopic,sizeof(szTopic));

	for(MODULE_ID  im=DL_MODULE_BEG;im<DL_MODULE_END;++im)
	{
		DATA2STRINGRMAP fn_data2stringRMAP = iface_module[im].pFN_Data2StringRMAP;

		//const size_t off = selectDataSource(im);

		if (fn_data2stringRMAP==NULL)
		{
			debug_string_1P(NORMAL,PSTR("[WARNING] Data2String null method, skip"));
			continue;
		}


		static const int16_t PAYLOAD_SIZE = 512;
		char szPayLoad[PAYLOAD_SIZE];
		uint8_t subModule = 0;
				
		while(1) {

			uint16_t lenSZTopic	= sizeof(szTopic) - lenTopicPreamble;

			int16_t lenPayload = sizeof(szPayLoad);
					
			if(AC_ERROR_OK == fn_data2stringRMAP(	 &subModule
													,pDS
													,pDS->dl_data.data_timestamp
													,g_timing.task_store
													,szTopic+lenTopicPreamble
													,&lenSZTopic
													,szPayLoad
													,&lenPayload )) 	{
						
				debug_string(NORMAL,szTopic,RAM_STRING);
				debug_string(NORMAL,g_szCRLF,PGM_STRING);
				debug_string(NORMAL,szPayLoad,RAM_STRING);
				debug_string(NORMAL,g_szCRLF,PGM_STRING);

				topicString.cstring = szTopic;
				unsigned short pid = *pNumRMAPmessages;
				
				const int l = MQTTSerialize_publish(	szBuf + len,		//pointer to the buffer
														BUFFER_SIZE - len,	//length of the buffer
														0,					//dup
														1,					//QOS
														0,					//retained
														pid,				//packet ID
														topicString,		//topic
														szPayLoad,			//MSG payload
														lenPayload);		//MSG payload len
				if (l<0) {
					debug_string_2P(	NORMAL,
										PSTR("send_data_with_MQTT"),
										PSTR("provided buffer is not big enough. Discarding message.") 
									);
					break;
				}

				len += l;
				(*pNumRMAPmessages)++;
						
				if (subModule == 255)
				{
					debug_string_1P(NORMAL,PSTR("+++ last submodule +++"));
					break;
				}
				
						
			} else {
				debug_string_1P(NORMAL,PSTR("[WARNING] send with RMAP buffer overflow skipping records"));
				break;
			}
		}
	}
	
	*pBufLen = len;
	return AC_ERROR_OK;
}


static bool send_data_with_RMAP_browse_queue( DL_RMAP_SEND_PARAMS * const pPara	)
{
	DB_ITERATOR itBeg;
	do {

		DB_iterator_get_begin(&itBeg);

		if(pPara->iter_send.plain==itBeg.plain) {
			debug_string_1P(NORMAL,PSTR("*** Begin of memory reached ***"));
			return false;
		}

		DB_iterator_moveback(&(pPara->iter_send),1);
		DB_get_record( &(pPara->iter_send), &(pPara->ds) );
		
		if ( (pPara->ds.dl_data.data_timestamp) < (pPara->dt_end) )
		{
			debug_string_1P(NORMAL,PSTR("*** Minimum date reached ***"));
			return false;
		}

		
	} while( (pPara->send_flags & DL_SEND_FLAGS_ONCE) && ((pPara->ds.flags & 0x02) != 0) );
	
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// this function must be called with a previously selected record
// and it will exit requesting another iteration if the queue contains another record to be sent
//
static RET_ERROR_CODE send_data_with_RMAP_iterate(	 DL_RMAP_SEND_PARAMS * const pPara
													,char * const szBuf
													,const int16_t BUFFER_SIZE
													,int * const p_idx_szBuf
													,uint8_t numExtraMessages )
{
	//char szTopic[128];
	//CFG_ITEM_ADDRESS f;
//
//
	//if ( AC_ERROR_OK != cfg_find_item(CFG_TAG_DATALOGGER_RMAP_TOPIC,&f)) {
		//strncat_P(szTopic,PSTR(""),sizeof(szTopic));
	//} else if(AC_ERROR_OK != cfg_get_item_file(f,szTopic,sizeof(szTopic))) {
		//strncat_P(szTopic,PSTR(""),sizeof(szTopic));
	//}
//
	//const uint16_t lenTopicPreamble = strnlen(szTopic,sizeof(szTopic));

	DB_ITERATOR itBeg;
	DB_iterator_get_begin(&itBeg);

	//Nothing to send
	if(pPara->iter_send.plain==itBeg.plain) {
		debug_string_1P(NORMAL,PSTR("*** Nothing left to send, quit the procedure ***"));
		return AC_NOTHING_TO_DO;
	}
		
	//DB_RECORD * pDS = &(pPara->ds);
	//bool hasSent = false;


	uint16_t lenBuf = BUFFER_SIZE - *p_idx_szBuf;
	uint8_t  numMSG = 0;
	send_data_with_RMAP_serialize(	 &(pPara->ds)
//									,szTopic+lenTopicPreamble
//									,sizeof(szTopic)-lenTopicPreamble
									,szBuf + (*p_idx_szBuf)
									,&lenBuf
									,&numMSG );
		
	*p_idx_szBuf += lenBuf;
	//if (numMSG==0)
	//{
		///*break;*/
	//}

	RET_ERROR_CODE err = AC_TASK_STILL_PENDING;
	if (!send_data_with_RMAP_browse_queue( pPara ))
	{
		debug_string_1P(NORMAL,PSTR("Serializing CLOSE\r\n"));
		(*p_idx_szBuf) += MQTTSerialize_disconnect(szBuf+(*p_idx_szBuf), BUFFER_SIZE-(*p_idx_szBuf) );
		pPara->send_flags = 0;
		//numExtraMessages++;
		err = AC_ERROR_OK;
	}
	

	//mem_dump(szBuf,*p_idx_szBuf);		
		
	uint16_t acks = 0;
	const unsigned short numRMAPmessages = numMSG;
	if( (AC_ERROR_OK == dl_MQTT_send(1,szBuf,*p_idx_szBuf, numRMAPmessages+numExtraMessages, &acks)) )
	{
		//Flag the record as sent only if all the messages sent with QOS==1
		//are covered by an ACK message
		if(acks >= numRMAPmessages) {
			DB_mark_record(pPara->iter_send, (pPara->ds.flags | 0x02) );
		}
	}
		
	*p_idx_szBuf = 0;
	
	return err;
}

RET_ERROR_CODE send_data_with_RMAP(   DL_RMAP_SEND_PARAMS * const pPara
									, MQTTPacket_connectData * const pData
									, const uint16_t MSGMAXLEN )
{
	//DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"SEND_DATA_WITH_RMAP");

	static const int16_t BUFFER_SIZE = 3584;
	char szBuf[BUFFER_SIZE];
	char szName[32] = {0};
	int idx_szBuf = 0;

	uint8_t numExtraMessages = 0; 

	if (!(pPara->send_flags & DL_SEND_FLAGS_CONNECTED))
	{
		pPara->send_flags |= DL_SEND_FLAGS_CONNECTED;
		CFG_ITEM_ADDRESS f;
		
		if ( AC_ERROR_OK != cfg_find_item(CFG_TAG_DATALOGGER_AWS_PRETTYNAME,&f)) {
			strncat(szName,pData->clientID.cstring,sizeof(szName));
		} else if(AC_ERROR_OK != cfg_get_item_file(f,szName,sizeof(szName))) {
			strncat(szName,pData->clientID.cstring,sizeof(szName));
		}

		if (!send_data_with_RMAP_browse_queue( pPara ))
		{
			//No record found to be sent
			pPara->iter_send.plain = 0;
			pPara->send_flags = (pPara->send_flags & 0b11111100);
			return AC_ERROR_OK;
		}
		
		debug_string_1P(NORMAL,PSTR("Serializing connect\r\n"));
		idx_szBuf = MQTTSerialize_connect(szBuf, BUFFER_SIZE, pData);
		numExtraMessages++;
	}
		
////////////////////////////////////////////////////////////////////////////////////////////////////////////



	return send_data_with_RMAP_iterate(pPara,szBuf,BUFFER_SIZE,&idx_szBuf,numExtraMessages);
}

#endif //RMAP_SERVICES


///////////////////////////////////////////////////////////////////////////////
//
//SNAPSHOT
//
///////////////////////////////////////////////////////////////////////////////


#define SNAPSHOT_SIZE 8 //Must be a power of 2
static volatile DB_RECORD g_data_snapshot[SNAPSHOT_SIZE];
static volatile uint8_t g_data_snapshot_beg = 0;
static volatile uint8_t g_data_snapshot_end = 0;

static inline void dl_snapshot_iter_dec(volatile uint8_t * const idx)
{
	//if( *idx == 0 ) {
		//(*idx)=(SNAPSHOT_SIZE-1);
	//} else {
		//(*idx)--;
	//}

	*idx = ( ((*idx) - 1) & (SNAPSHOT_SIZE-1) );
	
}

static inline void dl_snapshot_iter_inc(volatile uint8_t * const idx)
{
	//if( *idx>=(SNAPSHOT_SIZE-1) ) {
		//*idx=0;
	//} else {
		//(*idx)++;
	//}

	*idx = ( ((*idx) + 1) & (SNAPSHOT_SIZE-1) );
	
}


static void  dl_snapshot_init(const uint32_t ts)
{
	g_data_snapshot_beg = 0;
	g_data_snapshot_end = 0;

	volatile DB_RECORD * const p_item = &g_data_snapshot[0];
	p_item->data_timestamp = (ts==-1)?hal_rtc_get_time():ts;
	p_item->flags = 0;

}

static uint8_t  dl_snapshot_make(const uint32_t ts)
{
	
	volatile DB_RECORD * const p_rec = &g_data_snapshot[(g_data_snapshot_end & (SNAPSHOT_SIZE-1))];


	for(MODULE_ID  im=DL_MODULE_BEG;im<DL_MODULE_END;++im)
	{
		GETDATA fn_getStats = iface_module[im].pFN_GetData;
		RESETDATA fn_resetStats = iface_module[im].pFN_ResetData;

		//const size_t off = selectDataSource(im);
		
		if(fn_getStats)	fn_getStats(p_rec);
		if(fn_resetStats) fn_resetStats();

	}


	p_rec->data_timestamp = (ts==-1)?hal_rtc_get_time():ts;
	p_rec->flags = 0;

	dl_snapshot_iter_inc(&g_data_snapshot_end);
	if(g_data_snapshot_end==g_data_snapshot_beg) {
		debug_string(NORMAL,PSTR("(datalogger_snapshot_data) Snapshot memory is full. Something wrong is happening, discarding snapshots\r\n"),PGM_STRING);
		dl_snapshot_iter_inc(&g_data_snapshot_beg);
	}

	g_data_snapshot[(g_data_snapshot_end & (SNAPSHOT_SIZE-1))].data_timestamp = 0;
	g_data_snapshot[(g_data_snapshot_end & (SNAPSHOT_SIZE-1))].flags = 0;


	return 0;
}


static RET_ERROR_CODE dl_task_store_data( void )
{
	task_status_store_data = TASK_RUNNING;
	RET_ERROR_CODE err = AC_ERROR_OK;

	//DEBUG_PRINT_FUNCTION_NAME(VERBOSE,"STORE DATA");


	uint8_t idx = (g_data_snapshot_beg & (SNAPSHOT_SIZE-1));
	const uint8_t sn_end = (g_data_snapshot_end & (SNAPSHOT_SIZE-1));

	if(AC_ERROR_OK!=AT24CXX_Init())
	{
		goto task_store_quit;
	}

	while(idx!=sn_end) {
		volatile DB_RECORD * const pd = &g_data_snapshot[idx];
		
		//char szBuf[64];
		//snprintf_P(szBuf,sizeof(szBuf),PSTR("REC_ID %lu\r\n"),pd->dl_data.data_timestamp);
		//debug_string(NORMAL,szBuf,RAM_STRING);

		DB_append_record(pd);
		dl_snapshot_iter_inc(&idx);
	}


	DB_iterator_write_to_eeprom();

	task_store_quit:

	g_data_snapshot_beg = sn_end;//g_data_snapshot_end;
	task_status_store_data = TASK_STOP;
	return err;
}


bool dl_flush_buffers(void)
{
	//if (AC_ERROR_OK!=LOG_process_buffer())
	//{
		//return false;
	//}

	LOG_process_buffer();
	uint32_t l;
	LOG_get_length(&l);

	if( ((LOG_EEPROM_PARTITION_SIZE/3)*2) < l ) {
		task_status_send_log =TASK_READY;
	}

	return true;
}



static bool dl_yield( void )
{
	MODULE_ID im=DL_MODULE_BEG;

	while(im<DL_MODULE_END)
	{
		YIELDMODULE fn_Yield = iface_module[im++].pFN_Yield;
		if (NULL==fn_Yield)	continue;
		if(fn_Yield()) {
			goto redo_yield;
		}
	}
	
	return false;

	redo_yield:

	while(im<DL_MODULE_END)
	{
		YIELDMODULE fn_Yield = iface_module[im++].pFN_Yield;
		if (NULL==fn_Yield)	continue;
		fn_Yield();
	}
	
	return true;
}

static void dl_task_make_statistics(void)
{

	char buf[192];
	const int i =	snprintf_P(buf,sizeof(buf),PSTR("TIME=%lu"),hal_rtc_get_time());
	uint16_t lb = sizeof(buf) - i;
	hal_board_get_stats(buf+i,&lb);
	LOG_say(buf);
	buf[i+lb]   = '\r';
	buf[i+lb+1] = '\n';
	buf[i+lb+2] = 0;
	debug_string(NORMAL,buf,RAM_STRING);

}

static uint8_t sim900_guard_retries = 0;

static void dl_sim900_guard_clear(void)
{
	sim900_guard_retries = 0;
}

static void dl_sim900_guard_set(void)
{
	if(++sim900_guard_retries > 10) {
		sim900_init();
		dl_sim900_guard_clear();
	}
}


bool TM_DoYield( void )
{
	  return dl_yield();
}

static bool is_task_status_ready(const uint8_t theTask)
{
		dl_flush_buffers();
		return theTask == TASK_READY;	
}


void dl_run(void)
{
	while(1) {

		if (is_task_status_ready(task_make_statistics))
		{
			dl_task_make_statistics();
			task_make_statistics = TASK_STOP;
		}


		//////////////////////////////////////////////////////////////////////////////////////////////

		if(is_task_status_ready(task_status_store_data)) {
			dl_task_store_data();

#if defined (SETUP_CAP_RAIN) || defined (SETUP_CAP_LEVEL)
			cap_schedule_check();
#endif
		}
		
		//////////////////////////////////////////////////////////////////////////////////////////////


		if( is_task_status_ready(task_status_send_data_prepare_RT)) {
			
			dl_sim900_guard_set();
			
			const RET_ERROR_CODE e = dl_task_send_data_prepare_RT();
			if(AC_ERROR_OK==e)	{
				dl_sim900_guard_clear();
				task_status_send_data_RT = TASK_READY;
			} else /*if( (AC_SIM900_LINE_NOT_REGISTERED==e) || (AC_SIM900_GPRS_NOT_ATTACHED==e))*/ {
				//GPRS Line is not available try later
				//Schedule the SEND task after 10 seconds
				
				g_acc_task_send_RT=g_timing.task_send_rt-10;
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////////////

		if( is_task_status_ready(task_status_send_data_RT) ) {
			if(AC_ERROR_OK!=dl_task_send_data_RT()) {
				g_acc_task_send_RT=g_timing.task_send_rt-30; //Schedule the SEND task after 30 seconds
				
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////////////
#ifdef RMAP_SERVICES
		if(is_task_status_ready(task_status_send_data_RMAP) ) {
			dl_RMAP_task_send_data();
		}
#endif

		//////////////////////////////////////////////////////////////////////////////////////////////

		if(is_task_status_ready(task_status_send_log) ) {
			dl_task_send_log();
		}
		
		//////////////////////////////////////////////////////////////////////////////////////////////

		if( is_task_status_ready(task_status_server_cmd_check)) {
			dl_task_cmd_check();
		}

		//////////////////////////////////////////////////////////////////////////////////////////////

		if(is_task_status_ready(task_status_send_data)) {
			dl_task_send_data();
		}

		//////////////////////////////////////////////////////////////////////////////////////////////

		if(is_task_status_ready(task_status_sync_time))  {
			dl_task_sync_time();
		}

		//////////////////////////////////////////////////////////////////////////////////////////////

		if (dl_cycle_lock==false)
		{
			dl_periodic_update();
		}



		if(	(task_status_send_data_RT		==	TASK_READY)	||
			(task_status_send_data			==	TASK_READY)	||
			(task_status_server_cmd_check	==	TASK_READY)	||
			(task_status_sync_time			==	TASK_READY)) continue;
		
		sim900_bearer_simple_close();

		if(	(task_status_store_data			==	TASK_READY)	||
#ifdef RMAP_SERVICES
			(task_status_send_data_RMAP		==	TASK_READY)	||
#endif			
			(task_make_statistics			==	TASK_READY)) continue;

		
		dl_cycle_lock = true;
		while(dl_cycle_lock) {
			while(dl_yield()) {dl_flush_buffers();}
			sleepmgr_enter_sleep();
		};
		
		//dl_test_psw();
		
	};

}

void dl_test_do_dataset(void);

static void dl_test_sim900_terminal(void)
{
	//char c;
	//while(1) {
		//if(usart_rx_is_complete(USART_DEBUG)) {
			//c = usart_get(USART_DEBUG);
			//usart_putchar(USART_GPRS,c);
		//}
		//
		//if (USART_RX_CBuffer_Data_Available(SIM900_get_PUSART()))
		//{
			//c = USART_RX_CBuffer_GetByte(SIM900_get_PUSART());
			//usart_putchar(USART_DEBUG,c);
		//}
	//}	

		USART_data_t * ud = SIM900_get_PUSART();

		const usart_rs232_options_t USART_SERIAL_OPTIONS = {
			.baudrate = USART_GPRS_BAUDRATE,
			.charlength = USART_CHAR_LENGTH,
			.paritytype = USART_PARITY,
			.stopbits = USART_STOP_BIT
		};

		usart_serial_init(USART_GPRS, &USART_SERIAL_OPTIONS);

		usart_interruptdriver_initialize(ud,USART_GPRS,USART_INT_LVL_LO);
		usart_set_rx_interrupt_level(USART_GPRS,USART_INT_LVL_LO);
		usart_set_tx_interrupt_level(USART_GPRS,USART_INT_LVL_OFF);


		//sim900_init();	

		sim900_power_off();
		sim900_power_on();
	
		cpu_irq_enable();

		usart_tx_enable(USART_GPRS);
		usart_rx_enable(USART_GPRS);

		usart_putchar(USART_GPRS,'A');
		usart_putchar(USART_GPRS,'T');
		usart_putchar(USART_GPRS,'\r');
		usart_putchar(USART_GPRS,'\n');



		while (1)
		{

			if(USART_RX_CBuffer_Data_Available(ud))
			{
				status_led_toggle();
				char c =  USART_RX_CBuffer_GetByte(ud);
				usart_putchar(USART_DEBUG,c);
			}

			if(usart_rx_is_complete(USART_DEBUG) == true)
			{
				status_led_toggle();
				char c =  usart_get(USART_DEBUG);
				usart_putchar(USART_GPRS,c);
				usart_putchar(USART_DEBUG,c);
				if (c=='\r')
				{
					usart_putchar(USART_DEBUG,'\n');
				}
			}
		}

}

static void dl_test_do_eeprom_lookup(uint32_t t0,uint32_t td)
{
	DB_ITERATOR it;
	dl_search_record_by_date(t0 + (td / 10),EXACT_MATCH,&it);
	dl_search_record_by_date(t0 + (td /  2),EXACT_MATCH,&it);
	dl_search_record_by_date(t0 + (td /  2),RIGHT_BOUND,&it);
	
}

void dl_test_do_dataset(void)
{
	char szBuf[256];
	debug_string_1P(NORMAL,PSTR("eeprom fill inizio"));
	DB_reset_eeprom();
	DB_iterator_read_from_eeprom();


	DB_RECORD data;
	AT24CXX_iterator it,it2;
	
	DB_iterator_get_begin(&it);
	
	
	uint32_t t0 = 1221955200;
	uint32_t td = 0;
	int8_t p = -1;

	int filler = 0xA;

//	goto fine_fill;
	do {
		td += 900;
		memset(&data,filler++,sizeof(DB_RECORD));
		if (filler>0xF)
		{
			filler = 0xA;
		}
		
		data.data_timestamp = t0 + td;
		
		DB_iterator_get_end(&it);
		DB_append_record(&data);
		
		if (p != it.byte[PAGE_BYTE])
		{
			snprintf_P(szBuf,sizeof(szBuf),PSTR("timestamp : %lu "),t0+td);
			debug_string(NORMAL,szBuf,RAM_STRING);

			AT24CXX_iterator_report(it);
			p = it.byte[PAGE_BYTE];	
			
			dl_test_do_eeprom_lookup(t0,td);
		}
		
		DB_iterator_get_end(&it2);
	} while(it.plain<it2.plain);

fine_fill:

	debug_string_1P(NORMAL,PSTR("eeprom fill fine"));
	
	debug_string_1P(NORMAL,PSTR("\r\nBEGIN : "));
	
	DB_iterator_get_begin(&it);
	DB_iterator_get_end(&it2);

	AT24CXX_iterator_report(it);
	debug_string(NORMAL,PSTR("END   : "),PGM_STRING);
	AT24CXX_iterator_report(it2);
	
	it.plain = 0;
	it.byte[MSB_BYTE] = 1;
	
	it2 = it;
	it2.plain += 192;
	DB_dump(it,it2);
	
//	calendar_timestamp_to_date(t0+td,&adt);
//	snprintf_P(szBuf,sizeof(szBuf),PSTR("%02d/%02d/%d %02d:%02d:%02d\r\n"),adt.date+1,adt.month+1,adt.year,adt.hour,adt.minute,adt.second);

	snprintf_P(szBuf,sizeof(szBuf),PSTR("last timestamp : %lu\r\n"),t0+td);
	debug_string(NORMAL,szBuf,RAM_STRING);

	dl_test_do_eeprom_lookup(t0,td);

	
	while(1) {status_led_toggle();delay_ms(250);}
}


RET_ERROR_CODE cap_introspection_lookup(char * pKey,CAP_INTROSPECTION * pVal)
{
#if defined(SETUP_CAP_LEVEL) || defined(SETUP_CAP_RAIN)
	
	static const size_t d = sizeof(g_cap_introspection) / sizeof(g_cap_introspection[0]);
	char buf[128];
	sprintf_P(buf,PSTR("Introspection table elements = %u\r\n"),d);
	debug_string(NORMAL,buf,RAM_STRING);
	
	for (uint8_t id=0;id<d;++id)
	{
		debug_string_1P(NORMAL,g_cap_introspection[id].op_token);
		if (strcasecmp_P(pKey,g_cap_introspection[id].op_token)==0)
		{
			pVal->num_params = pgm_read_byte_far(&g_cap_introspection[id].num_params);
			pVal->fn = pgm_read_dword_far(&g_cap_introspection[id].fn);

			return AC_ERROR_OK;
			//debug_string_P(NORMAL,PSTR("\tOK\r\n"));
		} 
		else
		{
			//debug_string_P(NORMAL,PSTR("\tNO\r\n"));
		}
	}
#endif

	pVal->fn = 0;
	pVal->num_params = 0;
	
	return AC_ERROR_GENERIC;
}

RET_ERROR_CODE cap_rtips(float * res,float p1)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"CAP_RTIPS");

#if defined(SETUP_CAP_RAIN)

#ifndef SETUP_RAINGAUGE
	#error "CAP Implementation requires raingauge sensor"
#endif

	
	char dbgSZ[128];
	
	float f = p1;
	uint32_t vi = (uint32_t) f;
	uint16_t vv = (f-vi) * 100;
	sprintf_P(dbgSZ,PSTR("rtips called with param %lu.%d\r\n"),vi,vv);
	debug_string(NORMAL,dbgSZ,RAM_STRING);

	DB_RECORD ds;

	DB_ITERATOR it;
	DB_iterator_get_end(&it);

	DB_iterator_moveback(&it,1);
	DB_get_record(&it,&ds);

	const uint32_t dtEnd = ds.dl_data.data_timestamp - p1;
	sprintf_P(dbgSZ,PSTR("rtips interval %lu - %lu\r\n"),dtEnd,ds.dl_data.data_timestamp);
	debug_string(NORMAL,dbgSZ,RAM_STRING);

	DB_iterator_get_end(&it);


	DB_ITERATOR itBeg;
	DB_iterator_get_begin(&itBeg);

	uint32_t cum = 0;
	uint32_t cc = 0;

	while(true) {
		
		if (it.plain == itBeg.plain)
		{
			debug_string_1P(NORMAL,PSTR("Begin of memory reached\r\n"));
			break;
		}
		
		DB_iterator_moveback(&it,1);
		DB_get_record(&it,&ds);

		if( cc++ == 0 ) {
			sprintf_P(dbgSZ,PSTR("First REC timestamp %lu\r\n"),ds.dl_data.data_timestamp);
			debug_string(NORMAL,dbgSZ,RAM_STRING);
		}
		

		
		if (ds.dl_data.data_timestamp<dtEnd)
		{
			debug_string_1P(NORMAL,PSTR("Minimum date reached\r\n"));
			break;
		}

		cum += ds.raingauge_data[0].tips;

	}

	
	if (res!=NULL)
	{
		*res = (float) cum;
	}
	
	f = *res;
	vi = (uint32_t) f;
	vv = (f-vi) * 100;
	sprintf_P(dbgSZ,PSTR("Num of records inspected %lu\r\nLast Record timestap %lu\r\nRTIPS result %lu.%d\r\n"),cc,ds.dl_data.data_timestamp,vi,vv);
	debug_string(NORMAL,dbgSZ,RAM_STRING);

	
#endif


	return AC_ERROR_OK;
}

RET_ERROR_CODE cap_lev(float * res)
{
#ifdef SETUP_CAP_LEVEL
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"CAP_LEV");


	DB_RECORD ds;
	DB_ITERATOR it;
	DB_ITERATOR itBeg;

	DB_iterator_get_end(&it);
	DB_iterator_get_begin(&itBeg);

	if (it.plain == itBeg.plain)
	{
		debug_string_1P(NORMAL,PSTR("Datalogger memory is empty\r\n"));
		return AC_ERROR_OK;
	}

	DB_iterator_moveback(&it,1);
	DB_get_record(&it,&ds);

#ifdef SETUP_MBXXXX
	if (res!=NULL)
	{
		*res = (float) ds.mbXXXX_data.val;
		//*res = 8;
	}
#elif defined(SETUP_VP61) 
	if (res!=NULL)
	{
		*res = (float) ds.vp61_data.v;	
	}
#else
	#error "CAP Implementation requires level sensor"
#endif
	
	const float f = *res;
	const uint32_t vi = (uint32_t) f;
	const uint16_t vv = (f-vi) * 100;
	char dbgSZ[128];

	sprintf_P(dbgSZ,PSTR("lev result %lu.%d\r\n"),vi,vv);
	debug_string(NORMAL,dbgSZ,RAM_STRING);

#endif // SETUP_CAP_LEVEL

	return AC_ERROR_OK;
}

RET_ERROR_CODE cap_levbias(float * res)
{
#if defined(SETUP_CAP_LEVEL) 
	
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"CAP_LEVBIAS");
	if (res!=NULL)
	{
		*res = 0.0F;
	}
#endif

	return AC_ERROR_OK;
}

RET_ERROR_CODE cap_plcoeff(float * res)
{
#if defined(SETUP_CAP_RAIN)
	
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"CAP_PLCOEFF");
	if (res!=NULL)
	{
		*res = 0.1F;
	}
#endif

	return AC_ERROR_OK;
}


RET_ERROR_CODE cap_consume(const char * const pDoc,const uint16_t lenDoc)
{
	RET_ERROR_CODE e = sim900_GPRS_check_line();
	if(AC_ERROR_OK!=e) {
		debug_string_2P(NORMAL,PSTR("CAP_CONSUME"),PSTR("GPRS line is not OK"));
		return e;
	}

	const uint16_t lenData = sizeof(MQTTPacket_connectData)+256;
	char data[lenData];
	
	e = dl_MQTTconnData_init(data,lenData);
	if ( AC_ERROR_OK != e)	{	return e;  }

	MQTTPacket_connectData * const pConnData = (MQTTPacket_connectData *) data;

	uint16_t TCPBufLen = 0;

	e = dl_MQTT_tcp_open(&TCPBufLen);
	if ( AC_ERROR_OK != e)	{	return e;  }

	char szBuf[LOG_EEPROM_PARTITION_SIZE+2];
	int buflen = sizeof(szBuf);

	int len = MQTTSerialize_connect(szBuf, buflen, data);

	MQTTString topicString = MQTTString_initializer;

	char szTopic[128];
	char szName[32] = {0};
	
	CFG_ITEM_ADDRESS f;
	if ( AC_ERROR_OK != cfg_find_item(CFG_TAG_DATALOGGER_AWS_PRETTYNAME,&f)) {
		strncat(szName,pConnData->clientID.cstring,sizeof(szName));
	} else if(AC_ERROR_OK != cfg_get_item_file(f,szName,sizeof(szName))) {
		strncat(szName,pConnData->clientID.cstring,sizeof(szName));
	}

	snprintf_P(szTopic,sizeof(szTopic),PSTR("ACRONET/STATION/%s/CAP"),szName);
	topicString.cstring = szTopic;

	
	return AC_ERROR_OK;
}


#include "usart.h"

static void dl_test_log_mqtt(void)
{
	static const char __flash msg[] = "This is a test... %d";
	char szBUF[32];
	for (uint8_t i = 0;i<40;++i)
	{
		sprintf_P(szBUF,msg,i);
		
		LOG_say(szBUF);
		is_task_status_ready(task_status_send_log) ;

	}
	
	dl_task_send_log();

	while(1) {
		delay_ms(500);
		hal_status_led_toggle();
	}
	
}

#include "Acronet/drivers/PCAL9554B/PCAL9554B.h"

static void atest(void)
{
	
	
	debug_string_1P(NORMAL,PSTR("Board 3.0 powerswitch test\r\n"));

	hal_PCALXXXX_Init();

	//RET_ERROR_CODE e = hal_PCALXXXX_Init();

	//if (STATUS_OK!=e)
	//{
		//debug_string_1P(NORMAL,PSTR("PCALXXXX_Init falied\r\n"));
	//} else {
		//debug_string_1P(NORMAL,PSTR("PCALXXXX_Init OK\r\n"));
		//PCAL9554B_Write(0x03,0);
		//debug_string_1P(NORMAL,PSTR("PCALXXXX_Write -1- OK\r\n"));
		//delay_ms(500);
		//PCAL9554B_Write(0x01,0xFF);
		//debug_string_1P(NORMAL,PSTR("PCALXXXX_Write -2- OK\r\n"));
	//}


	while(1) {
		delay_ms(500);
		hal_status_led_toggle();
	}
}


static void dl_test_now(void)
{
	//dl_test_sim900_terminal();
	return;
	//char sz[256];
	//
	//AT24CXX_Init();
	//
	//while(1) {
		//uint16_t l = sizeof(sz);
		//hal_board_get_stats(sz,&l);
		//sz[l  ]='\r';
		//sz[l+1]='\n';
		//debug_string(NORMAL,sz,RAM_STRING);
		//delay_ms(2000);
	//}
	//
	//return;
	
	dl_test_sim900_terminal();
	//atest();
//	dl_test_log_mqtt();
//	goto lbl_cap_test;
	return;
	hal_PCALXXXX_Init();

	const uint8_t cmd_port0_cfg = 6;
	const uint8_t cmd_port1_cfg = 7;

	uint8_t data = 0b01101111;

/////////////////////////////////////////////////////////////
///////// PORT0 INPUT/OUTPUT PIN CONFIG
	if(AC_ERROR_OK != PCAL9535A_Write(cmd_port0_cfg,data) )
	{
		debug_string(NORMAL,PSTR("ALLA PRIMA\r\n"),PGM_STRING);
		goto looppp;
	}
	delay_ms(5);

/////////////////////////////////////////////////////////////
///////// PORT0 PULLUP/PULLDOWN ENABLE PIN CONFIG
	if(AC_ERROR_OK != PCAL9535A_Write(0x46,0b00000111) )
	{
		debug_string(NORMAL,PSTR("ALLA PRIMA\r\n"),PGM_STRING);
		goto looppp;
	}
	delay_ms(5);
/////////////////////////////////////////////////////////////
///////// PORT0 CHOOSE PULLUP OR PULLDOWN  PIN CONFIG
	if(AC_ERROR_OK != PCAL9535A_Write(0x48,0b00000111) )
	{
		debug_string(NORMAL,PSTR("ALLA PRIMA\r\n"),PGM_STRING);
		goto looppp;
	}
	delay_ms(5);

/////////////////////////////////////////////////////////////
///////// PORT0 OUTPUT WRITE

	data = 0;
	if(AC_ERROR_OK != PCAL9535A_Write(0x2,data) )
	{
		debug_string(NORMAL,PSTR("ALLA PRIMA -2-\r\n"),PGM_STRING);
		goto looppp;
	}
	delay_ms(5);

/////////////////////////////////////////////////////////////
///////// PORT0 INPUTS READ
	uint8_t val = 0;
	if(AC_ERROR_OK != PCAL9535A_Read(0,&val) )
	{
		debug_string(NORMAL,PSTR("ALLA PRIMA\r\n"),PGM_STRING);
		goto looppp;
	}
	delay_ms(5);

	debug_string_1P(NORMAL,PSTR("POWER INPUT MASK"));

	char szBuf[8];
	szBuf[0] = (val & 4) ? '1' : '0';
	szBuf[1] = (val & 2) ? '1' : '0';
	szBuf[2] = (val & 1) ? '1' : '0';
	szBuf[3] = '\r';
	szBuf[4] = '\n';
	szBuf[5] = 0;

	debug_string(NORMAL,szBuf,RAM_STRING);

	//while (1)
	//{
		//nop();
	//}

	
///////////////////////////////////////////////////////////
///////// PORT1 CONFIG
	data = 0b11110000;
	if(AC_ERROR_OK != PCAL9535A_Write(cmd_port1_cfg,data) )
	{
		debug_string(NORMAL,PSTR("ALLA SECONDA\r\n"),PGM_STRING);
		goto looppp;
	}
	
	delay_ms(5);

	//ioport_configure_pin(CV7L_PIN_TX_ENABLE, IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
	//ioport_configure_pin(L8095N_PIN_TX_ENABLE, IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
//
	//ioport_configure_pin(CV7L_PIN_TX_SIGNAL, IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
	//ioport_configure_pin(CV7L_PIN_RX_SIGNAL, IOPORT_DIR_INPUT);
//
	//ioport_configure_pin(L8095N_PIN_TX_SIGNAL, IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
	//ioport_configure_pin(L8095N_PIN_RX_SIGNAL, IOPORT_DIR_INPUT);
//
	//
	//static usart_rs232_options_t usart_options = {
		//.baudrate = 4800,
		//.charlength = USART_CHSIZE_8BIT_gc,
		//.paritytype = USART_PMODE_DISABLED_gc,
		//.stopbits = false
	//};
	//
////	usart_serial_init(CV7L_PUSART, &usart_options);
	//usart_serial_init(L8095N_PUSART, &usart_options);
//
////	usart_rx_disable(CV7L_PUSART);
	//usart_rx_enable(L8095N_PUSART);
	//
//
	//while (1)
	//{
		//char c =  usart_getchar(L8095N_PUSART);
		//usart_putchar(USART_DEBUG,c);
		//status_led_toggle();
	//}

	while(1) {

		status_led_toggle();
		if(AC_ERROR_OK != PCAL9535A_Write(3,0b11111111) )
		{
			debug_string(NORMAL,PSTR("ALLA TERZA\r\n"),PGM_STRING);
			goto looppp;
		}
		delay_ms(1000);

		status_led_toggle();
		if(AC_ERROR_OK != PCAL9535A_Write(3,0b00000000) )
		{
			debug_string(NORMAL,PSTR("ALLA QUARTA\r\n"),PGM_STRING);
			goto looppp;
		}
		delay_ms(1000);
	}
	
looppp:

	while(1) {
		nop();
	}
	
	return;

	////g_log_verbosity = VERY_VERBOSE;
////

	USART_data_t * ud = SIM900_get_PUSART();
	usart_rx_enable(USART_DEBUG);

	while (1)
	{

		if(USART_RX_CBuffer_Data_Available(ud)) 
		{
			status_led_toggle();
			char c =  USART_RX_CBuffer_GetByte(ud);
			usart_putchar(USART_DEBUG,c);
		}

		if(usart_rx_is_complete(USART_DEBUG) == true)
		{
			status_led_toggle();
			char c =  usart_get(USART_DEBUG);
			usart_putchar(USART_GPRS,c);
			usart_putchar(USART_DEBUG,c);
			if (c=='\r')
			{
				usart_putchar(USART_DEBUG,'\n');
			}
		}
	}

	//LOG_reset_test();

	LOG_init();

	AT24CXX_iterator b = { .plain = LOG_EEPROM_PARTITION_BEGIN };
	AT24CXX_iterator e = { .plain = LOG_EEPROM_PARTITION_END };
	DB_dump(b,e);

	while(1) {	
		dl_task_make_statistics();
		LOG_process_buffer();
		//AT24CXX_iterator_report(g_log_iter_beg);
		//AT24CXX_iterator_report(g_log_iter_end);
		
		AT24CXX_iterator b = { .plain = LOG_EEPROM_PARTITION_BEGIN };
		AT24CXX_iterator e = { .plain = LOG_EEPROM_PARTITION_END };
		//DB_dump(b,e);
		dl_MQTT_sendFromDatastore_2(DATASTORE_LOG);
		//DB_dump(b,e);
		usart_getchar(USART_DEBUG);
	}
	
	
	
	return;
lbl_cap_test:
	cap_test();
	while(1) {	status_led_toggle();delay_ms(100); }

	return;
	//g_log_verbosity = VERY_VERBOSE;
	
	fw_update_init(0);
	fw_update_run_test(0);
	
	while(1) {	status_led_toggle();delay_ms(100); }

}


void dl_dump_db(void)
{
	DB_ITERATOR itBeg;
	DB_ITERATOR itEnd;

	
	DB_init(sizeof(DB_RECORD));

	DB_iterator_get_begin(&itBeg);
	DB_iterator_get_end(&itEnd);
	
	if (itBeg.plain > itEnd.plain) {
		DB_ITERATOR itt;
		
		itt.plain = DB_EEPROM_PARTITION_END;
		DB_dump(itBeg,itt);
		itt.plain = DB_EEPROM_PARTITION_BEGIN;
		DB_dump(itt,itEnd);

		} else {
		DB_dump(itBeg,itEnd);
	}
	
}


void dl_dump_db2(void)
{

	char szBuf[1024];

	DB_ITERATOR itBeg;
	DB_ITERATOR itEnd;
	DB_ITERATOR itt;
	
	DB_init(sizeof(DB_RECORD));

	DB_iterator_get_begin(&itBeg);
	DB_iterator_get_end(&itEnd);

	if (itEnd.plain == itBeg.plain)
	{
		debug_string_1P(NORMAL,PSTR("Database is empty\r\n"));
		return;
	}

	itt.plain = itEnd.plain;
	DB_iterator_moveback(&itt,1);
	
	DB_RECORD ds;

	int16_t lb;
	
	while(itt.plain!=itBeg.plain) {

		DB_get_record(&itt,&ds);

		szBuf[0] = '\r';
		szBuf[1] = '\n';

		int16_t szEnd = 2;

		for(MODULE_ID  im=DL_MODULE_BEG;im<DL_MODULE_END;++im)
		{
			DATA2STRING fn_stat2string = iface_module[im].pFN_Data2String;

//			const size_t off = selectDataSource(im);
//			if ((off==0xFFFF) || (fn_stat2string==NULL))
			if (fn_stat2string==NULL)
			{
				debug_string_1P(NORMAL,PSTR("[WARNING] Data2String NULL method, skip"));
				continue;
			}
			
			fn_stat2string( &ds , szBuf+szEnd , &lb);
			szEnd += lb;
			lb = sizeof(szBuf) - szEnd;
		}

		//DATA2STRING fn_stat2string = iface_module[DL_INTERNAL].pFN_Data2String;
		//const size_t off = selectDataSource(DL_INTERNAL);
		//fn_stat2string( off+(uint8_t*)&ds , szBuf+szEnd , &lb);
		
		szBuf[sizeof(szBuf)-1]=0;
		
		debug_string(NORMAL,szBuf,RAM_STRING);
		
		DB_iterator_moveback(&itt,1);
	}
}
