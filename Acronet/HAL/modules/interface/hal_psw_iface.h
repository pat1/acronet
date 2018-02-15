
#ifndef HAL_PSW_IFACE_H_
#define HAL_PSW_IFACE_H_

void hal_PCALXXXX_Init(void);
void hal_psw_init(void);
void hal_psw_set(uint8_t val);
void hal_psw_get_value(uint8_t * const pval);
void hal_psw_get_status(uint8_t * const pStat);


#endif /* HAL_PSW_IFACE_H_ */