
#ifndef HAL_STATUS_LED_IMPL_H_
#define HAL_STATUS_LED_IMPL_H_


void hal_status_led_blink(uint8_t t)
{
	while( t-- ) {
		gpio_set_pin_high(STATUS_LED_PIN);
		delay_ms(100);
		gpio_set_pin_low(STATUS_LED_PIN);
		delay_ms(50);
	}
	delay_ms(50);
	gpio_set_pin_high(STATUS_LED_PIN);
}

void hal_status_led_toggle(void)
{
	gpio_toggle_pin(STATUS_LED_PIN);
}




#endif /* HAL_STATUS_LED_IMPL_H_ */