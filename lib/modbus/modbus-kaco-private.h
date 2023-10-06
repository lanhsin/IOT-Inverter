/*
 * Copyright © 2001-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef MODBUS_KACO_PRIVATE_H
#define MODBUS_KACO_PRIVATE_H

#include <stdint.h>
#include <termios.h>

#define _MODBUS_KACO_HEADER_LENGTH      0
#define _MODBUS_KACO_PRESET_REQ_LENGTH  5
#define _MODBUS_KACO_CHECKSUM_LENGTH    2

typedef struct _modbus_kaco {
    /* Device: "/dev/ttyS0", "/dev/ttyUSB0" or "/dev/tty.USA19*" on Mac OS X. */
    char *device;
    /* Bauds: 9600, 19200, 57600, 115200, etc */
    int baud;
    /* Data bit */
    uint8_t data_bit;
    /* Stop bit */
    uint8_t stop_bit;
    /* Parity: 'N', 'O', 'E' */
    char parity;
    /* Save old termios settings */
    struct termios old_tios;
} modbus_kaco_t;

#endif /* MODBUS_KACO_PRIVATE_H */
