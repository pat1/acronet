

#ifndef HAL_BOARD_IFACE_H_
#define HAL_BOARD_IFACE_H_


void hal_board_init(void);
RET_ERROR_CODE hal_board_get_stats(char * const pSZ,uint16_t * const len);

bool hal_sim_get_status(void);

#endif /* HAL_BOARD_IFACE_H_ */