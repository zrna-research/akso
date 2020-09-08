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
#ifndef __AXOBOARD_H
#define __AXOBOARD_H

#include "axoloti_defines.h"

#define ADC_CHANNELS 15
extern unsigned short adcvalues[ADC_CHANNELS];
extern unsigned short adc3values[ADC_CHANNELS];

//extern Mutex Mutex_DMAStream_1_7;

void axoloti_board_init(void);
void adc_init(void);
void adc_configpads(void);
void adc_convert(void);


#define LED1_PORT GPIOG
#define LED1_PIN 6
#define LED2_PORT GPIOC
#define LED2_PIN 6
// SW1 is also BOOT0
#define SW1_PORT GPIOB
#define SW1_PIN 5
#define SW2_PORT GPIOA
#define SW2_PIN 10
#define OCFLAG_PORT GPIOG
#define OCFLAG_PIN 13
#define SDCSW_PORT GPIOD
#define SDCSW_PIN 13
#define SDMIDI SD6




#define IN1_SCALE_PORT GPIOH
#define IN1_SCALE_PIN 9

#define IN1_MODE_PORT GPIOI
#define IN1_MODE_PIN 10

#define IN1_BIAS_SEL_PORT GPIOC
#define IN1_BIAS_SEL_PIN 13

#define IN2_SCALE_PORT GPIOI
#define IN2_SCALE_PIN 7

#define IN2_MODE_PORT GPIOI
#define IN2_MODE_PIN 9

#define IN2_BIAS_SEL_PORT GPIOI
#define IN2_BIAS_SEL_PIN 8

#define OUT1_SCALE_PORT GPIOD
#define OUT1_SCALE_PIN 5

#define OUT1_BAL_EN_PORT GPIOD
#define OUT1_BAL_EN_PIN 4

#define OUT1_BIAS_SEL_PORT GPIOC
#define OUT1_BIAS_SEL_PIN 15



#define OUT2_STR_EN_PORT GPIOG
#define OUT2_STR_EN_PIN 3

#define OUT2_BAL_EN_PORT GPIOH
#define OUT2_BAL_EN_PIN 13

#define OUTPUT2_SCALE_PORT GPIOH
#define OUTPUT2_SCALE_PIN 10

#define OUT2_BIAS_SEL_PORT GPIOC
#define OUT2_BIAS_SEL_PIN 14

#endif
