#!/bin/bash
${external}/STM32CubeProgrammer/linux/bin/STM32_Programmer.sh -c port=USB1 -d ${firmware_directory}/build/axoloti.bin 0x08000000
