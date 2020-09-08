/**
 * Copyright (C) 2015 Johannes Taelman
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

#include "watchdog.h"

#include "ch.h"
#include "hal.h"

#define WATCHDOG_ENABLED 0


void watchdog_init(void) {
#if WATCHDOG_ENABLED
  RCC->APB3ENR |= RCC_APB3ENR_WWDG1EN;
  // disable watchdog when debugger active?
  DBGMCU->APB3FZ1 |= DBGMCU_APB3FZ1_DBG_WWDG1;
  WWDG1->CFR = WWDG_CFR_W | WWDG_CFR_WDGTB_1 | WWDG_CFR_WDGTB_1 | WWDG_CFR_EWI;
  WWDG1->CR = WWDG_CR_T | WWDG_CR_WDGA;
  WWDG1->SR = 0;
  nvicEnableVector(WWDG_IRQn, CORTEX_PRIO_MASK(0));
#endif
}

void watchdog_feed(void) {
#if WATCHDOG_ENABLED
  if ((WWDG1->CR & WWDG_CR_T) != WWDG_CR_T)
    WWDG1->CR = WWDG_CR_T;
#endif
}
