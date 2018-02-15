/**
 * \file
 *
 * \brief User board configuration template
 *
 */

#ifndef CONF_BOARD_H
#define CONF_BOARD_H

#define BOARD_XOSC_HZ                   32768
#define BOARD_XOSC_TYPE                 XOSC_TYPE_32KHZ
#define BOARD_XOSC_STARTUP_US           1000000



#define USB_PROBE_PIN   IOPORT_CREATE_PIN(PORTD, 5)

//#define CONF_BOARD_USB_PORT



#endif // CONF_BOARD_H
