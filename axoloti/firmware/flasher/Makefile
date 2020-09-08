#############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT = -O2 -ggdb -fomit-frame-pointer -falign-functions=16 -fno-math-errno -D__FPU_PRESENT -specs=nano.specs -specs=nosys.specs -DRACK_ARCH_STM32
endif

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT =
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -fno-rtti -std=c++11 -fno-exceptions
endif

# Enable this if you want the linker to remove unused code and data
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = no
endif

# Linker extra options here.
ifeq ($(USE_LDOPT),)
  USE_LDOPT = --print-memory-usage
endif

# Enable this if you want link time optimizations (LTO)
ifeq ($(USE_LTO),)
  USE_LTO = no
endif

#If enabled, this option allows to compile the application in THUMB mode.
ifeq ($(USE_THUMB),)
  USE_THUMB = yes
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

# If enabled, this option makes the build process faster by not compiling
# modules not used in the current configuration.
ifeq ($(USE_SMART_BUILD),)
  USE_SMART_BUILD = yes
endif

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

# Stack size to be allocated to the Cortex-M process stack. This stack is
# the stack used by the main() thread.
ifeq ($(USE_PROCESS_STACKSIZE),)
  USE_PROCESS_STACKSIZE = 0x450
endif

# Stack size to the allocated to the Cortex-M main/exceptions stack. This
# stack is used for processing interrupts and exceptions.
ifeq ($(USE_EXCEPTIONS_STACKSIZE),)
  USE_EXCEPTIONS_STACKSIZE = 0x400
endif

# Enables the use of FPU on Cortex-M4 (no, softfp, hard).
ifeq ($(USE_FPU),)
  USE_FPU = hard
endif
AXO_OPT_MODULES =

#Axoloti optional modules, here we define defaults if none supplied
ifeq ($(AXO_OPT_MODULES),)
	# AXO_OPT_MODULES = BRAIDS RINGS STREAMS ELEMENTS CLOUDS WARPS
endif

##############################################################################
# Project, sources and paths
#

# Define project name here
PROJECT = flasher
FIRMWARE ?= ..
external ?= $(FIRMWARE)/../../external
CHIBIOS  := $(external)/ChibiOS
BUILDDIR = flasher_build
DEPDIR = $(BUILDDIR)/.dep
CONFDIR  := $(FIRMWARE)/cfg

# Imported source files and paths
include $(FIRMWARE)/ch_version.mk

include $(CHIBIOS)/os/license/license.mk
include $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk/startup_stm32h7xx.mk
include $(CHIBIOS)/os/hal/ports/STM32/STM32H7xx/platform.mk
include $(FIRMWARE)/board.mk

include $(CHIBIOS)/os/rt/rt.mk
include $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC/mk/port_v7m.mk
include $(CHIBIOS)/os/hal/osal/rt-nil/osal.mk
include $(CHIBIOS)/tools/mk/autobuild.mk

include $(CHIBIOS_CONTRIB)/os/hal/hal.mk
include ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/FSMCv1/driver.mk

ALLCSRC += $(PLATFORMSRC_CONTRIB)
ALLINC  += $(PLATFORMINC_CONTRIB)

# Define linker script file here
LDSCRIPT = ../STM32H743trojan.ld

CCFLAGS = -nostdlib -fno-exceptions -fno-rtti -mcpu=cortex-m7 -O3 -fomit-frame-pointer -falign-functions=16 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Wunused-parameter -DCORTEX_USE_FPU=TRUE -DTHUMB_PRESENT -mno-thumb-interwork -DTHUMB_NO_INTERWORKING -mthumb -DTHUMB -std=c++11 -DARM_MATH_CM7 -D__FPU_PRESENT -fno-math-errno -fno-threadsafe-statics -fno-use-cxa-atexit -Wno-unused-parameter -fpermissive -mno-unaligned-access
LDFLAGS = -nostartfiles -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -mthumb -mno-thumb-interwork -Wl,--print-memory-usage

# C sources that can be compiled in ARM or THUMB mode depending on the global
CSRC = $(ALLCSRC) \
       ${FIRMWARE}/axoloti_board.c \
	   $(CHIBIOS)/os/hal/lib/streams/chprintf.c \
	   $(CHIBIOS)/os/hal/lib/streams/memstreams.c \
       ${external}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c \
       ${external}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c \
       ${FIRMWARE}/crc32.c \
       main.c


# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC =

# C sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACSRC =

# C++ sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACPPSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCPPSRC =

# List ASM source files here
ASMSRC =
ASMXSRC = $(STARTUPASM) $(PORTASM) $(OSALASM)

INCDIR = ./ ../ \
         $(CHIBIOS)/os/license \
		 $(CONFDIR) $(ALLINC) $(TESTINC) \
         $(STARTUPINC) $(KERNINC) $(PORTINC) $(OSALINC) \
         $(HALINC) $(PLATFORMINC) $(BOARDINC) $(TESTINC) \
         ${external}/STM32H7xx_HAL_Driver/Inc \
         $(CHIBIOS)/os/various \
         $(CHIBIOS)/os/hal/lib/streams \

#
# Project, sources and paths
##############################################################################

##############################################################################
# Compiler settings
#

MCU  = cortex-m7

TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
# Enable loading with g++ only if you need C++ runtime support.
# NOTE: You can use C++ even without C++ support if you are careful. C++
#       runtime support makes code size explode.
LD   = $(TRGT)gcc
#LD   = $(TRGT)g++
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
OD   = $(TRGT)objdump
SZ   = $(TRGT)size
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

# ARM-specific options here
AOPT =

# THUMB-specific options here
TOPT = -mthumb -DTHUMB

# Define C warning options here
CWARN = -Wall -Wextra -Wstrict-prototypes

# Define C++ warning options here
CPPWARN = -Wall -Wextra

#
# Compiler settings
##############################################################################

##############################################################################
# Start of default section
#

# List all default C defines here, like -D_DEBUG=1
DDEFS =

# List all default ASM defines here, like -D_DEBUG=1
DADEFS =

# List all default directories to look for include files here
DINCDIR =

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS =

#
# End of default section
##############################################################################

##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1
UDEFS =

# Define ASM defines here
UADEFS =

# List all user directories here
UINCDIR =

# List the user directory to look for the libraries here
ULIBDIR = .

# List all user libraries here
ULIBS =


VPATH = .
#
# End of user defines
##############################################################################

RULESPATH = $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk
include $(RULESPATH)/arm-none-eabi.mk
include $(RULESPATH)/rules.mk

SPLITFILES := $(BUILDDIR)/$(PROJECT).sram1.bin \
            $(BUILDDIR)/$(PROJECT).sram3.bin \
            $(BUILDDIR)/$(PROJECT).sdram.bin

%.sram1.bin: %.elf
	@echo Creating $@
	@$(CP) -O binary "$<" "$@"

%.sram3.bin: %.elf
	@echo Creating $@
	@$(CP) -O binary --only-section=.rodata --only-section=.rodata.* --only-section=.data --only-section=.data.* "$<" "$@"

%.sdram.bin: %.elf
	@echo Creating $@
	@$(CP) -O binary --only-section=.sdram.* $< $@

POST_MAKE_ALL_RULE_HOOK: $(SPLITFILES)
