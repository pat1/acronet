
#ifndef HAL_PSW_IMPL_H_
#define HAL_PSW_IMPL_H_

#include "Acronet/drivers/PCAL9535A/PCAL9535A.h"
#include "Acronet/drivers/PCAL9554B/PCAL9554B.h"

void hal_psw_set(uint8_t val)
{
	PCAL9535A_Write(0x49,(val & 0x0F));
}

void hal_psw_get_value(uint8_t * const pval)
{
	uint8_t v = 0;
	PCAL9535A_Read(0x01,&v);
	*pval = (v & 0x0F);
}

void hal_psw_get_status(uint8_t * const pStat)
{
	uint8_t v = 0;
	PCAL9535A_Read(0x01,&v);
	*pStat = (v & 0xF0);
}

static RET_ERROR_CODE PCAL9554B_internal_Init( void )
{

	//	const char * const funName = PSTR("PCAL9554");

	/////////////////////////////////////////////////////////////
	/////////
	/////////  PORT INPUT/OUTPUT PIN CONFIG
	/////////  IF A	BIT IN THESE REGISTERS IS SET TO 1
	/////////  THE CORRESPONDING PORT PIN IS ENABLED AS A
	/////////  HIGH-IMPEDANCE INPUT.
	/////////  IF A BIT IN THESE REGISTERS IS CLEARED TO 0,
	/////////  THE CORRESPONDING PORT PIN IS ENABLED AS AN OUTPUT
	/////////
	/////////  On board V03 PORT0 PINS are configured as follow
	/////////  P_0 : OUTPUT -> SP336_2_M1
	/////////  P_1 : OUTPUT -> SP336_2_M2
	/////////  P_2 : OUTPUT -> SP336_2_M0
	/////////  P_3 : OUTPUT -> SP336_2_SLEW_RATE_LIMIT
	/////////  P_4 : OUTPUT -> SP336_SLEW_RATE_LIMIT
	/////////  P_5 : OUTPUT -> SP336_M0
	/////////  P_6 : OUTPUT -> SP336_M2
	/////////  P_7 : OUTPUT -> SP336_M1
	/////////
	/////////  NUMBERS ARE BIG_ENDIAN
	/////////
	
	RET_ERROR_CODE r = PCAL9554B_Write(0x3,0b11111111); //Using only PULLUP/PULLDOWN, leaving in HighZ
	delay_ms(5);
	if(AC_ERROR_OK != r )
	{
		return r;
	}

	/////////////////////////////////////////////////////////////
	/////////
	///////// PORT0 PULLUP/PULLDOWN ENABLE PIN CONFIG
	/////////
	///////// SETTING THE BIT TO LOGIC 1 ENABLES THE SELECTION OF PULL-UP/PULL-DOWN RESISTORS.
	///////// SETTING THE BIT TO LOGIC 0 DISCONNECTS THE PULL-UP/PULL-DOWN RESISTORS FROM THE I/O PINS.
	///////// THE RESISTORS WILL BE DISCONNECTED WHEN THE OUTPUTS ARE CONFIGURED AS OPEN-DRAIN OUTPUTS
	///////// USE THE PULL-UP/PULL-DOWN REGISTERS TO SELECT EITHER A PULL-UP OR PULL-DOWN
	///////// RESISTOR
	/////////
	r = PCAL9554B_Write(0x43,0b11111111);
	delay_ms(5);
	if(AC_ERROR_OK != r )
	{
		return r;
	}

	/////////////////////////////////////////////////////////////
	/////////
	///////// PORT0 CHOOSE PULLUP OR PULLDOWN  PIN CONFIG
	/////////
	/////////  P_0 to P_2 : M1,M2,M0 SP336_2 MODE SELECTION
	/////////       000 -> LOOPBACK
	/////////       100 -> 2 RS485 HALFDUPLEX
	/////////       010 -> 2 RS232
	/////////       001 -> MIXED HALFDUPLEX - RS232 ON PORTA - RS485 ON PORTB
	/////////       101 -> LOWPOWER RS232 ONLY RX
	/////////       011 -> MIXED FULLDUPLEX - RS232 ON PORTA - RS485 ON PORTB
	/////////       110 -> 2 RS485 FULLDUPLEX
	/////////       111 -> SHUTDOWN
	/////////  P_3 : SP336_2 SLEWRATE LIMIT SELECTION
	/////////  P_4 : SP336 SLEWRATE LIMIT SELECTION
	/////////  P_5 to P_7 : M0,M2,M1 SP336 MODE SELECTION
	/////////       000 -> LOOPBACK
	/////////       001 -> 2 RS485 HALFDUPLEX
	/////////       010 -> 2 RS232
	/////////       011 -> 2 RS485 FULLDUPLEX
	/////////       100 -> MIXED HALFDUPLEX - RS232 ON PORTA - RS485 ON PORTB
	/////////       101 -> LOWPOWER RS232 ONLY RX
	/////////       110 -> MIXED FULLDUPLEX - RS232 ON PORTA - RS485 ON PORTB
	/////////       111 -> SHUTDOWN



	r = PCAL9554B_Write(0x44,SP336_SELECTED_SETUP);
	delay_ms(5);
	if(AC_ERROR_OK != r )
	{
		return r;
	}

	///////////////////////////////////////////////////////////////
	/////////// PORT0 OUTPUT WRITE
	//r = PCAL9554B_Write(0x2,0b01110111);
	//delay_ms(5);
	//if(AC_ERROR_OK != r )
	//{
		//return r;
	//}


	return AC_ERROR_OK;
}


static RET_ERROR_CODE PCAL9535A_internal_Init( void )
{

	//	const char * const funName = PSTR("PCAL9535");

	/////////////////////////////////////////////////////////////
	/////////
	/////////  PORT0 INPUT/OUTPUT PIN CONFIG
	/////////  IF A	BIT IN THESE REGISTERS IS SET TO 1
	/////////  THE CORRESPONDING PORT PIN IS ENABLED AS A
	/////////  HIGH-IMPEDANCE INPUT.
	/////////  IF A BIT IN THESE REGISTERS IS CLEARED TO 0,
	/////////  THE CORRESPONDING PORT PIN IS ENABLED AS AN OUTPUT
	/////////
	/////////  On board V04 PORT0 PINS are configured as follow
	/////////  P0_0 : INPUT  -> NVALID3 (WALL POWER INPUT VOLTAGE VALIDITY)
	/////////  P0_1 : INPUT  -> NVALID2 (PRIMARY POWER INPUT VOLTAGE VALIDITY)
	/////////  P0_2 : INPUT  -> NVALID1 (BACKUP POWER INPUT VOLTAGE VALIDITY)
	/////////  P0_3 : OUTPUT -> OC1 (Open Collector output)
	/////////  P0_4 : OUTPUT -> OC2 (Open Collector output)
	/////////  P0_5 : OUTPUT -> OC3 (Open Collector output)
	/////////  P0_6 : OUTPUT -> OC4 (Open Collector output)
	/////////  P0_7 : OUTPUT -> BOARD POWER SWITCH
	/////////
	/////////
	RET_ERROR_CODE r = PCAL9535A_Write(0x6,0b11111111); //Using only PULLUP/PULLDOWN, leaving in HighZ
	delay_ms(5);
	if(AC_ERROR_OK != r )
	{
		return r;
	}

	/////////////////////////////////////////////////////////////
	/////////
	///////// PORT0 PULLUP/PULLDOWN ENABLE PIN CONFIG
	/////////
	///////// SETTING THE BIT TO LOGIC 1 ENABLES THE SELECTION OF PULL-UP/PULL-DOWN RESISTORS.
	///////// SETTING THE BIT TO LOGIC 0 DISCONNECTS THE PULL-UP/PULL-DOWN RESISTORS FROM THE I/O PINS.
	///////// THE RESISTORS WILL BE DISCONNECTED WHEN THE OUTPUTS ARE CONFIGURED AS OPEN-DRAIN OUTPUTS
	///////// USE THE PULL-UP/PULL-DOWN REGISTERS TO SELECT EITHER A PULL-UP OR PULL-DOWN
	///////// RESISTOR
	/////////
	r = PCAL9535A_Write(0x46,0b11111111);
	delay_ms(5);
	if(AC_ERROR_OK != r )
	{
		return r;
	}


	/////////////////////////////////////////////////////////////
	/////////
	///////// PORT0 CHOOSE PULLUP OR PULLDOWN  PIN CONFIG
	/////////
	/////////  P0_0 : PULL-UP -> 1 : NVALID3 (WALL POWER INPUT VOLTAGE VALIDITY)
	/////////  P0_1 : PULL-UP -> 1 : NVALID2 (PRIMARY POWER INPUT VOLTAGE VALIDITY)
	/////////  P0_2 : PULL-UP -> 1 : NVALID1 (BACKUP POWER INPUT VOLTAGE VALIDITY)
	/////////  P0_3 : PULL-DW -> 0 : OC1 (Open Collector output)
	/////////  P0_4 : PULL-DW -> 0 : OC2 (Open Collector output)
	/////////  P0_5 : PULL-DW -> 0 : OC3 (Open Collector output)
	/////////  P0_6 : PULL-DW -> 0 : OC4 (Open Collector output)
	/////////  P0_7 : PULL-DW -> 0 : BOARD POWER SWITCH
	/////////
	/////////
	r = PCAL9535A_Write(0x48,0b00000111);
	delay_ms(5);
	if(AC_ERROR_OK != r )
	{
		return r;
	}

	///////////////////////////////////////////////////////////////
	/////////// PORT0 OUTPUT WRITE
	//r = PCAL9535A_Write(0x2,0b00000000);
	//delay_ms(5);
	//if(AC_ERROR_OK != r )
	//{
		//return r;
	//}

	/////////////////////////////////////////////////////////////
	/////////
	/////////  PORT1 INPUT/OUTPUT PIN CONFIG
	/////////  IF A	BIT IN THESE REGISTERS IS SET TO 1
	/////////  THE CORRESPONDING PORT PIN IS ENABLED AS A
	/////////  HIGH-IMPEDANCE INPUT.
	/////////  IF A BIT IN THESE REGISTERS IS CLEARED TO 0,
	/////////  THE CORRESPONDING PORT PIN IS ENABLED AS AN OUTPUT
	/////////
	/////////  On board V04 PORT0 PINS are configured as follow
	/////////  P1_7 : INPUT  -> PSW1 STATUS
	/////////  P1_6 : INPUT  -> PSW2 STATUS
	/////////  P1_5 : INPUT  -> PSW3 STATUS
	/////////  P1_4 : INPUT  -> PSW4 STATUS
	/////////  P1_3 : OUTPUT -> PSW1 COMMAND
	/////////  P1_2 : OUTPUT -> PSW2 COMMAND
	/////////  P1_1 : OUTPUT -> PSW3 COMMAND
	/////////  P1_0 : OUTPUT -> PSW4 COMMAND
	/////////
	/////////
	r = PCAL9535A_Write(0x7,0b11111111);//Using only PULLUP/PULLDOWN, leaving in HighZ
//	r = PCAL9535A_Write(0x7,0b11110000);
	delay_ms(5);
	if(AC_ERROR_OK != r )
	{
		return r;
	}
	
	/////////////////////////////////////////////////////////////
	/////////
	///////// PORT1 PULLUP/PULLDOWN ENABLE PIN CONFIG
	/////////
	///////// SETTING THE BIT TO LOGIC 1 ENABLES THE SELECTION OF PULL-UP/PULL-DOWN RESISTORS.
	///////// SETTING THE BIT TO LOGIC 0 DISCONNECTS THE PULL-UP/PULL-DOWN RESISTORS FROM THE I/O PINS.
	///////// THE RESISTORS WILL BE DISCONNECTED WHEN THE OUTPUTS ARE CONFIGURED AS OPEN-DRAIN OUTPUTS
	///////// USE THE PULL-UP/PULL-DOWN REGISTERS TO SELECT EITHER A PULL-UP OR PULL-DOWN
	///////// RESISTOR
	/////////
	r = PCAL9535A_Write(0x47,0b11111111);
	delay_ms(5);
	if(AC_ERROR_OK != r )
	{
		return r;
	}

	/////////////////////////////////////////////////////////////
	/////////
	///////// PORT1 CHOOSE PULLUP OR PULLDOWN  PIN CONFIG
	/////////
	/////////  P0_7 : INPUT WITH PULL-UP    -> 1
	/////////  P0_6 : INPUT WITH PULL-UP    -> 1
	/////////  P0_5 : INPUT WITH PULL-UP    -> 1
	/////////  P0_4 : INPUT WITH PULL-UP    -> 1
	/////////
	
	//r = PCAL9535A_Write(0x49,0b11111111);
	r = PCAL9535A_Write(0x49,0b11110000);
	delay_ms(5);
	if(AC_ERROR_OK != r )
	{
		return r;
	}


	/////////////////////////////////////////////////////////////
	/////////// PORT1 WRITE
	//r = PCAL9535A_Write(0x3,0b00001111);
	//delay_ms(5);
	//if(AC_ERROR_OK != r )
	//{
		//return r;
	//}


	return AC_ERROR_OK;
}

void hal_PCALXXXX_Init( void )
{
	DEBUG_PRINT_FUNCTION_NAME(NORMAL,"PCALXXXX_INIT");
	
	twi_master_options_t opt = {
		.speed = PCAL9535A_TWI_SPEED,
		.chip  = TWI_MASTER_ADDRESS,
	};
	

	const int e = twi_master_setup(PCAL9535A_TWI_PORT, &opt);

	if (STATUS_OK==e)
	{
		PCAL9535A_internal_Init();
		PCAL9554B_internal_Init();
	}

	//	return AC_UNSUPPORTED;

}


#endif /* HAL_PSW_IMPL_H_ */