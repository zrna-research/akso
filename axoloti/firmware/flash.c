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
#include "ch.h"
#include "hal.h"
#include "watchdog.h"
#include "flash.h"

#if BOARD_AKSO
#include "patch.h"
#include "sdram.h"
#include "stm32h7xx_hal.h"

int flash_erase_patch() {
    flash_unlock();
    FLASH_EraseInitTypeDef eraseInit;
    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.Banks = FLASH_BANK_2;
    eraseInit.Sector = 7;
    eraseInit.NbSectors = 1;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_1;

    uint32_t sectorError = 0;
    HAL_FLASHEx_Erase(&eraseInit, &sectorError);
    flash_lock();
    return sectorError != 0xffffffff;
}

int flash_write_patch() {
    flash_unlock();
    const uint32_t patch_flash_location = PATCHFLASHLOC;
    const uint32_t patch_flash_size = PATCHFLASHSIZE;

    uint32_t flash_address = patch_flash_location;
    uint32_t source_address = SDRAM_BANK_ADDR;
    uint8_t word_size = 32;
    uint32_t i;

    for (i = 0; i < (patch_flash_size + (word_size - 1)) / word_size; i++) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, flash_address, source_address);
        flash_address += word_size;
        source_address += word_size;
    }
    flash_lock();

    flash_address = patch_flash_location;
    source_address = SDRAM_BANK_ADDR;
    int err = 0;

    for (i = 0; i < (patch_flash_size + (word_size - 1)) / word_size; i++) {
        if ((*(int32_t *) flash_address) != (*(int32_t *) source_address)) {
            err++;
        }
        flash_address += word_size;
        source_address += word_size;
    }
    return err;
}

void flash_unlock(void) {
    HAL_FLASH_Unlock();
}
void flash_lock(void) {
    HAL_FLASH_Lock();
}
#else

static int flash_WaitForLastOperation(void) {
    while (FLASH->SR == FLASH_SR_BSY) {
#if WATCHDOG_ENABLED
        WWDG->CR = WWDG_CR_T;
#endif
    }
    return FLASH->SR;
}

int flash_Erase_sector(int sector) {
    chSysLock();
    // assume VDD>2.7V
    FLASH->CR &= ~FLASH_CR_PSIZE;
    FLASH->CR |= FLASH_CR_PSIZE_1;
    FLASH->CR &= ~FLASH_CR_SNB;
    FLASH->CR |= FLASH_CR_SER | (sector << 3);
    FLASH->CR |= FLASH_CR_STRT;
    flash_WaitForLastOperation();

    FLASH->CR &= (~FLASH_CR_SER);
    FLASH->CR &= ~FLASH_CR_SER;
    flash_WaitForLastOperation();
    chSysUnlock();
    return 0;
}

int flash_ProgramWord(uint32_t Address, uint32_t Data) {
    int status;

    flash_WaitForLastOperation();

    /* if the previous operation is completed, proceed to program the new data */
    FLASH->CR &= ~FLASH_CR_PSIZE;
    FLASH->CR |= FLASH_CR_PSIZE_1;
    FLASH->CR |= FLASH_CR_PG;

    *(__IO uint32_t*)Address = Data;

    /* Wait for last operation to be completed */
    status = flash_WaitForLastOperation();

    /* if the program operation is completed, disable the PG Bit */
    FLASH->CR &= (~FLASH_CR_PG);

    watchdog_feed();

    /* Return the Program Status */
    return 0;
}

void flash_unlock(void) {
    // unlock sequence
    FLASH->KEYR = 0x45670123;
    FLASH->KEYR = 0xCDEF89AB;
}

void flash_lock(void) {
    FLASH->CR |= FLASH_CR_LOCK;
}
#endif
