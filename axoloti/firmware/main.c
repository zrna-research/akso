/**
 * Copyright (C) 2013 - 2017 Johannes Taelman
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

#include "axoloti_board.h"
#include "axoloti_control.h"
#include "axoloti_defines.h"
#include "axoloti_math.h"
#include "ch.h"
#include "chprintf.h"
#include "codec.h"
#include "exceptions.h"
#include "hal.h"
#include "midi.h"
#include "fusb.h"

#include "patch.h"
#include "pconnection.h"
#if SDCARD_ENABLED
#include "sdcard.h"
#endif
#include "spilink.h"
#include "string.h"
#include "sysmon.h"
#include "ui.h"
#include "usbcfg.h"
#include "usbh.h"
#include "watchdog.h"
#include <stdio.h>

#include "midi_usb.h"
#include "sdram.h"

#include "audio_io.h"

int main(void) {
    exception_clear();

#if !BOARD_AKSO
    SYSCFG->MEMRMP |= 0x03;
#endif

    halInit();
    chSysInit();

    audio_io_init();

#if SDCARD_ENABLED
    mpuConfigureRegion(MPU_REGION_7,
                       0x2407e800U,
                       MPU_RASR_ATTR_AP_RW_RW |
                       MPU_RASR_ATTR_SHARED_DEVICE |
                       MPU_RASR_ATTR_NON_CACHEABLE |
                       MPU_RASR_SIZE_8K |
                       MPU_RASR_ENABLE);
    sdcard_init();
#endif
    sysmon_init();
    exception_init();

    InitPatch0();

    InitPConnection();
    midi_usb_init();

//    palSetPadMode(GPIOC, 1, PAL_MODE_OUTPUT_PUSHPULL);
//    palSetPad(GPIOC, 1);

    chThdSleepMilliseconds(10);

    palSetPadMode(SW2_PORT, SW2_PIN, PAL_MODE_INPUT_PULLDOWN);

    axoloti_board_init();

    fusb_init();

#if SPILINK_ENABLED
    // connect PB10 to ground to enable slave mode
    bool is_master = palReadPad(GPIOB, GPIOB_PIN10);
#else
    bool is_master = true;
#endif

    start_dsp_thread();
    codec_init(is_master);
    adc_init();
    axoloti_math_init();
    midi_init();

#if AXOLOTI_CONTROL_ENABLED
    if (!palReadPad(SW2_PORT, SW2_PIN)) {
        chThdSleepMilliseconds(1);
    }
    axoloti_control_init();
#endif

#if SPILINK_ENABLED
    pThreadSpilink = 0;
    spilink_init(is_master);
    ui_init();
#endif

    configSDRAM();

    MY_USBH_Init();


#if SDCARD_ENABLED
    sdcard_attemptMountIfUnmounted();
#endif

    if (!exception_check()) {
        // only try booting a patch when no exception is to be reported
        // TODO: maybe only skip startup patch when exception was caused by startup patch
        // and button S2 is not pressed

#if SDCARD_ENABLED
        if (fs_ready) {
            LoadPatchStartSD();
        }
#endif
        // if no patch booting or running yet
        // try loading from flash
        if (patchStatus == STOPPED) {
            LoadPatchStartFlash();
        }
    }

	while (1) {
		chThdSleepMilliseconds(1000);
	}
}

void HAL_Delay(uint32_t n) {
    chThdSleepMilliseconds(n);
}
