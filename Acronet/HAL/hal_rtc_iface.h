
#ifndef HAL_RTC_IFACE_H_
#define HAL_RTC_IFACE_H_




RET_ERROR_CODE hal_rtc_init(void);
void hal_rtc_set_time(const uint32_t nt);

typedef void (*rtc_callback_t)(uint32_t time);
void hal_rtc_set_callback(rtc_callback_t callback);

void hal_rtc_set_alarm(const uint32_t to);

uint32_t hal_rtc_get_time(void);



#endif /* HAL_RTC_IFACE_H_ */