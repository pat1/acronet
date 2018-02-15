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


#ifndef CAP_COMMON_H_
#define CAP_COMMON_H_


typedef enum {	CAP_TYPE_BEG = 0
				,CAP_RAIN_1HR = CAP_TYPE_BEG
				,CAP_RAIN_3HR
				,CAP_RAIN_6HR
				,CAP_RAIN_12HR
				,CAP_WATER_LEVEL_LOW
				,CAP_WATER_LEVEL_MED
				,CAP_WATER_LEVEL_HIG
				,CAP_TYPE_END
} CAP_TYPE;


typedef enum { CAP_OP_TYPE_WATERLEV,
				CAP_OP_TYPE_CUMUL_1HR,
				CAP_OP_TYPE_CUMUL_3HR,
				CAP_OP_TYPE_CUMUL_6HR,
				CAP_OP_TYPE_CUMUL_12HR 
} CAP_OPERATION;


typedef enum {
				CAP_ALERT_STATUS_ON,
				CAP_ALERT_STATUS_ISSUE,
				CAP_ALERT_STATUS_CLEAR,
				CAP_ALERT_STATUS_OFF
				
} CAP_ALERT_STATUS;

typedef struct  {
	CAP_TYPE			cap_id;
	CAP_ALERT_STATUS	cap_status;
	float				trigger_value;
	float				actual_value;
} CAP_ALERT_ACTION;

enum { CAP_URGENCY_EXPECTED, CAP_URGENCY_IMMEDIATE};
enum { CAP_SEVERITY_MODERATE,CAP_SEVERITY_SEVERE,CAP_SEVERITY_EXTREME};

typedef struct {
	CAP_OPERATION op_type;
	char    op_expr[32];
	float   trigger_value_in;
	float   trigger_value_out;
	uint8_t urgency;
	uint8_t severity;
	uint8_t response_type;
	flash_addr_t pTemplate;
} CAP_ALERT;

typedef RET_ERROR_CODE (*CAP_INTROSPECTION_VOID)(float *);
typedef RET_ERROR_CODE (*CAP_INTROSPECTION_1PAR)(float,float *);

typedef struct {
	char op_token[8];
	uint8_t num_params;
	void * fn;
} CAP_INTROSPECTION;

extern const __flash CAP_INTROSPECTION g_cap_introspection[];
RET_ERROR_CODE cap_introspection_lookup(char * pKey,CAP_INTROSPECTION * pVal);


extern const __flash CAP_ALERT g_alert[];
	
//extern CAP_ALERT_STATUS g_cap_status[CAP_TYPE_END];


RET_ERROR_CODE cap_check( CAP_TYPE type , CAP_ALERT_ACTION * const pAction);
RET_ERROR_CODE cap_issue( CAP_ALERT_ACTION * const pAction );
RET_ERROR_CODE cap_clear( CAP_ALERT_ACTION * const pAction );

void cap_test(void);

#endif /* CAP_COMMON_H_ */