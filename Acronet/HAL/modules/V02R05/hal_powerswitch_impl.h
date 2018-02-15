
#ifndef HAL_POWERSWITCH_IMPL_H_
#define HAL_POWERSWITCH_IMPL_H_

//ioport_configure_pin(MCU_SWITCH_PIN, IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);


void hal_powerSwitch_init(void)
{
	ioport_configure_pin(MCU_SWITCH_PIN, IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
}

void hal_powerSwitch_toggle(void)
{
	ioport_configure_pin(MCU_SWITCH_PIN, IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
	delay_ms(2000);
	ioport_configure_pin(MCU_SWITCH_PIN, IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
	while(1) {	gpio_toggle_pin(STATUS_LED_PIN);delay_ms(100); }
}




#endif /* HAL_POWERSWITCH_IMPL_H_ */