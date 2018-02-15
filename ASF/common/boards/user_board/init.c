/**
 * \file
 *
 * \brief User board initialization template
 *
 */

#include <asf.h>
#include <board.h>
#include <conf_board.h>

#include "Acronet/HAL/hal_interface.h"

void board_init(void)
{
	hal_board_init();	
}
