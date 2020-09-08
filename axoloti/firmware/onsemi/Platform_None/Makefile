# System configuration
CC = gcc
RM=rm -rf

# Assembler, Compiler and Linker flags and linker settings
COMPILER_FLAGS += -c -g -O0 -Wall -ffunction-sections -fdata-sections
ASSEMBLER_FLAGS= -c -g -O0 -x assembler-with-cpp
LINKER_FLAGS=-static -Wl,-Map=$(BIN_DIR)/output_bin.map

COMPILER_FLAGS += -DPLATFORM_NONE \
				  -DFSC_HAVE_SRC \
				  #-DFSC_HAVE_DRP \
				  #-DFSC_HAVE_SNK \

#Include path
INCLUDE_PATH = -I.\
			   -I $(CORE_SRC)/\
			   -I $(CORE_SRC)/modules\
			   -I $(CORE_SRC)/vdm\
			   -I $(CORE_SRC)/vdm/DisplayPort

COMPILER_FLAGS += -c $(INCLUDE_PATH)

# Define output directory
OBJECT_DIR = Debug
BIN_DIR = $(OBJECT_DIR)
CORE_SRC := $(realpath ../core)

SRC = $(wildcard $(CORE_SRC)/*.c) \
	  $(wildcard $(CORE_SRC)/modules/*.c) \
	  $(wildcard $(CORE_SRC)/vdm/*.c) \
	  $(wildcard $(CORE_SRC)/vdm/DisplayPort/*.c) \
	  $(wildcard *.c) \

OBJS := $(SRC:%.c=$(OBJECT_DIR)/%.o) $(SRCSASM:%.s=$(OBJECT_DIR)/%.o)
OBJS := $(OBJS:%.S=$(OBJECT_DIR)/%.o)

###############
# Build project
# Major targets
###############
all: buildelf

buildelf: $(OBJS) 
	$(CC) -o "$(BIN_DIR)/out_bin.elf" $(OBJS) $(LINKER_FLAGS)

clean:
	$(RM) -r $(BIN_DIR)

##################
# Implicit targets
##################
$(OBJECT_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(COMPILER_FLAGS) $< -o $@

$(OBJECT_DIR)/%.o: %.s
	mkdir -p $(dir $@)
	$(CC) $(ASSEMBLER_FLAGS) $< -o $@
	
$(OBJECT_DIR)/%.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(ASSEMBLER_FLAGS) $< -o $@
