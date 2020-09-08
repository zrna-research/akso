/**
 * Copyright (C) 2013, 2014 Johannes Taelman
 *
 * This file is part of Axoloti.
 *
 * Axoloti is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Axoloti is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Axoloti. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __AXOLOTI_DEFINES_H
#define __AXOLOTI_DEFINES_H

#include <stdint.h>

#define PI_F 3.1415927f
#define SAMPLERATE 48000
#define BUFSIZE 16
#define BUFSIZE_POW 4
typedef int32_t int32buffer[BUFSIZE];

#define BOARD_AXOLOTI_V05 1
#define BOARD_AKSO 1
#define SDCARD_ENABLED 1

#define SPILINK_ENABLED 0
#define AXOLOTI_CONTROL_ENABLED 0

#define DEBUG_INT_ON_GPIO 0

#define DEAD_CODE 0

// firmware version 1.1.0.0
#define FWVERSION1 3
#define FWVERSION2 0
#define FWVERSION3 1
#define FWVERSION4 0

#endif
