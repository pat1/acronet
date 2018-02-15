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



#ifndef GLOBALS_H_
#define GLOBALS_H_


enum {
	TASK_STOP=0,
	TASK_READY,
	TASK_RUNNING
	};

//#define SIM900_USART_POLLED

#include <asf.h> //TODO: remove when you will move simple_signal away from this file
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <compiler.h>
//#include <util/delay.h>
//
//#define delay_ms _delay_ms
//#define delay_us _delay_us

#include "string.h"
#include "progmem.h"



//Strings
extern const char g_szCRLF[];


//struct Sensor_API
//{
//
    //int dummy;
//
//};


///////////////////////////////////////////////////////////////////////////////////////
//
#define MAX_AWS_NAME_LENGTH 64



///////////////////////////////////////////////////////////////////////////////////////


//ERROR CODES
typedef uint16_t RET_ERROR_CODE;

#define AC_ERROR_OK	((RET_ERROR_CODE)(0))
#define AC_ERROR_GENERIC ((RET_ERROR_CODE)(1))

#define AC_NOTHING_TO_DO ((RET_ERROR_CODE)(90))

#define AC_UNSUPPORTED ((RET_ERROR_CODE)(100))

#define AC_BAD_PARAMETER ((RET_ERROR_CODE)(101))
#define AC_BUFFER_OVERFLOW ((RET_ERROR_CODE)(102))
#define AC_BUFFER_TOO_SMALL ((RET_ERROR_CODE)(103))
#define AC_TRUNCATED_READ ((RET_ERROR_CODE)(104))

#define AC_ITEM_NOT_FOUND ((RET_ERROR_CODE)(200))

#define AC_SIM900_ERROR_BEGIN_PLACEHOLDER	((RET_ERROR_CODE)(1000))
#define AC_SIM900_NOT_READY		AC_SIM900_ERROR_BEGIN_PLACEHOLDER
#define AC_SIM900_TIMEOUT ((RET_ERROR_CODE)(1001))
#define AC_SIM900_RESOURCE_UNAVAILABLE ((RET_ERROR_CODE)(1002))

#define AC_SIM900_COMM_ERROR ((RET_ERROR_CODE)(1003))
#define AC_SIM900_SIM_NOT_PRESENT ((RET_ERROR_CODE)(1004))
#define AC_SIM900_SIM_LOCKED ((RET_ERROR_CODE)(1005))
#define AC_SIM900_SIM_PIN_WRONG ((RET_ERROR_CODE)(1006))
#define AC_SIM900_SIM_PUK_LOCK ((RET_ERROR_CODE)(1007))
#define AC_SIM900_LINE_NOT_REGISTERED ((RET_ERROR_CODE)(1008))

#define AC_SIM900_GPRS_NOT_ATTACHED ((RET_ERROR_CODE)(1010))
#define AC_SIM900_ERROR_END_PLACEHOLDER	((RET_ERROR_CODE)(1011))

#define AC_AT24C_NOT_READY ((RET_ERROR_CODE)(1100))
#define AC_AT24C_IO_ERROR ((RET_ERROR_CODE)(1101))
#define AC_AT24C_FLUSHED ((RET_ERROR_CODE)(1102))
#define AC_AT24C_TIMEOUT ((RET_ERROR_CODE)(1103))
#define AC_AT24C_BAD_DATA ((RET_ERROR_CODE)(1104))
#define AC_AT24C_PROTOCOL ((RET_ERROR_CODE)(1105))
#define AC_AT24C_UNSOPPORTED_DEV ((RET_ERROR_CODE)(1106))
#define AC_AT24C_NO_MEMORY ((RET_ERROR_CODE)(1107))
#define AC_AT24C_INVALID_ARG ((RET_ERROR_CODE)(1108))
#define AC_AT24C_BAD_ADDRESS ((RET_ERROR_CODE)(1109))
#define AC_AT24C_BUSY ((RET_ERROR_CODE)(1110))
#define AC_AT24C_BAD_FORMAT ((RET_ERROR_CODE)(1111))


#define AC_TASK_NOT_READY_FOR_EXECUTION ((RET_ERROR_CODE)(5000))
#define AC_TASK_STILL_PENDING ((RET_ERROR_CODE)(5001))



//DEBUG facilities

extern uint8_t g_log_verbosity;
#define RAM_STRING false
#define PGM_STRING true

enum {QUIET=0,NON_MASKERABLE=0,NORMAL,VERBOSE,VERY_VERBOSE};
void debug_string(const uint8_t level,const char * const sz,const uint8_t isPGM);
//void debug_string_P(const uint8_t level,const char * const sz);
void debug_string_2P(const uint8_t level,const char * const szWho,const char * const szWhat);
void debug_string_1P(const uint8_t level,const char * const szWhat);


#define DEBUG_FUNCTION_NAME_PRINT_1(x) 		const char * const func_name __attribute__((__cleanup__(debug_function_out_name_print))) = __func__; \
											debug_function_in_name_print(x,func_name);
#define DEBUG_FUNCTION_NAME_PRINT_2(x,y)	const char * const func_name __attribute__((__cleanup__(debug_function_out_name_print_P))) = PSTR( y );\
											debug_function_in_name_print_P(x,func_name);


#define GET_3RD_ARG(a1,a2,a3,...) a3
#define DEBUG_FUNCTION_CHOOSER(...) GET_3RD_ARG(__VA_ARGS__,DEBUG_FUNCTION_NAME_PRINT_2,DEBUG_FUNCTION_NAME_PRINT_1)
#define DEBUG_PRINT_FUNCTION_NAME(...) 	DEBUG_FUNCTION_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

void debug_function_in_name_print(const uint8_t level,const char fname[]);
void debug_function_out_name_print(const char * const * const fname);

void debug_function_in_name_print_P(const uint8_t level,const char fname[]);
void debug_function_out_name_print_P(const char * const * const fname);


#define SIGNAL_SET_AND_CLEAR_AUTOMATIC(s)	volatile uint8_t * const sig_##s __attribute__((__cleanup__(simple_signal_clear2))) = &s; \
											simple_signal_set(&s);
											
#define SIGNAL_CLEAR_AUTOMATIC(s)			volatile uint8_t * const sig_##s __attribute__((__cleanup__(simple_signal_clear2))) = &s;


static inline void simple_signal_clear(volatile uint8_t * const pSignal)
{
	*pSignal = 0;
}
static inline void simple_signal_set  (volatile uint8_t * const pSignal)
{
	*pSignal = 0xFF;
}
//RET_ERROR_CODE simple_signal_wait(volatile uint8_t * const pSignal,uint16_t mills);
static inline void simple_signal_wait(volatile uint8_t * const pSignal)
{
	while(*pSignal) { barrier(); }
}



static inline void simple_signal_clear2(volatile uint8_t * const * pSignal)
{
	simple_signal_clear(*pSignal);
}


static inline void PORT_ConfigureInterrupt0( PORT_t * port, PORT_INT0LVL_t intLevel,  uint8_t pinMask )
{
    port->INTCTRL = ( port->INTCTRL & ~PORT_INT0LVL_gm ) | intLevel;
    port->INT0MASK = pinMask;
    /* Enable the level interrupts in the PMIC. */
    PMIC.CTRL |= intLevel;
}


static inline void PORT_ConfigureInterrupt1( PORT_t * port,PORT_INT1LVL_t intLevel,uint8_t pinMask )
{
	port->INTCTRL = ( port->INTCTRL & ~PORT_INT1LVL_gm ) | intLevel;
	port->INT1MASK = pinMask;
	/* Enable the level interrupts in the PMIC. */
	PMIC.CTRL |= intLevel;
}



//void dump_rainstats_to_log(const uint8_t id);


#endif /* GLOBALS_H_ */