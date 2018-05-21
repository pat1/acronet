
#ifndef HAL_RTC_IMPL_H_
#define HAL_RTC_IMPL_H_

///////////////////////////////////////////////////////////
//
//	This code derives from the ATMEL implementation 
//


typedef struct RTC32_struct2 {
	register8_t CTRL;
	register8_t SYNCCTRL;
	register8_t INTCTRL;
	register8_t INTFLAGS;
	_DWORDREGISTER(CNT);
	_DWORDREGISTER(PER);
	_DWORDREGISTER(COMP);
} RTC32_t2;

#undef RTC32
#define RTC32 (*(RTC32_t2 *)0x0420)

static rtc_callback_t fn_alarm  = NULL;
static rtc_callback_t fn_period = NULL;

//static volatile uint32_t g_epoch = 0;
//static volatile uint32_t g_alarm = 0;


inline void hal_rtc_set_alarm_relative(uint32_t offset)
{
	Assert(offset >= 2);

	hal_rtc_set_alarm(hal_rtc_get_time() + offset);
}


static __always_inline bool rtc_is_busy(void)
{
	return RTC32.SYNCCTRL & RTC32_SYNCBUSY_bm;
}

/*
static inline uint32_t rtc_get_counter(void)
{
	RTC32.SYNCCTRL = RTC32_SYNCCNT_bm;
	while (RTC32.SYNCCTRL & RTC32_SYNCCNT_bm);
	return RTC32.CNT;
}
*/

static void hal_vbat_init(void)
{
	// Enable access to VBAT
	VBAT.CTRL |= VBAT_ACCEN_bm;

	ccp_write_io((void *)&VBAT.CTRL, VBAT_RESET_bm);

	VBAT.CTRL |= VBAT_XOSCFDEN_bm;
	/* This delay is needed to give the voltage in the backup system some
	* time to stabilize before we turn on the oscillator. If we do not
	* have this delay we may get a failure detection.
	*/
	delay_us(200);
//	VBAT.CTRL |= VBAT_XOSCEN_bm | 0;				//Enable this for 1 Hz tick
	VBAT.CTRL |= VBAT_XOSCEN_bm | VBAT_XOSCSEL_bm;  //Enable this for 1024Hz tick

	while (!(VBAT.STATUS & VBAT_XOSCRDY_bm));
}

RET_ERROR_CODE hal_rtc_init(void)
{
	sysclk_enable_module(SYSCLK_PORT_GEN, SYSCLK_RTC);
	// Set up VBAT system and start oscillator
	hal_vbat_init();

	// Disable the RTC32 module before setting it up
	RTC32.CTRL = 0;

	while (rtc_is_busy());

	// Set up maximum period and start at 0
//	RTC32.PER = 0xffffffff;
//	RTC32.PER = 0x0400;
	RTC32.PER = 0x03FF;
	RTC32.CNT = 0;

	while (rtc_is_busy());

	RTC32.INTCTRL = 0;
	RTC32.CTRL = RTC32_ENABLE_bm;

	// Make sure it's sync'ed before return
	while (rtc_is_busy());
	RTC32.INTCTRL = RTC32_OVFINTLVL_MED_gc;

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

	TCC1.CTRLD = TC_EVSEL_CH2_gc | TC_EVACT_RESTART_gc;
	//	TCC0.CTRLD = TC_EVSEL_CH2_gc | TC_EVACT_RESTART_gc;

	TCC0.PER = 0xFFFF;
	TCC1.PER = 0xFFFF;

	TCC0.CNT = 0;
	TCC1.CNT = 0;


	return AC_ERROR_OK;
}

void hal_rtc_set_time(uint32_t time)
{
	RTC32.CTRL = 0;

	while (rtc_is_busy());
	irqflags_t flags = cpu_irq_save();
	TCC0.CNT = (uint16_t) (time & 0xFFFF);
	TCC1.CNT = (uint16_t) (time>>16);
	RTC32_CNT = 0;
	cpu_irq_restore(flags);
	
	RTC32.CTRL = RTC32_ENABLE_bm;
}

inline uint32_t hal_rtc_get_time(void)
{
	irqflags_t flags = cpu_irq_save();
	const uint16_t tcc1 = TCC1.CNT;
	const uint16_t tcc0 = TCC0.CNT;
	cpu_irq_restore(flags);
	
	return ((((uint32_t)tcc1) << 16) | tcc0) ;
}


void hal_rtc_set_period_cb(rtc_callback_t callback)
{
	irqflags_t flags = cpu_irq_save();
	fn_period = callback;
	cpu_irq_restore(flags);
//	RTC32.INTFLAGS = 1;
}


void hal_rtc_set_callback(rtc_callback_t callback)
{
	fn_alarm = callback;
}

void hal_rtc_set_alarm(uint32_t time)
{
	//RTC32.INTCTRL = RTC32_COMPINTLVL_LO_gc;
	//RTC32.COMP = time;
	//RTC32.INTFLAGS = RTC32_COMPIF_bm;
	
	//g_alarm = time;
}

uint16_t hal_rtc_get_millis(void)
{
	RTC32.SYNCCTRL = RTC32_SYNCCNT_bm;
	while (RTC32.SYNCCTRL & RTC32_SYNCCNT_bm);
	return RTC32.CNT;

}


//ISR(RTC32_COMP_vect)
//{
	//RTC32.INTCTRL = 0;
	//if (fn_alarm)
		//fn_alarm(hal_rtc_get_time());
//}


ISR(RTC32_OVF_vect)
{
	if (fn_period)
		fn_period(hal_rtc_get_time());
}

#endif /* HAL_RTC_IMPL_H_ */