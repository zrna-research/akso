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
#ifndef __FUSB_H
#define __FUSB_H

#if BOARD_AKSO
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_i2c.h"
extern I2C_HandleTypeDef fusb_i2c_handle;

void fusb_init(void);
bool fusb_WriteRegister_on(uint16_t RegisterAddr, uint8_t RegisterValue);
bool fusb_ReadRegister_on(uint16_t RegisterAddr, uint8_t* data);

#endif
#endif
