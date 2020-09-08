#!/bin/bash
${external}/STM32CubeProgrammer/mac/STM32CubeProgrammer.app/Contents/MacOs/bin/STM32_Programmer_CLI -c port=USB1 -d ${firmware_directory}/build/axoloti.bin 0x08000000
