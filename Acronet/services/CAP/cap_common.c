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

#include "Acronet/setup.h"
#include "Acronet/HAL/hal_interface.h"

#include <asf.h>


#include "Acronet/globals.h"
#include <stdio.h>
#include <conf_board.h>

#include "Acronet/drivers/SIM/sim900.h"
#include "Acronet/services/config/config.h"
#include "cap_common.h"
#include "cap_consumer.h"

#define EXP_MAX_LEN 32
#define EXP_MAX_PARAMS 4

const __flash CAP_ALERT g_alert[CAP_TYPE_END] = {
	{CAP_OP_TYPE_WATERLEV,"%LEV() %LEVBIAS() -",1.0F,0.8F,CAP_URGENCY_EXPECTED,CAP_SEVERITY_MODERATE,0},
	{CAP_OP_TYPE_WATERLEV,"%LEV() %LEVBIAS() -",2.0F,1.8F,CAP_URGENCY_IMMEDIATE,CAP_SEVERITY_SEVERE,0},
	{CAP_OP_TYPE_WATERLEV,"%LEV() %LEVBIAS() -",3.0F,2.8F,CAP_URGENCY_IMMEDIATE,CAP_SEVERITY_EXTREME,0},
	{CAP_OP_TYPE_CUMUL_1HR ,"%RTIPS(3600) %PLCOEFF() *",30.0F,29.0F,CAP_URGENCY_EXPECTED,CAP_SEVERITY_MODERATE,0},
	{CAP_OP_TYPE_CUMUL_3HR ,"%RTIPS(10800) %PLCOEFF() *",90.0F,89.0F,CAP_URGENCY_IMMEDIATE,CAP_SEVERITY_MODERATE,0},
	{CAP_OP_TYPE_CUMUL_6HR ,"%RTIPS(21600) %PLCOEFF() *",180.0F,179.0F,CAP_URGENCY_IMMEDIATE,CAP_SEVERITY_SEVERE,0},
	{CAP_OP_TYPE_CUMUL_12HR,"%RTIPS(43200) %PLCOEFF() *",360.0F,359.0F,CAP_URGENCY_IMMEDIATE,CAP_SEVERITY_EXTREME,0}
};

static CAP_ALERT_STATUS g_cap_status[CAP_TYPE_END];

static RET_ERROR_CODE _introspective_call_params2(char *sz,uint8_t * sl)
{
	uint8_t i = *sl;
	do {
		const char u = sz[i];
		if ( (u==',') || (u==')') )
		{
			goto p2_quit;
		}
		
		if ((u<'0') || (u>'9'))
		{
			if (u=='.')
			{
				continue;
			}
			
			//ERROR... bad expression
			debug_string_2P(NORMAL,PSTR("Introspective call error") ,PSTR("Bad expression in parsing string"));
			return AC_ERROR_GENERIC;
		}
	} while (++i<EXP_MAX_LEN);

	debug_string_2P(NORMAL,PSTR("Introspective call error") ,PSTR("Bad expression in parsing string-2-"));
	return AC_ERROR_GENERIC;

	
p2_quit:
	*sl = i;
	return AC_ERROR_OK;
}

static RET_ERROR_CODE _introspective_call_params(char *sz,uint8_t * sl,uint8_t plist[EXP_MAX_PARAMS],uint8_t * pNumParams)
{
	uint8_t idx_par = 0;
	uint8_t i = *sl;

	while (++i<EXP_MAX_LEN) {
		char u = sz[i];

		if (u==' ')
		{
			continue;
		}
		
		if ((u>='0') && (u<='9'))
		{
			plist[idx_par++] = i;

			if(AC_ERROR_OK != _introspective_call_params2(sz,&i))
			{
				return AC_ERROR_GENERIC;
			}
			u = sz[i];
			sz[i] = 0;
		}
	
		if (u==')')
		{
			*sl = i;
			*pNumParams = idx_par;
			return AC_ERROR_OK;
		}
	}
	
	return AC_ERROR_GENERIC;
}


static RET_ERROR_CODE _introspective_call(char *sz,float * res)
{
	CAP_INTROSPECTION cin;

	char dbgSZ[128];

	uint8_t p[EXP_MAX_PARAMS];
	uint8_t px;

	uint8_t i=0;
	
	do {
		char u = sz[i];
		if((u=='(')) {
			sz[i] = 0;
			debug_string(NORMAL,sz,RAM_STRING);
			_introspective_call_params(sz,&i,p,&px);
		} 
		
		if (sz[i]==0)
		{
			break;
		}
	} while (++i<EXP_MAX_LEN);

	sprintf_P(dbgSZ,PSTR("\r\nNome funzione %s\r\nNumero di parametri %d\r\n"),sz,px);
	debug_string(NORMAL,dbgSZ,RAM_STRING);
	
	for (uint8_t k=0;k<px;++k)
	{
		float f = atof(sz+p[k]);
		uint32_t vi = (uint32_t) f;
		uint16_t vv = (f-vi) * 100;
		sprintf_P(dbgSZ,PSTR("parametro #%d valore %lu.%d\r\n"),k,vi,vv);
		debug_string(NORMAL,dbgSZ,RAM_STRING);
	}

	if(AC_ERROR_OK != cap_introspection_lookup(sz+1,&cin))
	{
		debug_string_1P(NORMAL,PSTR("Introspection lookup failed"));
		return AC_ERROR_GENERIC;
	}

	if (NULL == cin.fn)
	{
		debug_string_1P(NORMAL,PSTR("Pointer function is NULL"));
		return AC_ERROR_GENERIC;
	}
	
	if (cin.num_params==0) {
		debug_string_1P(NORMAL,PSTR("Zero params"));
		void (*fn)(float *) = cin.fn;
		fn(res);
	} else 	if (cin.num_params==1) {
		debug_string_1P(NORMAL,PSTR("ONE parameter"));
		void (*fn)(float *,float) = cin.fn;

		fn(res,atof(sz+p[0]));
	} else 	if (cin.num_params==2) {
		debug_string_1P(NORMAL,PSTR("TWO parameters"));
		void (*fn)(float *,float,float) = cin.fn;
		fn(res,atof(sz+p[0]),atof(sz+p[1]));
	} else 	if (cin.num_params==3) {
		debug_string_1P(NORMAL,PSTR("THREE parameters"));
		void (*fn)(float *,float,float,float) = cin.fn;
		fn(res,atof(sz+p[0]),atof(sz+p[1]),atof(sz+p[2]));
	} else 	if (cin.num_params==4) {
		debug_string_1P(NORMAL,PSTR("FOUR parameters"));
		void (*fn)(float *,float,float,float,float) = cin.fn;
		fn(res,atof(sz+p[0]),atof(sz+p[1]),atof(sz+p[2]),atof(sz+p[3]));
	}
	
	return AC_ERROR_OK;
}

#include <math.h>

static RET_ERROR_CODE _evaluate_RPN(const char * const sz,float * const res)
{
	debug_string_1P(NORMAL,PSTR("Evaluating string :"));
	debug_string_1P(NORMAL,sz);
	debug_string_1P(NORMAL,g_szCRLF);
	
	float acc = 0.0F;
	float param = 0.0F;
	uint8_t loaded = false;
	
	const uint8_t len = strnlen_P(sz,254);
	for (uint8_t i=0;i<len;++i)
	{
		char t = pgm_read_byte(sz+i);
		if (t==' ')
		{
			continue;
		}

		if(t=='*') {
			
			acc *= param;
			loaded = false;
			continue;
			
		} else if(t=='-') {

			//uint16_t la = acc;
			//uint16_t lp = param;
			//char  buf[32];
			//
			//sprintf_P(buf,PSTR("acc = %u param = %u"),la,lp);
			//debug_string(NORMAL,buf,RAM_STRING);
			//debug_string_P(NORMAL,g_szCRLF);

			
			acc -= param;
			loaded = false;
			continue;
			
		} else if(t=='+') {

			//uint16_t la = acc;
			//uint16_t lp = param;
			//char  buf[32];
			//
			//sprintf_P(buf,PSTR("acc = %u param = %u"),la,lp);
			//debug_string(NORMAL,buf,RAM_STRING);
			//debug_string_P(NORMAL,g_szCRLF);

			acc += param;
			loaded = false;
			continue;

		} else if(t=='/') {

			acc /= param;
			loaded = false;
			continue;

		} else {

			if (loaded) {
				acc = param;
			}
		
			uint8_t intro = (t=='%') ? true : false;
			char  buf[EXP_MAX_LEN];

			for (uint8_t j=0;j<EXP_MAX_LEN;++j,++i)
			{
				char u = pgm_read_byte(sz+i);
				if ((u==' ') || (u==0)) {
					buf[j] = 0;
					break;
				}
					
				buf[j] = u;
			}
					
			buf[31] = 0;
			
			
			if (intro==true)
			{
				if( AC_ERROR_OK != _introspective_call(buf,&param) )
				{
					debug_string_1P(NORMAL,PSTR("ERROR in introspective call, defaulting to 0.0F\r\n"));
					param = 0.0F;
				}
			} else {
				param = atof(buf);
				debug_string(NORMAL,buf,RAM_STRING);
				debug_string_1P(NORMAL,g_szCRLF);
			}
			loaded = true;
		} 
	}
	
	*res = acc;
	char szBuf[96];
	const uint32_t v = (uint32_t) acc * 100;
	sprintf_P(szBuf,PSTR("Expression result * 100 = %lu\r\n"),v);
	debug_string(NORMAL,szBuf,RAM_STRING);

	return AC_ERROR_OK;
}


RET_ERROR_CODE cap_check( CAP_TYPE type , CAP_ALERT_ACTION * const pAction)
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"CAP_CHECK");
	float res = 0.0F;

	_evaluate_RPN(g_alert[type].op_expr,&res);

	if (res >= g_alert[type].trigger_value_in) {
		debug_string_1P(NORMAL,PSTR("CAP CHECK passed alert threshold"));

		if (g_cap_status[type]==CAP_ALERT_STATUS_ISSUE)
		{
			debug_string_1P(NORMAL,PSTR("This Alert was already signaled"));
		} else	if (pAction) {
			pAction->cap_status = CAP_ALERT_STATUS_ISSUE;
			pAction->actual_value = res;
		}
	} else if(res <= g_alert[type].trigger_value_out) {
		debug_string_1P(NORMAL,PSTR("CAP CHECK back to normal"));

		if (g_cap_status[type]==CAP_ALERT_STATUS_OFF)
		{
			debug_string_1P(NORMAL,PSTR("This Alert was already cleared"));
		} else if (pAction) {
			pAction->cap_status = CAP_ALERT_STATUS_OFF;
			pAction->actual_value = res;
		}
	}

	return AC_ERROR_OK;
}

static RET_ERROR_CODE cap_get_msg_id( CAP_ALERT_ACTION * const pAction, char * const szBuf, int szBufLen )
{
	return AC_ERROR_OK;
}

static RET_ERROR_CODE cap_get_AWS_id( CAP_ALERT_ACTION * const pAction, char * const szBuf, int szBufLen )
{
	CFG_ITEM_ADDRESS f;
	cfg_find_item(CFG_TAG_DATALOGGER_AWS_ID,&f);
	cfg_get_item_file(f,szBuf,MAX_AWS_NAME_LENGTH);

	return AC_ERROR_OK;
}

static RET_ERROR_CODE cap_get_msg_date( CAP_ALERT_ACTION * const pAction, char * const szBuf, int szBufLen )
{
	struct calendar_date dt;

	const uint32_t now = hal_rtc_get_time();
	calendar_timestamp_to_date(now,&dt);

	snprintf_P(	szBuf,
				szBufLen,
				PSTR("%04d-%02d-%02dT%02d:%02d:%02d-00:00"),
				dt.year,
				dt.month+1,
				dt.date+1,
				dt.hour,
				dt.minute,
				dt.second );
	
	return AC_ERROR_OK;
}

static RET_ERROR_CODE cap_get_status( CAP_ALERT_ACTION * const pAction, char * const szBuf, int szBufLen )
{
	strncpy_P(szBuf,PSTR("ACTUAL"),szBufLen);
	return AC_ERROR_OK;
}

static RET_ERROR_CODE cap_get_alert( CAP_ALERT_ACTION * const pAction, char * const szBuf, int szBufLen )
{
	char const * psz = PSTR("UNDEFINED");

	switch(pAction->cap_id) {
		case CAP_RAIN_1HR:
			psz = PSTR("1Hr Cumulated RAIN Threshold");
		break;
		case CAP_RAIN_3HR:
			psz = PSTR("3Hr Cumulated RAIN Threshold");
		break;
		case CAP_RAIN_6HR:
			psz = PSTR("6Hr Cumulated RAIN Threshold");
		break;
		case CAP_RAIN_12HR:
			psz = PSTR("12Hr Cumulated RAIN Threshold");
		break;
		case CAP_WATER_LEVEL_LOW:
			psz = PSTR("Water Level Threshold LOW");
		break;
		case CAP_WATER_LEVEL_MED:
			psz = PSTR("Water Level Threshold MED");
		break;
		case CAP_WATER_LEVEL_HIG:
			psz = PSTR("Water Level Threshold HIGH");
		break;
		default:
		break;
		
	}
	
	strncpy_P(szBuf,psz,szBufLen);

	return AC_ERROR_OK;
}

static RET_ERROR_CODE cap_get_scope( CAP_ALERT_ACTION * const pAction, char * const szBuf, int szBufLen )
{
	strncpy_P(szBuf,PSTR("PUBLIC"),szBufLen);
	return AC_ERROR_OK;
}

static RET_ERROR_CODE cap_get_event_desc( CAP_ALERT_ACTION * const pAction, char * const szBuf, int szBufLen )
{
	int l = 0;
	char const * psz = PSTR("UNDEFINED");
	//g_alert[pAction->cap_id].
	switch(pAction->cap_id) {
		case CAP_RAIN_1HR:
		
			if ((CAP_ALERT_STATUS_ON == pAction->cap_status) || (CAP_ALERT_STATUS_ISSUE == pAction->cap_status))
			{
				psz = PSTR("1Hr Cumulated RAIN threshold exceeded");
			} else {
				psz = PSTR("1Hr Cumulated RAIN threshold cleared");
			}
		
		break;
		case CAP_RAIN_3HR:
			if ((CAP_ALERT_STATUS_ON == pAction->cap_status) || (CAP_ALERT_STATUS_ISSUE == pAction->cap_status))
			{
				psz = PSTR("3Hr Cumulated RAIN threshold exceeded");
				} else {
				psz = PSTR("3Hr Cumulated RAIN threshold cleared");
			}
		break;
		case CAP_RAIN_6HR:
			if ((CAP_ALERT_STATUS_ON == pAction->cap_status) || (CAP_ALERT_STATUS_ISSUE == pAction->cap_status))
			{
				psz = PSTR("6Hr Cumulated RAIN threshold exceeded");
				} else {
				psz = PSTR("6Hr Cumulated RAIN threshold cleared");
			}
		break;
		case CAP_RAIN_12HR:
			if ((CAP_ALERT_STATUS_ON == pAction->cap_status) || (CAP_ALERT_STATUS_ISSUE == pAction->cap_status))
			{
				psz = PSTR("12Hr Cumulated RAIN threshold exceeded");
				} else {
				psz = PSTR("12Hr Cumulated RAIN threshold cleared");
			}
		break;
		case CAP_WATER_LEVEL_LOW:
		strncpy_P(szBuf,PSTR("Water Level Threshold LOW"),szBufLen);
			if ((CAP_ALERT_STATUS_ON == pAction->cap_status) || (CAP_ALERT_STATUS_ISSUE == pAction->cap_status))
			{
				psz = PSTR("Water Level LOW Threshold exceeded");
				} else {
				psz = PSTR("Water Level LOW Threshold cleared");
			}
		break;
		case CAP_WATER_LEVEL_MED:
			if ((CAP_ALERT_STATUS_ON == pAction->cap_status) || (CAP_ALERT_STATUS_ISSUE == pAction->cap_status))
			{
				psz = PSTR("Water Level MED Threshold exceeded");
				} else {
				psz = PSTR("Water Level MED Threshold cleared");
			}
		break;
		case CAP_WATER_LEVEL_HIG:
			if ((CAP_ALERT_STATUS_ON == pAction->cap_status) || (CAP_ALERT_STATUS_ISSUE == pAction->cap_status))
			{
				psz = PSTR("Water Level HIGH Threshold exceeded");
				} else {
				psz = PSTR("Water Level HIGH Threshold cleared");
			}
		break;
		default:
		break;
		
	}
	
	strncpy_P(szBuf,psz,szBufLen);
	l = strnlen(szBuf,szBufLen);
	if (l==szBufLen)
	{
		return AC_BUFFER_TOO_SMALL;
	}
	
	
	return AC_ERROR_OK;
}

//RET_ERROR_CODE cap_get_response( CAP_ALERT_ACTION * const pAction, char * const szBuf, int szBufLen )
//{
	//return AC_ERROR_OK;
//}

static RET_ERROR_CODE cap_get_urgency( CAP_ALERT_ACTION * const pAction, char * const szBuf, int szBufLen )
{
	char const * psz = PSTR("UNDEFINED");

	switch(pAction->cap_id) {
		case CAP_RAIN_1HR:
			psz = PSTR("Expected");
		break;
		case CAP_RAIN_3HR:
			psz = PSTR("Immediate");
		break;
		case CAP_RAIN_6HR:
			psz = PSTR("Immediate");
		break;
		case CAP_RAIN_12HR:
			psz = PSTR("Immediate");
		break;
		case CAP_WATER_LEVEL_LOW:
			psz = PSTR("Expected");
		break;
		case CAP_WATER_LEVEL_MED:
			psz = PSTR("Immediate");
		break;
		case CAP_WATER_LEVEL_HIG:
			psz = PSTR("Immediate");
		break;
		default:
		break;
		
	}

	strncpy_P(szBuf,psz,szBufLen);

	return AC_ERROR_OK;
}

static RET_ERROR_CODE cap_get_severity( CAP_ALERT_ACTION * const pAction, char * const szBuf, int szBufLen )
{
	char const * psz = PSTR("UNDEFINED");

	switch(pAction->cap_id) {
		case CAP_RAIN_1HR:
			psz = PSTR("Moderate");
		break;
		case CAP_RAIN_3HR:
			psz = PSTR("Moderate");
		break;
		case CAP_RAIN_6HR:
			psz = PSTR("Severe");
		break;
		case CAP_RAIN_12HR:
			psz = PSTR("Extreme");
		break;
		case CAP_WATER_LEVEL_LOW:
			psz = PSTR("Moderate");
		break;
		case CAP_WATER_LEVEL_MED:
			psz = PSTR("Severe");
		break;
		case CAP_WATER_LEVEL_HIG:
			psz = PSTR("Extreme");
		break;
		default:
		break;
		
	}
	
	strncpy_P(szBuf,psz,szBufLen);

	return AC_ERROR_OK;
}

static RET_ERROR_CODE cap_get_certainty( CAP_ALERT_ACTION * const pAction, char * const szBuf, int szBufLen )
{
	
	strncpy_P(szBuf,PSTR("Observed"),szBufLen);

	return AC_ERROR_OK;
}

static RET_ERROR_CODE cap_get_sender_name( CAP_ALERT_ACTION * const pAction, char * const szBuf, int szBufLen )
{
	CFG_ITEM_ADDRESS f;
	cfg_find_item(CFG_TAG_CAP_SENDER_NAME,&f);
	cfg_get_item_file(f,szBuf,min(32,szBufLen));

	return AC_ERROR_OK;
}

static RET_ERROR_CODE cap_get_area_desc( CAP_ALERT_ACTION * const pAction, char * const szBuf, int szBufLen )
{
	CFG_ITEM_ADDRESS f;
	cfg_find_item(CFG_TAG_CAP_AREA_DESC,&f);
	cfg_get_item_file(f,szBuf,min(32,szBufLen));

	return AC_ERROR_OK;
}

static RET_ERROR_CODE cap_get_polygon( CAP_ALERT_ACTION * const pAction, char * const szBuf, int szBufLen )
{
	CFG_ITEM_ADDRESS f;
	cfg_find_item(CFG_TAG_CAP_AREA_DESC,&f);
	cfg_get_item_file(f,szBuf,min(32,szBufLen));

	return AC_ERROR_OK;
}

static RET_ERROR_CODE cap_compose_document( CAP_ALERT_ACTION * const pAction , char * pDest, uint16_t * pDestLen)
{
	int sLen = 0;
	int dstLen = *pDestLen;
	
	const static int BUFLEN = 512;
	char szBuf[BUFLEN];
	
	char * const psz = pDest;
	
	sLen  = snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("<?xml version = \"1.0\" encoding = \"UTF-8\"?>\r\n"));
	sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("<alert xmlns = \"urn:oasis:names:tc:emergency:cap:1.2\">\r\n"));

	cap_get_msg_id(pAction,szBuf,BUFLEN);
	sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("\t<identifier>%s</identifier>\r\n"),szBuf);

	cap_get_AWS_id(pAction,szBuf,BUFLEN);
	sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("\t<sender>%s</sender>\r\n"),szBuf);

	cap_get_msg_date(pAction,szBuf,BUFLEN);
	sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("\t<sent>%s</sent>\r\n"),szBuf);

	cap_get_status(pAction,szBuf,BUFLEN);
	sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("\t<status>%s</status>\r\n"),szBuf);

    cap_get_alert(pAction,szBuf,BUFLEN);
	sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("\t<msgType>Alert</msgType>\r\n"));

	cap_get_scope(pAction,szBuf,BUFLEN);
	sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("\t<scope>%s</scope>\r\n"),szBuf);

	sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("\t<info>\r\n"));
	sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("\t\t<category>Met</category>\r\n"));

	cap_get_event_desc(pAction,szBuf,BUFLEN);
	sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("\t\t<event>%s</event>\r\n"),szBuf);

	//cap_get_response(pAction,szBuf,BUFLEN);
	//sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("\t\t<responseType>%s</responseType>\r\n"),szBuf);

	cap_get_urgency(pAction,szBuf,BUFLEN);
	sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("\t\t<urgency>%s</urgency>\r\n"),szBuf);

	cap_get_severity(pAction,szBuf,BUFLEN);
	sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("\t\t<severity>%s</severity>\r\n"),szBuf);

	cap_get_certainty(pAction,szBuf,BUFLEN);
	sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("\t\t<certainty>Observed</certainty>\r\n"));

	cap_get_sender_name(pAction,szBuf,BUFLEN);
	sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("\t\t<senderName>%s</senderName>\r\n"),szBuf);

	sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("\t\t<area>\r\n"));

	cap_get_area_desc(pAction,szBuf,BUFLEN);
	sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("\t\t\t<areaDesc>%s</areaDesc>\r\n"),szBuf);

	cap_get_polygon(pAction,szBuf,BUFLEN);
	sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("\t\t\t<polygon>%s</polygon>\r\n"),szBuf);

	sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("\t\t</area>\r\n"));
	sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("\t</info>\r\n"));
	sLen += snprintf_P(psz+sLen,max(0,dstLen-sLen),PSTR("</alert>\r\n"));

	*pDestLen = sLen;

	return AC_ERROR_OK;
}

static RET_ERROR_CODE cap_say( CAP_ALERT_ACTION * const pAction )
{
	//RET_ERROR_CODE e = sim900_GPRS_check_line();
	//if(AC_ERROR_OK!=e) {
		//return e;
	//}
	//
	//e = sim900_bearer_simple_open();
	//if(AC_ERROR_OK!=e) {
		//return e;
	//}
//
	static const uint16_t BUFFER_SIZE = 3584;
	char szBuf[BUFFER_SIZE];
	
	//CFG_ITEM_ADDRESS f;
//
	//e = cfg_find_item(CFG_TAG_CAP_SERVER_URL,&f);
	//
	//if(e!=AC_ERROR_OK) {
		//debug_string_1P(NORMAL,PSTR("Unable to open configuration section CAP_SERVER_URL"));
		//return e;
	//}
	//
	//cfg_get_item_file(f,szBuf,128);
//
	//const uint16_t lz = strnlen(szBuf,128)-1;
	//
	//szBuf[lz] = 0;
	//char * const pDoc = szBuf+lz+1;
	//int docBufLen = BUFFER_SIZE-(lz+1);

	char * const pDoc = szBuf;
	int docBufLen = BUFFER_SIZE;

	cap_compose_document(pAction,pDoc,&docBufLen);
	debug_string(NORMAL,szBuf,RAM_STRING);
	debug_string_1P(NORMAL,g_szCRLF);
	debug_string(NORMAL,pDoc,RAM_STRING);
	debug_string_1P(NORMAL,g_szCRLF);

	cap_consume(pDoc,docBufLen);

	//sim900_http_post(szBuf,RAM_STRING,pDoc,strnlen(pDoc,docBufLen),RAM_STRING);
	//sim900_http_close();
//
	//sim900_bearer_simple_close();

	return AC_ERROR_OK;

}

RET_ERROR_CODE cap_clear( CAP_ALERT_ACTION * const pAction )
{
	RET_ERROR_CODE e = cap_say(pAction);

	if(e!=AC_ERROR_OK) {
		debug_string_1P(NORMAL,PSTR("Cap clear aborting"));
		return e;
	}
	
	g_cap_status[pAction->cap_id] = CAP_ALERT_STATUS_OFF;
	
	return AC_ERROR_OK;
}


RET_ERROR_CODE cap_issue( CAP_ALERT_ACTION * const pAction )
{
	RET_ERROR_CODE e = cap_say(pAction);

	if(e!=AC_ERROR_OK) {
		debug_string_1P(NORMAL,PSTR("Cap Issue aborting"));
		return e;
	}
	
	g_cap_status[pAction->cap_id] = CAP_ALERT_STATUS_ON;
	
	return AC_ERROR_OK;
}

void cap_test(void)
{
	float res = 0.0F;
	char buf[128];
	
	_evaluate_RPN(PSTR("3 2 - 4 + %LEV() - 2 * %RTIPS(3600) + %PLCOEFF() *"),&res);
	uint16_t d = (uint16_t) res;
	sprintf_P(buf,PSTR("RPN 3 2 - 4 + %%LEV() - 2 * %%RTIPS(3600) + %%PLCOEFF() * = %u\r\n"),d);
	debug_string(NORMAL,buf,RAM_STRING);
	
	//cap_issue(CAP_RAIN_1HR);
}
