#
# embedXcode
# ----------------------------------
# Embedded Computing on Xcode
#
# Copyright © Rei VILO, 2010-2017
# http://embedxcode.weebly.com
# All rights reserved
#
#
# Last update: Nov 23, 2016 release 5.3.8



# Energia LaunchPad MSP430 and FR5739 specifics
# ----------------------------------
#
PLATFORM         := Energia
BUILD_CORE       := msp430

PLATFORM_TAG      = ENERGIA=$(ENERGIA_RELEASE) ARDUINO=$(ARDUINO_RELEASE) EMBEDXCODE=$(RELEASE_NOW) ENERGIA_ARCH_MSP430 ENERGIA_$(BOARD_TAG) $(filter __%__ ,$(GCC_PREPROCESSOR_DEFINITIONS))

APPLICATION_PATH := $(ENERGIA_18_PATH)

#ENERGIA_RELEASE := $(shell tail -c2 $(APPLICATION_PATH)/lib/version.txt)
#ARDUINO_RELEASE := $(shell head -c4 $(APPLICATION_PATH)/lib/version.txt | tail -c3)
ENERGIA_RELEASE  := 10610
ARDUINO_RELEASE  := 10610
PLATFORM_VERSION := MSP430 $(ENERGIA_MSP430_RELEASE) for Energia $(ENERGIA_RELEASE)
BOARD_TAG        := $(BOARD_TAG_18)

HARDWARE_PATH     = $(ENERGIA_MSP430_PATH)/hardware/msp430/$(ENERGIA_MSP430_RELEASE)
TOOL_CHAIN_PATH   := $(APPLICATION_PATH)/hardware/tools/msp430/bin
OTHER_TOOLS_PATH  = $(ENERGIA_18_PATH)/hardware/tools/DSLite


# Uploader
# ----------------------------------
#
ifeq ($(UPLOADER), mspdebug)
    # mspdebug for MSP-EXP430G2553LP and MSP-EXP430FR5739LP only
    UPLOADER          = mspdebug
    UPLOADER_PATH     = $(APPLICATION_PATH)/hardware/tools/msp430/bin
    UPLOADER_EXEC     = $(UPLOADER_PATH)/mspdebug
    UPLOADER_PROTOCOL = $(call PARSE_BOARD,$(BOARD_TAG),upload.protocol)
    UPLOADER_OPTS     = $(UPLOADER_PROTOCOL) --force-reset

    # MSP-EXP430FR5739LP requires a specific command
    #
    ifeq ($(BOARD_TAG), MSP-EXP430FR5739LP)
        UPLOADER_COMMAND = load
    else
        UPLOADER_COMMAND = prog
    endif
else
    # General case with DSLite
    UPLOADER          = DSLite
    UPLOADER_PATH     = $(OTHER_TOOLS_PATH)/DebugServer/bin
    UPLOADER_EXEC     = $(UPLOADER_PATH)/DSLite
    UPLOADER_OPTS     = $(OTHER_TOOLS_PATH)/$(VARIANT).ccxml

    COMMAND_UPLOAD = $(UPLOADER_EXEC) load -c $(UPLOADER_OPTS) -f $(TARGET_ELF)
endif

CORE_LIB_PATH    := $(HARDWARE_PATH)/cores/msp430
APP_LIB_PATH     := $(HARDWARE_PATH)/libraries
BOARDS_TXT       := $(HARDWARE_PATH)/boards.txt
#BOARDS_TXT      := $(APPLICATION_PATH)/hardware/msp430/boards.txt

# Sketchbook/Libraries path
# ----------------------------------
# wildcard required for ~ management
# ?ibraries required for libraries and Libraries
#
ifeq ($(USER_LIBRARY_DIR)/Energia15/preferences.txt,)
    $(error Error: run Energia once and define the sketchbook path)
endif

ifeq ($(wildcard $(SKETCHBOOK_DIR)),)
    SKETCHBOOK_DIR = $(shell grep sketchbook.path $(wildcard ~/Library/Energia15/preferences.txt) | cut -d = -f 2)
endif

ifeq ($(wildcard $(SKETCHBOOK_DIR)),)
    $(error Error: sketchbook path not found)
endif
USER_LIB_PATH  = $(wildcard $(SKETCHBOOK_DIR)/?ibraries)


# Horrible patch for core libraries
# ----------------------------------
#
# If driverlib/libdriverlib.a is available, exclude driverlib/
#
CORE_LIB_PATH   = $(HARDWARE_PATH)/cores/msp430
CORE_LIB_PATH  += $(HARDWARE_PATH)/cores/msp430/avr

#CORE_A   = $(CORE_LIB_PATH)/driverlib/libdriverlib.a
#
#BUILD_CORE_LIB_PATH = $(shell find $(CORE_LIB_PATH) -type d)
#ifneq ($(wildcard $(CORE_A)),)
#    BUILD_CORE_LIB_PATH := $(filter-out %/driverlib,$(BUILD_CORE_LIB_PATH))
#endif

BUILD_CORE_CPP_SRCS = $(filter-out %program.cpp %main.cpp,$(foreach dir,$(CORE_LIB_PATH),$(wildcard $(dir)/*.cpp))) # */
BUILD_CORE_C_SRCS   = $(foreach dir,$(CORE_LIB_PATH),$(wildcard $(dir)/*.c)) # */

BUILD_CORE_OBJ_FILES  = $(BUILD_CORE_C_SRCS:.c=.c.o) $(BUILD_CORE_CPP_SRCS:.cpp=.cpp.o)
BUILD_CORE_OBJS       = $(patsubst $(HARDWARE_PATH)/%,$(OBJDIR)/%,$(BUILD_CORE_OBJ_FILES))

CORE_LIBS_LOCK = 1
# ----------------------------------


# Horrible patch for Ethernet library
# ----------------------------------
#
# APPlication Arduino/chipKIT/Digispark/Energia/Maple/Microduino/Teensy/Wiring sources
#
APP_LIB_PATH     := $(HARDWARE_PATH)/libraries

ifneq ($(APP_LIBS_LIST),0)
msp430_20    = $(foreach dir,$(APP_LIB_PATH),$(patsubst %,$(dir)/%,$(APP_LIBS_LIST)))
msp430_20   += $(foreach dir,$(APP_LIB_PATH),$(patsubst %,$(dir)/%/utility,$(APP_LIBS_LIST)))

APP_LIB_CPP_SRC = $(foreach dir,$(msp430_20),$(wildcard $(dir)/*.cpp)) # */
APP_LIB_C_SRC   = $(foreach dir,$(msp430_20),$(wildcard $(dir)/*.c)) # */
APP_LIB_H_SRC   = $(foreach dir,$(msp430_20),$(wildcard $(dir)/*.h)) # */

APP_LIB_OBJS     = $(patsubst $(HARDWARE_PATH)/%.cpp,$(OBJDIR)/%.cpp.o,$(APP_LIB_CPP_SRC))
APP_LIB_OBJS    += $(patsubst $(HARDWARE_PATH)/%.c,$(OBJDIR)/%.c.o,$(APP_LIB_C_SRC))

BUILD_APP_LIBS_LIST = $(subst $(APP_LIB_PATH)/, ,$(APP_LIB_CPP_SRC))
BUILD_APP_LIB_PATH  = $(msp430_20) $(foreach dir,$(APP_LIB_PATH),$(patsubst %,$(dir)/%,$(APP_LIBS_LIST)))
endif

BUILD_APP_LIB_PATH     = $(APPLICATION_PATH)/libraries

msp430_10    = $(foreach dir,$(BUILD_APP_LIB_PATH),$(patsubst %,$(dir)/%,$(APP_LIBS_LIST)))
msp430_10   += $(foreach dir,$(BUILD_APP_LIB_PATH),$(patsubst %,$(dir)/%/utility,$(APP_LIBS_LIST)))
msp430_10   += $(foreach dir,$(BUILD_APP_LIB_PATH),$(patsubst %,$(dir)/%/src,$(APP_LIBS_LIST)))
msp430_10   += $(foreach dir,$(BUILD_APP_LIB_PATH),$(patsubst %,$(dir)/%/src/utility,$(APP_LIBS_LIST)))
msp430_10   += $(foreach dir,$(BUILD_APP_LIB_PATH),$(patsubst %,$(dir)/%/src/arch/$(BUILD_CORE),$(APP_LIBS_LIST)))

BUILD_APP_LIB_CPP_SRC = $(foreach dir,$(msp430_10),$(wildcard $(dir)/*.cpp)) # */
BUILD_APP_LIB_C_SRC   = $(foreach dir,$(msp430_10),$(wildcard $(dir)/*.c)) # */
BUILD_APP_LIB_H_SRC   = $(foreach dir,$(msp430_10),$(wildcard $(dir)/*.h)) # */

BUILD_APP_LIB_OBJS     = $(patsubst $(APPLICATION_PATH)/%.cpp,$(OBJDIR)/%.cpp.o,$(BUILD_APP_LIB_CPP_SRC))
BUILD_APP_LIB_OBJS    += $(patsubst $(APPLICATION_PATH)/%.c,$(OBJDIR)/%.c.o,$(BUILD_APP_LIB_C_SRC))

APP_LIBS_LOCK = 1
# ----------------------------------

# Rules for making a c++ file from the main sketch (.pde)
#
PDEHEADER      = \\\#include \"Energia.h\"  


# Tool-chain names
#
CC      = $(TOOL_CHAIN_PATH)/msp430-gcc
CXX     = $(TOOL_CHAIN_PATH)/msp430-g++
AR      = $(TOOL_CHAIN_PATH)/msp430-ar
OBJDUMP = $(TOOL_CHAIN_PATH)/msp430-objdump
OBJCOPY = $(TOOL_CHAIN_PATH)/msp430-objcopy
SIZE    = $(TOOL_CHAIN_PATH)/msp430-size
NM      = $(TOOL_CHAIN_PATH)/msp430-nm

BOARD          = $(call PARSE_BOARD,$(BOARD_TAG),board)
#LDSCRIPT = $(call PARSE_BOARD,$(BOARD_TAG),ldscript)
VARIANT        = $(call PARSE_BOARD,$(BOARD_TAG),build.variant)
VARIANT_PATH   = $(HARDWARE_PATH)/variants/$(VARIANT)

OPTIMISATION   = -Os

MCU_FLAG_NAME  = mmcu
MCU = $(call PARSE_BOARD,$(BOARD_TAG),build.mcu)
#EXTRA_LDFLAGS  =
#EXTRA_LDFLAGS = -T$(CORE_LIB_PATH)/$(LDSCRIPT)
#EXTRA_CPPFLAGS = $(addprefix -D, $(PLATFORM_TAG)) -I$(VARIANT_PATH)

INCLUDE_PATH     = $(VARIANT_PATH)
#INCLUDE_PATH    += $(APPLICATION_PATH)/hardware/tools/cc3200/include
INCLUDE_PATH    += $(CORE_LIB_PATH)
INCLUDE_PATH    += $(BUILD_CORE_LIB_PATH)
#INCLUDE_PATH    += $(APPLICATION_PATH)/hardware/cc3200/cores/cc3200/driverlib
INCLUDE_PATH    += $(APP_LIB_PATH)
INCLUDE_PATH    += $(BUILD_APP_LIB_PATH)
INCLUDE_PATH    += $(sort $(dir $(APP_LIB_H_SRC) $(BUILD_APP_LIB_H_SRC)))


# Flags for gcc, g++ and linker
# ----------------------------------
#
# Common CPPFLAGS for gcc, g++, assembler and linker
#
CPPFLAGS     = $(OPTIMISATION) $(WARNING_FLAGS)
CPPFLAGS    += -c -Os -Wall -ffunction-sections -fdata-sections
CPPFLAGS    += -$(MCU_FLAG_NAME)=$(MCU) -DF_CPU=$(F_CPU)
CPPFLAGS    += $(addprefix -I, $(INCLUDE_PATH))
CPPFLAGS    += $(addprefix -D, $(PLATFORM_TAG))

# Specific CFLAGS for gcc only
# gcc uses CPPFLAGS and CFLAGS
#
CFLAGS       = #

# Specific CXXFLAGS for g++ only
# g++ uses CPPFLAGS and CXXFLAGS
#
CXXFLAGS    = -fno-exceptions -fno-threadsafe-statics

# Specific ASFLAGS for gcc assembler only
# gcc assembler uses CPPFLAGS and ASFLAGS
#
ASFLAGS      = --asm_extension=S

# Specific LDFLAGS for linker only
# linker uses CPPFLAGS and LDFLAGS
#
LDFLAGS      = $(OPTIMISATION) $(WARNING_FLAGS)
LDFLAGS     += -fno-rtti -fno-exceptions -Wl,--gc-sections,-u,main 
LDFLAGS     += -$(MCU_FLAG_NAME)=$(MCU) -DF_CPU=$(F_CPU)
LDFLAGS     += -LBuilds -lm

# Specific OBJCOPYFLAGS for objcopy only
# objcopy uses OBJCOPYFLAGS only
#
OBJCOPYFLAGS  = -Oihex


# Target
#
TARGET_HEXBIN = $(TARGET_HEX)


# Commands
# ----------------------------------
#
# Link command
#
COMMAND_LINK    = $(CC) $(OUT_PREPOSITION)$@ $(LOCAL_OBJS) $(TARGET_A) $(LDFLAGS)


