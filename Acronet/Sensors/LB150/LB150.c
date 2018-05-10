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

#include <asf.h>


#include "Acronet/globals.h"
#include <stdio.h>
#include <conf_board.h>
#include <conf_usart_serial.h>
#include "config/conf_usart_serial.h"
#include "Acronet/channels/NMEA/nmea.h"
#include "Acronet/drivers/SP336/SP336.h"
#include "Acronet/Sensors/LB150/LB150.h"
#include "Acronet/services/config/config.h"
#include "Acronet/drivers/StatusLED/status_led.h"
#include "Acronet/drivers/UART_INT/cbuffer_usart.h"


/*

Sentence	| Description							| Enabled by Default	| Maximum Length(chars)
------------+---------------------------------------+-----------------------+-----------------------
$GPDTM		| Datum Reference						|	No					|	47
------------+---------------------------------------+-----------------------+-----------------------
$GPGGA		| GPS Fix Data							|	YES					|	82
------------+---------------------------------------+-----------------------+-----------------------
$GPGLL		| Geographic Position – Lat/Lon			|	No					|	48
------------+---------------------------------------+-----------------------+-----------------------
$GPGSA		| GNSS DOP and Active Satellites		|	No					|	66
------------+---------------------------------------+-----------------------+-----------------------
$GPGSV		| GNSS Satellites in View				|	No					|	70
------------+---------------------------------------+-----------------------+-----------------------
$HCHDG		| Heading, Deviation and Variation		|   No					|	33
------------+---------------------------------------+-----------------------+-----------------------
$HCHDT		| Heading relative to True North		|   YES					|	19
------------+---------------------------------------+-----------------------+-----------------------
$WIMDA		| Meteorological Composite.				|   YES					|   81
			| Barometric pressure, air temperature,	|						|
			| relative humidity, dew point,			|						|
			| wind direction,wind speed				|						|
------------+---------------------------------------+-----------------------+-----------------------
$WIMWD		| Wind Direction and Speed,				|	NO					|   41
			| with respect to north					|						|
------------+---------------------------------------+-----------------------+-----------------------
$WIMWV		|Wind Speed and Angle, in relation		|	YES					|	28
(relative)	|to the centerline of the front			|						|
			|of the vehicle (relative)				|						|
------------+---------------------------------------+-----------------------+-----------------------
$WIMWV		|Wind Speed and Angle, in relation		|	No					|	28
(theoret.)	|to the centerline of the front			|						|
			|of the vehicle	(theoretical)			|						|
------------+---------------------------------------+-----------------------+-----------------------
$GPRMC		|Recommended Minimum Specific GNSS Data	|	No					|	74
------------+---------------------------------------+-----------------------+-----------------------
$GPVTG		|Course Over Ground and Ground Speed	|	YES					|	42
------------+---------------------------------------+-----------------------+-----------------------
$WIVWR		|Relative Wind Speed and Angle			|	No					|	41
------------+---------------------------------------+-----------------------+-----------------------
$WIVWT		|True Wind Speed and Angle				|	No					|	41
------------+---------------------------------------+-----------------------+-----------------------
$YXXDR		|Transducer Measurements: wind chill,	|	YES					|	74
(type A)	|heat index, and station pressure		|						|
------------+---------------------------------------+-----------------------+-----------------------
$YXXDR		|Transducer Measurements:				|	YES					|	43
(type B)	|vehicle/instrument attitude			|						|
			|(pitch and roll)						|						|
------------+---------------------------------------+-----------------------+-----------------------
$GPZDA		|Time and Date							|	YES					|	38


*/

#define NMEA_SENTENCE_MAX_LENGTH 90


// enum {	LB150_STAT_BEG=0,
// 	LB150_PRESSURE=LB150_STAT_BEG,
// 	LB150_TEMPERATURE,
// 	LB150_TEMPERATURE_MAX,
// 	LB150_TEMPERATURE_MIN,
// 	LB150_RH,
// 	LB150_DEWPOINT,
// 	LB150_WINDIR,
// 	LB150_WINDIR_GUST,
// 	LB150_WINSPEED,
// 	LB150_WINSPEED_GUST,
// LB150_STAT_END};

enum {	LB150_STAT_OPERATOR_MEAN
	   ,LB150_STAT_OPERATOR_MAX
	   ,LB150_STAT_OPERATOR_MIN
	   ,LB150_STAT_OPERATOR_GUST_DIR
	   ,LB150_STAT_OPERATOR_GUST_SPEED
	   ,LB150_STAT_OPERATOR_END	};


typedef struct
{
	float   g_data   [LB150_STAT_END];
	uint8_t g_samples[LB150_STAT_END];

	
} LB150_PRIVATE_DATA;


typedef struct {
	char m_fmt[8];
	uint16_t m_fct;
	uint8_t m_oper;
} LB150_STAT_FORMAT;


static const LB150_STAT_FORMAT g_stat_fmt[LB150_STAT_END]  PROGMEM = {	 {"Pav=%d",  1000 ,	LB150_STAT_OPERATOR_MEAN}
																		,{"Tav=%d",   100 ,	LB150_STAT_OPERATOR_MEAN}
																		,{"Tmx=%d",   100 ,	LB150_STAT_OPERATOR_MAX }
																		,{"Tmn=%d",   100 ,	LB150_STAT_OPERATOR_MIN }
																		,{"RH=%d",    100 ,	LB150_STAT_OPERATOR_MEAN}
																		,{"DP=%d",    100 ,	LB150_STAT_OPERATOR_MEAN}
																		,{"Wdav=%d",   10 ,	LB150_STAT_OPERATOR_MEAN}
																		,{"Wdmx=%d",   10 ,	LB150_STAT_OPERATOR_GUST_DIR}																		
																		,{"Wsav=%d",   10 ,	LB150_STAT_OPERATOR_MEAN}
																		,{"Wsmx=%d",   10 ,	LB150_STAT_OPERATOR_GUST_SPEED} 
#ifdef LB150_ENABLE_GPS
																		,{"Lat=%ld" , 100,	LB150_STAT_OPERATOR_MEAN}
																		,{"Lon=%ld" , 100,	LB150_STAT_OPERATOR_MEAN}
#endif																			
																	};



static const char szNMEA_GPDTM[]		PROGMEM = "$GPDTM";
static const char szNMEA_GPGGA[]		PROGMEM = "$GPGGA";
static const char szNMEA_GPGLL[]		PROGMEM = "$GPGLL";
static const char szNMEA_GPGSA[]		PROGMEM = "$GPGSA";

static const char szNMEA_GPGSV[]		PROGMEM = "$GPGSV";
static const char szNMEA_HCHDG[]		PROGMEM = "$HCHDG";
static const char szNMEA_HCHDT[]		PROGMEM = "$HCHDT";
static const char szNMEA_WIMDA[]		PROGMEM = "$WIMDA";

static const char szNMEA_WIMWD[]		PROGMEM = "$WIMWD";
static const char szNMEA_WIMWV[]		PROGMEM = "$WIMWV";
static const char szNMEA_GPRMC[]		PROGMEM = "$GPRMC";
static const char szNMEA_GPVTG[]		PROGMEM = "$GPVTG";

static const char szNMEA_WIVWR[]		PROGMEM = "$WIVWR";
static const char szNMEA_WIVWT[]		PROGMEM = "$WIVWT";
static const char szNMEA_YXXDR[]		PROGMEM = "$YXXDR";
static const char szNMEA_GPZDA[]		PROGMEM = "$GPZDA";


static const char * const tbl_NMEAin[] PROGMEM = {	
													szNMEA_GPDTM,
													szNMEA_GPGGA,
													szNMEA_GPGLL,
													szNMEA_GPGSA,
													szNMEA_GPGSV,
													szNMEA_HCHDG,
													szNMEA_HCHDT,
													szNMEA_WIMDA,
													szNMEA_WIMWD,
													szNMEA_WIMWV,
													szNMEA_GPRMC,
													szNMEA_GPVTG,
													szNMEA_WIVWR,
													szNMEA_WIVWT,
													szNMEA_YXXDR,
													szNMEA_GPZDA
												};


enum {	NMEA_FIRST_ENTRY = 0, 
		NMEA_GPDTM = NMEA_FIRST_ENTRY,
		NMEA_GPGGA,	NMEA_GPGLL,	NMEA_GPGSA,	
		NMEA_GPGSV,	NMEA_HCHDG,	NMEA_HCHDT,	
		NMEA_WIMDA,	NMEA_WIMWD,	NMEA_WIMWV,	
		NMEA_GPRMC,	NMEA_GPVTG,	NMEA_WIVWR,	
		NMEA_WIVWT,	NMEA_YXXDR,	NMEA_GPZDA,
		NMEA_LAST_ENTRY
	};

#define NUM_OF_NMEA_in (sizeof(tbl_NMEAin)/sizeof(char *))

static void LB150_NMEA_Handler_GPDTM(LB150_PRIVATE_DATA * const pSelf,char * const psz);
static void LB150_NMEA_Handler_GPGGA(LB150_PRIVATE_DATA * const pSelf,char * const psz);
static void LB150_NMEA_Handler_GPGLL(LB150_PRIVATE_DATA * const pSelf,char * const psz);
static void LB150_NMEA_Handler_GPGSA(LB150_PRIVATE_DATA * const pSelf,char * const psz);
static void LB150_NMEA_Handler_GPGSV(LB150_PRIVATE_DATA * const pSelf,char * const psz);
static void LB150_NMEA_Handler_HCHDG(LB150_PRIVATE_DATA * const pSelf,char * const psz);
static void LB150_NMEA_Handler_HCHDT(LB150_PRIVATE_DATA * const pSelf,char * const psz);
static void LB150_NMEA_Handler_WIMDA(LB150_PRIVATE_DATA * const pSelf,char * const psz);
static void LB150_NMEA_Handler_WIMWD(LB150_PRIVATE_DATA * const pSelf,char * const psz);
static void LB150_NMEA_Handler_WIMWV(LB150_PRIVATE_DATA * const pSelf,char * const psz);
static void LB150_NMEA_Handler_GPRMC(LB150_PRIVATE_DATA * const pSelf,char * const psz);
static void LB150_NMEA_Handler_GPVTG(LB150_PRIVATE_DATA * const pSelf,char * const psz);
static void LB150_NMEA_Handler_WIVWR(LB150_PRIVATE_DATA * const pSelf,char * const psz);
static void LB150_NMEA_Handler_WIVWT(LB150_PRIVATE_DATA * const pSelf,char * const psz);
static void LB150_NMEA_Handler_YXXDR(LB150_PRIVATE_DATA * const pSelf,char * const psz);
static void LB150_NMEA_Handler_GPZDA(LB150_PRIVATE_DATA * const pSelf,char * const psz);

typedef void (*NMEA_FN_HANDLER)(LB150_PRIVATE_DATA * const ,char * const);

static const NMEA_FN_HANDLER tbl_NMEAfn[] PROGMEM = {
												LB150_NMEA_Handler_GPDTM,
												LB150_NMEA_Handler_GPGGA,
												LB150_NMEA_Handler_GPGLL,
												LB150_NMEA_Handler_GPGSA,
												LB150_NMEA_Handler_GPGSV,
												LB150_NMEA_Handler_HCHDG,
												LB150_NMEA_Handler_HCHDT,
												LB150_NMEA_Handler_WIMDA,
												LB150_NMEA_Handler_WIMWD,
												LB150_NMEA_Handler_WIMWV,
												LB150_NMEA_Handler_GPRMC,
												LB150_NMEA_Handler_GPVTG,
												LB150_NMEA_Handler_WIVWR,
												LB150_NMEA_Handler_WIVWT,
												LB150_NMEA_Handler_YXXDR,
												LB150_NMEA_Handler_GPZDA
												};
	

RET_ERROR_CODE LB150_Data2String(const LB150_DATA * const st,char * const sz, int16_t * const len_sz)
{
	int16_t len = 0;
	char sBuf[12] = "&";

	char * psz = sBuf;
	char * const p = sBuf+1;

	for (uint8_t ix=LB150_STAT_BEG;ix<LB150_STAT_END;++ix)
	{

		LB150_STAT_FORMAT af;

		nvm_flash_read_buffer((flash_addr_t)&g_stat_fmt[ix],&af,sizeof(LB150_STAT_FORMAT));
		strncpy(p,af.m_fmt,sizeof(af.m_fmt));

		
		float vf = st->m_stat[ix];
		if(-9999.0F!=vf) { vf*=af.m_fct; }
		const int32_t vi = (int32_t) vf;
		len += snprintf(sz+len,max(0,*len_sz-len),psz,vi);
		if(len>=*len_sz) {return AC_BUFFER_OVERFLOW;}
		//psz = sBuf;
	}
	

	*len_sz = len;

	return AC_ERROR_OK;
}

//#define SWITCH_PIN					IOPORT_CREATE_PIN(PORTD, 1)

static void	LB150_powercycle(void)
{
//	return;
	//gpio_toggle_pin(SWITCH_PIN);
	//delay_ms(3000);
	//gpio_toggle_pin(SWITCH_PIN);
	//delay_ms(3000);
}



static void LB150_process_NMEA_Statement(LB150_PRIVATE_DATA * const pSelf, char * const psz)
{
	const uint8_t le = NMEA_LAST_ENTRY;

	for(uint8_t i = NMEA_FIRST_ENTRY;i<le;++i)	{
		char * p = (char *) nvm_flash_read_word( (flash_addr_t) (tbl_NMEAin+i) );
		if (0==strncasecmp_P(psz,p,6))
		{
			NMEA_FN_HANDLER fn = nvm_flash_read_word( (flash_addr_t) (tbl_NMEAfn+i) );
			fn(pSelf,psz);
		}
	}
}

/*
bool LB150_Yield( void )
{
	static char szBuf[128];
	static uint8_t idx = 0;
	char c;
	while((c = NMEALine_getChar()) != 0)
	{
		if (c=='\r')	{
			szBuf[idx]=0;
			if(0==NMEALine_checksum_check(szBuf,idx)) {
				LB150_process_NMEA_Statement(szBuf);
			}
			idx=0;
		} else if (c=='$') {
			szBuf[0]='$';
			szBuf[1]=0;
			idx = 1;
		} else if(idx>0) {
			szBuf[idx++] = c;
		}

		if (idx==(sizeof(szBuf)-1)) 
		{
			debug_string_2P(NORMAL,PSTR("LB150_Yield"),PSTR("Synchronization lost"));
			szBuf[0]=0;
			idx = 0;
		}
		
		return true;
	}
	return false;
}
*/

static void LB150_NMEA_Handler_GPDTM(LB150_PRIVATE_DATA * const pSelf,char * const psz)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"NMEA_Handler_GPDTM (empty)");
}

static void LB150_NMEA_Handler_GPGGA(LB150_PRIVATE_DATA * const pSelf,char * const psz)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"NMEA_Handler_GPGGA (empty)");
}



static void LB150_NMEA_Handler_GPGSA(LB150_PRIVATE_DATA * const pSelf,char * const psz)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"NMEA_Handler_GPGSA (empty)");
}

static void LB150_NMEA_Handler_GPGSV(LB150_PRIVATE_DATA * const pSelf,char * const psz)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"NMEA_Handler_GPGSV (empty)");
}

static void LB150_NMEA_Handler_HCHDG(LB150_PRIVATE_DATA * const pSelf,char * const psz)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"NMEA_Handler_HCHDG (empty)");
}

static void LB150_NMEA_Handler_HCHDT(LB150_PRIVATE_DATA * const pSelf,char * const psz)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"NMEA_Handler_HCHDT (empty)");
}


static float LB150_compute_stats(LB150_PRIVATE_DATA * const pSelf,const uint8_t id)
{
	if (pSelf->g_samples[id]==0) { return -9999.0F; }
	

	const uint8_t op = nvm_flash_read_byte( (flash_addr_t) &g_stat_fmt[id].m_oper);
	
	if(op==LB150_STAT_OPERATOR_MEAN) {
		return pSelf->g_data[id] / pSelf->g_samples[id];
	} 
	
	return pSelf->g_data[id];
}


static RET_ERROR_CODE LB150_get_data(LB150_PRIVATE_DATA * const pSelf,LB150_DATA * const ps)
{
	for(uint8_t ix = LB150_STAT_BEG;ix<LB150_STAT_END;++ix)
	{
		ps->m_stat[ix] = LB150_compute_stats(pSelf,ix);
	}

	return AC_ERROR_OK;
}


static RET_ERROR_CODE LB150_reset_data(LB150_PRIVATE_DATA * const pSelf)
{

	for(uint8_t ix = LB150_STAT_BEG;ix<LB150_STAT_END;++ix)
	{
		pSelf->g_samples[ix] = 0;
	}


	return AC_ERROR_OK;
}

static void LB150_NMEA_UpdateStats(LB150_PRIVATE_DATA * const pSelf,const uint8_t id,const char * const p)
{
	const float val = atof(p);
	uint8_t op;

	op = nvm_flash_read_byte( (flash_addr_t) &g_stat_fmt[id].m_oper);

	static float gust_temp = -9999.0F;
	
	if(op==LB150_STAT_OPERATOR_GUST_DIR) {
		gust_temp = val;
		return;
	}
	
	if(op==LB150_STAT_OPERATOR_MEAN) {
		
		if(pSelf->g_samples[id]==0) {
			pSelf->g_data[id]=val;
		} else {
			pSelf->g_data[id]+=val;
		}
		
	} else if(op==LB150_STAT_OPERATOR_MAX) {
		
		if(pSelf->g_samples[id]==0) {
			pSelf->g_data[id]=val;
		} else {
			if(pSelf->g_data[id]<val) {pSelf->g_data[id]=val;}
		}
		
	} else if(op==LB150_STAT_OPERATOR_MIN) {

		if(pSelf->g_samples[id]==0) {
			pSelf->g_data[id]=val;
		} else {
			if(pSelf->g_data[id]>val) {pSelf->g_data[id]=val;}
		}

	} else if(op==LB150_STAT_OPERATOR_GUST_SPEED) {
		if(pSelf->g_samples[id]==0) {
			pSelf->g_data[id]=val;
		} else {
			if(pSelf->g_data[id]<val) {pSelf->g_data[id]=val;}
		}
		pSelf->g_data[LB150_STAT_WINDIR_GUST] = gust_temp;
		pSelf->g_samples[LB150_STAT_WINDIR_GUST] = 1;

	}

	pSelf->g_samples[id]++;



//  	char szBuf[128];
//  	sprintf_P(szBuf,PSTR("Update stat %d -> %s\r\n"),id,p);
//  	debug_string(NORMAL,szBuf,RAM_STRING);

}

static void LB150_process_Barometric_pressure(LB150_PRIVATE_DATA * const pSelf,const char * const p)
{
	LB150_NMEA_UpdateStats(pSelf,LB150_STAT_PRESSURE,p);
}

static void LB150_process_temperature(LB150_PRIVATE_DATA * const pSelf,const char * const p)
{
	LB150_NMEA_UpdateStats(pSelf,LB150_STAT_TEMPERATURE,p);
	LB150_NMEA_UpdateStats(pSelf,LB150_STAT_TEMPERATURE_MAX,p);
	LB150_NMEA_UpdateStats(pSelf,LB150_STAT_TEMPERATURE_MIN,p);
}

static void LB150_process_RH(LB150_PRIVATE_DATA * const pSelf,const char * const p)
{
	LB150_NMEA_UpdateStats(pSelf,LB150_STAT_RH,p);
}

static void LB150_process_DEWPoint(LB150_PRIVATE_DATA * const pSelf,const char * const p)
{
	LB150_NMEA_UpdateStats(pSelf,LB150_STAT_DEWPOINT,p);
}

static void LB150_process_WinDir(LB150_PRIVATE_DATA * const pSelf,const char * const p)
{
	LB150_NMEA_UpdateStats(pSelf,LB150_STAT_WINDIR,p);
	LB150_NMEA_UpdateStats(pSelf,LB150_STAT_WINDIR_GUST,p);
	
}

static void LB150_process_WindSpeed(LB150_PRIVATE_DATA * const pSelf,const char * const p)
{
	LB150_NMEA_UpdateStats(pSelf,LB150_STAT_WINSPEED,p);
	LB150_NMEA_UpdateStats(pSelf,LB150_STAT_WINSPEED_GUST,p);
}

static void LB150_NMEA_Handler_WIMDA(LB150_PRIVATE_DATA * const pSelf,char * const psz)
{
//	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"NMEA_Handler_WIMDA");

//  	debug_string(NORMAL,psz,RAM_STRING);
//  	debug_string_P(NORMAL,szCRLF);

	char * p = psz+7;
	char * q;
	uint8_t v;
	v = NMEALine_Tokenize(p,&q); //Barometric pressure, inches of mercury
	v = NMEALine_Tokenize(q,&p); //Barometric pressure, name of dimension ( I = inches)

	v = NMEALine_Tokenize(p,&q); //Barometric pressure, bars, to the nearest .001 bar
	if(v!=0) {LB150_process_Barometric_pressure(pSelf,p);}
	v = NMEALine_Tokenize(q,&p); //Barometric pressure, name of dimension ( B = bars)

	v = NMEALine_Tokenize(p,&q); //Air temperature, degrees C, to the nearest 0.1 degree C
	if(v!=0) {LB150_process_temperature(pSelf,p);}
	v = NMEALine_Tokenize(q,&p); //Air temperature, C = degrees C

	v = NMEALine_Tokenize(p,&q); //Water temperature, degrees C (this field left blank by WeatherStation)
	v = NMEALine_Tokenize(q,&p); //C = degrees C (this field left blank by WeatherStation)
	
	v = NMEALine_Tokenize(p,&q); //Relative humidity, percent, to the nearest 0.1 percent
	if(v!=0) {LB150_process_RH(pSelf,p);}

	v = NMEALine_Tokenize(q,&p); //Absolute humidity, percent (this field left blank by WeatherStation)
	
	v = NMEALine_Tokenize(p,&q); //Dew point, degrees C, to the nearest 0.1 degree C
	if(v!=0) {LB150_process_DEWPoint(pSelf,p);}
	v = NMEALine_Tokenize(q,&p); //C = degrees C

// 	v = NMEALine_Tokenize(p,&q); //Wind direction, degrees True, to the nearest 0.1 degree
// 	if(v!=0) {LB150_NMEA_UpdateStats(LB150_WINDIR,p);}
// 	v = NMEALine_Tokenize(q,&p); //T = true
// 
// 	v = NMEALine_Tokenize(p,&q); //Wind direction, degrees Magnetic, to the nearest 0.1 degree
// 	v = NMEALine_Tokenize(q,&p); //M = magnetic
// 
// 	v = NMEALine_Tokenize(p,&q); //Wind speed, knots, to the nearest 0.1 knot
// 	v = NMEALine_Tokenize(q,&p); //N = knots
// 
// 	v = NMEALine_Tokenize(p,&q); //Wind speed, meters per second, to the nearest 0.1 m/s
// 	if(v!=0) {LB150_NMEA_UpdateStats(LB150_WINSPEED,p);}
// 	v = NMEALine_Tokenize(q,&p); //M = meters per second

		
}

static void LB150_NMEA_Handler_WIMWD(LB150_PRIVATE_DATA * const pSelf,char * const psz)
{
//	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"NMEA_Handler_WIMWD");
//	debug_string(NORMAL,psz,RAM_STRING);

	char * p = psz+7;
	char * q;
	uint8_t v;

	v = NMEALine_Tokenize(p,&q); //Wind direction, degrees True, to the nearest 0.1 degree
	if(v!=0) {LB150_process_WinDir(pSelf,p);}
	v = NMEALine_Tokenize(q,&p); //T = true

	v = NMEALine_Tokenize(p,&q); //Wind direction, degrees Magnetic, to the nearest 0.1 degree
	v = NMEALine_Tokenize(q,&p); //M = magnetic

	v = NMEALine_Tokenize(p,&q); //Wind speed, knots, to the nearest 0.1 knot
	v = NMEALine_Tokenize(q,&p); //N = knots

	v = NMEALine_Tokenize(p,&q); //Wind speed, meters per second, to the nearest 0.1 m/s
	if(v!=0) {LB150_process_WindSpeed(pSelf,p);}
	v = NMEALine_Tokenize(q,&p); //M = meters per second

}

static void LB150_NMEA_Handler_WIMWV(LB150_PRIVATE_DATA * const pSelf,char * const psz)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"NMEA_Handler_WIMWV (empty)");
}

static void LB150_NMEA_Handler_GPRMC(LB150_PRIVATE_DATA * const pSelf,char * const psz)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"NMEA_Handler_GPRMC (empty)");
}

static void LB150_NMEA_Handler_GPVTG(LB150_PRIVATE_DATA * const pSelf,char * const psz)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"NMEA_Handler_GPVTG (empty)");
}

static void LB150_NMEA_Handler_WIVWR(LB150_PRIVATE_DATA * const pSelf,char * const psz)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"NMEA_Handler_WIVWR (empty)");
}

static void LB150_NMEA_Handler_WIVWT(LB150_PRIVATE_DATA * const pSelf,char * const psz)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"NMEA_Handler_WIVWT (empty)");
}

static void LB150_NMEA_Handler_YXXDR(LB150_PRIVATE_DATA * const pSelf,char * const psz)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"NMEA_Handler_YXXDR (empty)");
}

static void LB150_NMEA_Handler_GPZDA(LB150_PRIVATE_DATA * const pSelf,char * const psz)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"NMEA_Handler_GPZDA (empty)");
}

#ifdef LB150_ENABLE_GPS
static void LB150_process_GPS_Lat( LB150_PRIVATE_DATA * const pSelf,char * const p )
{
	LB150_NMEA_UpdateStats(pSelf,LB150_STAT_LATITUDE,p);
}

static void LB150_process_GPS_Lon( LB150_PRIVATE_DATA * const pSelf,char * const p )
{
	LB150_NMEA_UpdateStats(pSelf,LB150_STAT_LONGITUDE,p);
}
#endif


static void LB150_NMEA_Handler_GPGLL(LB150_PRIVATE_DATA * const pSelf,char * const psz)
{
#ifdef LB150_ENABLE_GPS
	
	//DEBUG_PRINT_FUNCTION_NAME(NORMAL,"NMEA_Handler_GPGLL (empty)");
	
	char * p = psz+7;
	char * q;
	uint8_t v;

	//debug_string(NORMAL,p,RAM_STRING);

	v = NMEALine_Tokenize(p,&q); //Latitude, to the nearest .0001 minute
	if(v!=0) {LB150_process_GPS_Lat(pSelf,p);}
	v = NMEALine_Tokenize(q,&p); //N = north ; S = South

	v = NMEALine_Tokenize(p,&q); //Longitude,  to the nearest .0001 minute
	if(v!=0) {LB150_process_GPS_Lon(pSelf,p);}
	v = NMEALine_Tokenize(q,&p); //E = east ; W = west

	v = NMEALine_Tokenize(p,&q); //UTC of position, in the form hhmmss
	v = NMEALine_Tokenize(q,&p); //Status A = data valid ; V = data invalid

	v = NMEALine_Tokenize(p,&q);	//Mode : A = Autonomous ; D = differential ; E = estimated
	// M = manual ; S = simulator ; N = data not valid
	
#endif //LB150_ENABLE_GPS
}

#define MODULE_INTERFACE_PRIVATE_DATATYPE LB150_PRIVATE_DATA


#define MODINST_PARAM_ID MOD_ID_LB150
#include "Acronet/datalogger/modinst/module_interface_definition.h"
#undef MODINST_PARAM_ID