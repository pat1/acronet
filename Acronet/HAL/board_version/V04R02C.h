
#ifndef V04R02C_H_
#define V04R02C_H_

#define CMD_I2C_PORT	(&TWIE)
#define CMD_I2C_SPEED	400000

#define EEPROM_CHIP_ADDR       0x50				//!< TWI slave memory address
#define EEPROM_TWI_SPEED       CMD_I2C_SPEED	//!< TWI data transfer rate
#define EEPROM_TWI_PORT        CMD_I2C_PORT
#define TWI_MASTER_ADDRESS     0x99

#define AUX_TWI_PORT        (&TWIC)

#define PCAL9535A_CHIP_ADDR       0x22					//!< TWI slave memory address
#define PCAL9535A_TWI_SPEED       CMD_I2C_SPEED			//!< TWI data transfer rate
#define PCAL9535A_TWI_PORT        CMD_I2C_PORT

#define PCAL9554B_CHIP_ADDR			0x20
#define PCAL9554B_TWI_SPEED       CMD_I2C_SPEED			//!< TWI data transfer rate
#define PCAL9554B_TWI_PORT        CMD_I2C_PORT

#define MPL3115A2_CHIP_ADDR			0x60
#define MPL3115A2_TWI_SPEED       CMD_I2C_SPEED			//!< TWI data transfer rate
#define MPL3115A2_TWI_PORT        CMD_I2C_PORT

//////////////////////////////////////////////////////////////////////////////////////
//
//
// SP336
//
//
//////////////////////////////////////////////////////////////////////////////////////

#define SP336_USART0 USARTC0
#define SP336_USART1 USARTC1
#define SP336_USART2 USARTD0
#define SP336_USART3 USARTD1

#define  SP336_USART0_RX_Vect		USARTC0_RXC_vect
#define  SP336_USART0_TX_Vect		USARTC0_TXC_vect
#define  SP336_USART0_DRE_Vect		USARTC0_DRE_vect
#define  SP336_USART0_SYSCLK		SYSCLK_USART0

#define  SP336_USART1_RX_Vect		USARTC1_RXC_vect
#define  SP336_USART1_TX_Vect		USARTC1_TXC_vect
#define  SP336_USART1_DRE_Vect		USARTC1_DRE_vect
#define  SP336_USART1_SYSCLK		SYSCLK_USART1

#define  SP336_USART2_RX_Vect		USARTD0_RXC_vect
#define  SP336_USART2_TX_Vect		USARTD0_TXC_vect
#define  SP336_USART2_DRE_Vect		USARTD0_DRE_vect
#define  SP336_USART2_SYSCLK		SYSCLK_USART0

#define  SP336_USART3_RX_Vect		USARTD1_RXC_vect
#define  SP336_USART3_TX_Vect		USARTD1_TXC_vect
#define  SP336_USART3_DRE_Vect		USARTD1_DRE_vect
#define  SP336_USART3_SYSCLK		SYSCLK_USART1

#define SP336_USART0_PIN_TX_ENABLE IOPORT_CREATE_PIN(PORTC,4)
#define SP336_USART0_PIN_TX_SIGNAL IOPORT_CREATE_PIN(PORTC,3)
#define SP336_USART0_PIN_RX_SIGNAL IOPORT_CREATE_PIN(PORTC,2)

#define SP336_USART1_PIN_TX_ENABLE IOPORT_CREATE_PIN(PORTC,5)
#define SP336_USART1_PIN_TX_SIGNAL IOPORT_CREATE_PIN(PORTC,7)
#define SP336_USART1_PIN_RX_SIGNAL IOPORT_CREATE_PIN(PORTC,6)

#define SP336_USART2_PIN_TX_ENABLE IOPORT_CREATE_PIN(PORTD,4)
#define SP336_USART2_PIN_TX_SIGNAL IOPORT_CREATE_PIN(PORTD,3)
#define SP336_USART2_PIN_RX_SIGNAL IOPORT_CREATE_PIN(PORTD,2)

#define SP336_USART3_PIN_TX_ENABLE IOPORT_CREATE_PIN(PORTD,5)
#define SP336_USART3_PIN_TX_SIGNAL IOPORT_CREATE_PIN(PORTD,7)
#define SP336_USART3_PIN_RX_SIGNAL IOPORT_CREATE_PIN(PORTD,6)

#define	SP336_MODE_LOOPBACK			0b00000000
#define	SP336_MODE_RS485_HALFDUP	0b00100000
#define	SP336_MODE_RS232			0b01000000
#define	SP336_MODE_RS485_FULLDUP	0b01100000
#define	SP336_MODE_MIXED_HALFDUP	0b10000000
#define	SP336_MODE_LOWPOWER_RX		0b10100000
#define	SP336_MODE_MIXED_FULLDUP	0b11000000
#define	SP336_MODE_HIGHZ			0b11100000
#define	SP336_MODE_SLEW_LIMIT		0b00010000


#define	SP336_2_MODE_LOOPBACK		0b00000000
#define	SP336_2_MODE_MIXED_HALFDUP	0b00000001
#define	SP336_2_MODE_RS232			0b00000010
#define	SP336_2_MODE_MIXED_FULLDUP	0b00000011
#define	SP336_2_MODE_RS485_HALFDUP	0b00000100
#define	SP336_2_MODE_LOWPOWER_RX	0b00000101
#define	SP336_2_MODE_RS485_FULLDUP	0b00000110
#define	SP336_2_MODE_HIGHZ			0b00000111
#define	SP336_2_MODE_SLEW_LIMIT		0b00001000


#define PULSE_CHAN_0 0
#define PULSE_CHAN_1 1

#define NMEA_CHAN_0	0
#define NMEA_CHAN_1	1
#define NMEA_CHAN_2	2
#define NMEA_CHAN_3	3

#define MODBUS_CHAN_0	0
#define MODBUS_CHAN_1	1
#define MODBUS_CHAN_2	2
#define MODBUS_CHAN_3	3

//#define MCU_SWITCH_PIN		IOPORT_CREATE_PIN(PORTB, 6)

#define STATUS_LED_PIN		IOPORT_CREATE_PIN(PORTD,0)
#define RAINGAUGE1_SWITCH	IOPORT_CREATE_PIN(PORTR,0)
#define RAINGAUGE2_SWITCH	IOPORT_CREATE_PIN(PORTR,1)



/////////////////////////////////////////////////////////////////////


#define STATUS_LED_PIN		IOPORT_CREATE_PIN(PORTD,0)
#define RAINGAUGE1_SWITCH	IOPORT_CREATE_PIN(PORTR,0)
#define RAINGAUGE2_SWITCH	IOPORT_CREATE_PIN(PORTR,1)

#define GPRS_SWITCH			IOPORT_CREATE_PIN(PORTF, 3)
//#define GPRS_RESET			IOPORT_CREATE_PIN(PORTF, 1)
#define GPRS_UART_RX		IOPORT_CREATE_PIN(PORTE, 2)
#define GPRS_UART_TX		IOPORT_CREATE_PIN(PORTE, 3)
#define GPRS_STATUS			IOPORT_CREATE_PIN(PORTF, 1)

//#define EXEEPROM_SWITCH		IOPORT_CREATE_PIN(PORTD, 3)
#define EXEEPROM_SCL		IOPORT_CREATE_PIN(PORTE, 1)
#define EXEEPROM_SDA		IOPORT_CREATE_PIN(PORTE, 0)

#define BATTERY_VOLTMETER			ADCA
#define BATTERY_VOLTMETER_CH		ADC_CH0
#define BATTERY_VOLTMETER_PIN		ADCCH_POS_PIN1
#define BATTERY_VOLTMETER_SWITCH	IOPORT_CREATE_PIN(PORTF, 4)

#define SETUP_PCAL9535A



#endif /* V04R01_H_ */