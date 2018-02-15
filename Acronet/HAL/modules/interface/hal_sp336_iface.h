#ifndef HAL_SP336_IFACE_H_
#define HAL_SP336_IFACE_H_

//enum {SP336_UART0=0,SP336_UART1,SP336_UART2,SP336_UART3};

void hal_sp336_usart0_rx_cb(const uint8_t c);
void hal_sp336_usart1_rx_cb(const uint8_t c);
void hal_sp336_usart2_rx_cb(const uint8_t c);
void hal_sp336_usart3_rx_cb(const uint8_t c);

//RET_ERROR_CODE hal_sp336_putstring(const char * szBuf,uint16_t lenSZ);

#endif /* HAL_SP336_IFACE_H_ */