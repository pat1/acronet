
#ifndef HAL_RTC_IMPL_H_
#define HAL_RTC_IMPL_H_

static rtc_callback_t fn_period = NULL;


__always_inline bool rtc_is_busy(void);
__always_inline bool rtc_is_busy(void)
{
	return RTC.STATUS & RTC_SYNCBUSY_bm;
}


void hal_rtc_set_time(uint32_t time)
{
	RTC.CTRL = RTC_PRESCALER_OFF_gc;

	while (rtc_is_busy());

	TCC0_CNT = time;
	TCC1_CNT = time >> 16;
	RTC_CNT = 0;
	RTC.CTRL = RTC_PRESCALER_DIV1_gc;
}


uint32_t hal_rtc_get_time(void)
{
	return ((((uint32_t)TCC1.CNT) << 16) | TCC0.CNT) ;

}

uint32_t hal_rtc_get_millis(void)
{
	while (rtc_is_busy());
	return RTC_CNT;

}

void hal_rtc_set_alarm(uint32_t time)
{

}


void hal_rtc_set_alarm_relative(uint32_t offset)
{

}




RET_ERROR_CODE hal_rtc_init(void)
{
	OSC.XOSCCTRL = XOSC_TYPE_32KHZ ;

	irqflags_t flags = cpu_irq_save();
	OSC.CTRL |= XOSC_TYPE_32KHZ;
	cpu_irq_restore(flags);

	CLK.RTCCTRL = SYSCLK_RTCSRC_TOSC32 | CLK_RTCEN_bm;

	sysclk_enable_module(SYSCLK_PORT_GEN, SYSCLK_RTC);
	while (RTC.STATUS & RTC_SYNCBUSY_bm) {
		//// Wait for RTC32 sysclk to become stable
	}

	RTC.PER = 0x8000;
	RTC.CNT = 0;
	RTC.CTRL = RTC_PRESCALER_DIV1_gc;
	RTC.INTCTRL = RTC_OVFINTLVL_HI_gc;

	sysclk_enable_module(SYSCLK_PORT_GEN, SYSCLK_EVSYS);
	
	tc_enable(&TCC0);
	tc_enable(&TCC1);
	
	tc_set_direction(&TCC0,TC_UP);
	tc_set_direction(&TCC1,TC_UP);
	

	TCC0.CTRLA = TC_CLKSEL_EVCH0_gc;
	TCC1.CTRLA = TC_CLKSEL_EVCH1_gc;

	//Setting the event channel
	//Channel0 is the RTC overflow event used to
	//clock the TCC0 16bit timer
	EVSYS.CH0MUX = EVSYS_CHMUX_RTC_OVF_gc;

	//Channel1 is the TCC0 overflow event used to
	//clock the TCC1 16bit timer
	EVSYS.CH1MUX = EVSYS_CHMUX_TCC0_OVF_gc;

	//	TCC1.CTRLD = TC_EVSEL_CH2_gc | TC_EVACT_RESTART_gc;
	//	TCC0.CTRLD = TC_EVSEL_CH2_gc | TC_EVACT_RESTART_gc;

	TCC0_PER = 0xFFFF;
	TCC1_PER = 0xFFFF;

	TCC0_CNT = 0x0000;
	TCC1_CNT = 0x0000;
	
	return AC_ERROR_OK;
}

void hal_rtc_set_callback(rtc_callback_t callback)
{

}

void hal_rtc_set_period_cb(rtc_callback_t callback)
{
	fn_period = callback;
}


/**
 * \internal
 * \brief Overflow interrupt handling high counter
 */
ISR(RTC_OVF_vect)
{
	if(fn_period)  fn_period(hal_rtc_get_time());

}

/**
 * \internal
 * \brief Compare interrupt used for alarm
 */
//ISR(RTC_COMP_vect)
//{
//}


#endif /* HAL_RTC_IMPL_H_ */
