/*
 * Copyright © 2001-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef MODBUS_KACO_H
#define MODBUS_KACO_H

#include "modbus.h"

MODBUS_BEGIN_DECLS

/* Modbus_Application_Protocol_V1_1b.pdf Chapter 4 Section 1 Page 5
 * RS232 / RS485 ADU = 253 bytes + slave (1 byte) + CRC (2 bytes) = 256 bytes
 */
#define MODBUS_KACO_MAX_ADU_LENGTH  256

MODBUS_API modbus_t* modbus_new_kaco(const char *device, int baud);

MODBUS_END_DECLS

#endif /* MODBUS_KACO_H */
