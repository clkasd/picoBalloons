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
# Last update: Jan 26, 2016 release 6.1.2






# Sketch unicity test and extension
# ----------------------------------
#
ifndef SKETCH_EXTENSION
    ifeq ($(words $(wildcard *.pde) $(wildcard *.ino)), 0)
        $(error No pde or ino sketch)
    endif

    ifneq ($(words $(wildcard *.pde) $(wildcard *.ino)), 1)
        $(error More than 1 pde or ino sketch)
    endif

    ifneq ($(wildcard *.pde),)
        SKETCH_EXTENSION := pde
    else ifneq ($(wildcard *.ino),)
        SKETCH_EXTENSION := ino
    else
        $(error Extension error)
    endif
endif

ifneq ($(MULTI_INO),1)
ifneq ($(SKETCH_EXTENSION),__main_cpp_only__)
    ifneq ($(SKETCH_EXTENSION),_main_cpp_only_)
        ifneq ($(SKETCH_EXTENSION),cpp)
            ifeq ($(words $(wildcard *.$(SKETCH_EXTENSION))), 0)
                $(error No $(SKETCH_EXTENSION) sketch)
            endif

            ifneq ($(words $(wildcard *.$(SKETCH_EXTENSION))), 1)
                $(error More than one $(SKETCH_EXTENSION) sketch)
            endif
        endif
    endif
endif
endif


# Board selection
# ----------------------------------
# Board specifics defined in .xconfig file
# BOARD_TAG and AVRDUDE_PORT 
#
ifneq ($(MAKECMDGOALS),boards)
    ifneq ($(MAKECMDGOALS),clean)
        ifndef BOARD_TAG
            $(error BOARD_TAG not defined)
        endif
    endif
endif

ifndef BOARD_PORT
    BOARD_PORT = /dev/tty.usb*
endif


# Path to applications folder
#
# $(HOME) same as $(wildcard ~)
# $(USER_PATH)/Library same as $(USER_LIBRARY_DIR)
#
USER_PATH      := $(HOME)
EMBEDXCODE_APP  = $(USER_LIBRARY_DIR)/embedXcode
PARAMETERS_TXT  = $(EMBEDXCODE_APP)/parameters.txt

APPLICATIONS_PATH := /Applications


# APPlications full paths
# ----------------------------------
#
# Welcome unified 1.8.0 release for all Arduino.CC and Genuino, Arduino.ORG boards!
#
ifneq ($(wildcard $(APPLICATIONS_PATH)/Arduino.app),)
    ARDUINO_APP   := $(APPLICATIONS_PATH)/Arduino.app
#else ifneq ($(wildcard $(APPLICATIONS_PATH)/ArduinoCC.app),)
#    ARDUINO_APP   := $(APPLICATIONS_PATH)/ArduinoCC.app
#else ifneq ($(wildcard $(APPLICATIONS_PATH)/ArduinoORG.app),)
#    ARDUINO_APP   := $(APPLICATIONS_PATH)/ArduinoORG.app
endif

# Unified Arduino.app 1.8.1 for all Arduino.CC and Genuino, Arduino.ORG boards
#
ifneq ($(wildcard $(APPLICATIONS_PATH)/Arduino.app),)
    ifneq ($(shell grep -e '$(ARDUINO_IDE_RELEASE)' $(APPLICATIONS_PATH)/Arduino.app/Contents/Java/lib/version.txt),)
        ARDUINO_180_APP = $(APPLICATIONS_PATH)/Arduino.app
    endif
endif

# Arduino
#
ARDUINO_PATH        := $(ARDUINO_APP)/Contents/Java
ARDUINO_180_PATH     := $(ARDUINO_180_APP)/Contents/Java


# Other IDEs
#
WIRING_APP       = $(APPLICATIONS_PATH)/Wiring.app
ENERGIA_18_APP   = $(APPLICATIONS_PATH)/Energia.app
MAPLE_APP        = $(APPLICATIONS_PATH)/MapleIDE.app
MBED_APP         = $(EMBEDXCODE_APP)/mbed-$(MBED_SDK_RELEASE)

include $(MAKEFILE_PATH)/About.mk
RELEASE_NOW = $(shell echo $(EMBEDXCODE_RELEASE) | sed 's/\.//g')


# Additional boards for Arduino 1.8.0 Boards Manager
# ----------------------------------
# Unified Arduino.app for Arduino.CC and Genuino, Arduino.ORG boards
# Only if ARDUINO_180_APP exists
#

ifneq ($(ARDUINO_180_APP),)

    ARDUINO_180_PACKAGES_PATH = $(HOME)/Library/Arduino15/packages

# find $(ARDUINO_180_PACKAGES_PATH) -name arm-none-eabi-gcc -type d
# find $(ARDUINO_180_PACKAGES_PATH) -name avr-gcc -type d


# Arduino path for Arduino 1.8.0
#
ARDUINO_AVR_1 = $(ARDUINO_180_PACKAGES_PATH)/arduino

ifneq ($(wildcard $(ARDUINO_AVR_1)/hardware/avr),)
    ARDUINO_AVR_APP     = $(ARDUINO_AVR_1)
    ARDUINO_AVR_PATH    = $(ARDUINO_AVR_APP)
    ARDUINO_180_AVR_BOARDS  = $(ARDUINO_AVR_APP)/hardware/avr/$(ARDUINO_AVR_RELEASE)/boards.txt
endif

ARDUINO_SAM_1 = $(ARDUINO_180_PACKAGES_PATH)/arduino

ifneq ($(wildcard $(ARDUINO_SAM_1)/hardware/sam),)
    ARDUINO_SAM_APP     = $(ARDUINO_SAM_1)
    ARDUINO_SAM_PATH    = $(ARDUINO_SAM_APP)
    ARDUINO_180_SAM_BOARDS  = $(ARDUINO_SAM_APP)/hardware/sam/$(ARDUINO_SAM_RELEASE)/boards.txt
endif

ARDUINO_SAMD_1 = $(ARDUINO_180_PACKAGES_PATH)/arduino

ifneq ($(wildcard $(ARDUINO_SAMD_1)/hardware/samd),)
    ARDUINO_SAMD_APP     = $(ARDUINO_SAMD_1)
    ARDUINO_SAMD_PATH    = $(ARDUINO_SAMD_APP)
    ARDUINO_180_SAMD_BOARDS  = $(ARDUINO_SAMD_APP)/hardware/samd/$(ARDUINO_SAMD_RELEASE)/boards.txt
endif

# ESP8266 NodeMCU.app path for Arduino 1.8.0
#
ESP8266_1 = $(ARDUINO_180_PACKAGES_PATH)/esp8266

ifneq ($(wildcard $(ESP8266_1)),)
    ESP8266_APP     = $(ESP8266_1)
    ESP8266_PATH    = $(ESP8266_APP)
    ESP8266_BOARDS  = $(ESP8266_1)/hardware/esp8266/$(ESP8266_RELEASE)/boards.txt
endif

endif # end  Arduino 1.8.0


# Additional boards for Energia 18 Boards Manager
# ----------------------------------
# Energia.app
#
ENERGIA_PACKAGES_PATH = $(HOME)/Library/Energia15/packages/energia

ENERGIA_18_PATH    = $(ENERGIA_18_APP)/Contents/Java
ENERGIA_18_MSP430_BOARDS       = $(ENERGIA_18_PATH)/hardware/energia/msp430/boards.txt
#ENERGIA_18_C2000_BOARDS        = $(ENERGIA_18_PATH)/hardware/c2000/boards.txt

ENERGIA_TIVAC_1    = $(ENERGIA_PACKAGES_PATH)/hardware/tivac/$(ENERGIA_TIVAC_RELEASE)
ifneq ($(wildcard $(ENERGIA_TIVAC_1)),)
    ENERGIA_TIVAC_APP    = $(ENERGIA_TIVAC_1)
    ENERGIA_TIVAC_PATH   = $(ENERGIA_PACKAGES_PATH)
    ENERGIA_18_TIVAC_BOARDS = $(ENERGIA_TIVAC_1)/boards.txt
endif

ENERGIA_MSP430_1    = $(ENERGIA_PACKAGES_PATH)/hardware/msp430/$(ENERGIA_IDE_MSP430_RELEASE)
ifneq ($(wildcard $(ENERGIA_MSP430_1)),)
    ENERGIA_MSP430_APP    = $(ENERGIA_MSP430_1)
    ENERGIA_MSP430_PATH   = $(ENERGIA_PACKAGES_PATH)
    ENERGIA_19_MSP430_BOARDS = $(ENERGIA_MSP430_1)/boards.txt
endif

ENERGIA_CC3200_1    = $(ENERGIA_PACKAGES_PATH)/hardware/cc3200/$(ENERGIA_CC3200_RELEASE)
ifneq ($(wildcard $(ENERGIA_CC3200_1)),)
    ENERGIA_CC3200_APP    = $(ENERGIA_CC3200_1)
    ENERGIA_CC3200_PATH   = $(ENERGIA_PACKAGES_PATH)
    ENERGIA_18_CC3200_BOARDS = $(ENERGIA_CC3200_1)/boards.txt
endif

ENERGIA_CC3200_EMT_1    = $(ENERGIA_PACKAGES_PATH)/hardware/cc3200emt/$(ENERGIA_CC3200_EMT_RELEASE)
ifneq ($(wildcard $(ENERGIA_CC3200_EMT_1)),)
    ENERGIA_CC3200_EMT_APP    = $(ENERGIA_CC3200_EMT_1)
    ENERGIA_CC3200_EMT_PATH   = $(ENERGIA_PACKAGES_PATH)
    ENERGIA_18_CC3200_EMT_BOARDS = $(ENERGIA_CC3200_EMT_1)/boards.txt
endif

#ENERGIA_18_CC2600_EMT_BOARDS   = $(ENERGIA_18_PATH)/hardware/cc2600emt/boards.txt
ENERGIA_MSP432_EMT_1    = $(ENERGIA_PACKAGES_PATH)/hardware/msp432/$(ENERGIA_MSP432_EMT_RELEASE)
ifneq ($(wildcard $(ENERGIA_MSP432_EMT_1)),)
    ENERGIA_MSP432_EMT_APP    = $(ENERGIA_MSP432_EMT_1)
    ENERGIA_MSP432_EMT_PATH   = $(ENERGIA_PACKAGES_PATH)
    ENERGIA_18_MSP432_EMT_BOARDS = $(ENERGIA_MSP432_EMT_1)/boards.txt
endif


# Other boards
# ----------------------------------
#

# Teensyduino.app path
#
TEENSY_0    = $(APPLICATIONS_PATH)/Teensyduino.app
ifneq ($(wildcard $(TEENSY_0)),)
    TEENSY_APP    = $(TEENSY_0)
else
    TEENSY_APP    = $(ARDUINO_APP)
endif

# Microduino.app path
#
MICRODUINO_0 = $(APPLICATIONS_PATH)/Microduino.app

ifneq ($(wildcard $(MICRODUINO_0)),)
    MICRODUINO_APP = $(MICRODUINO_0)
else
    MICRODUINO_APP = $(ARDUINO_APP)
endif


# Check at least one IDE installed
#
ifeq ($(wildcard $(ARDUINO_180_APP)),)
ifeq ($(wildcard $(ESP8266_APP)),)
    ifeq ($(wildcard $(LINKIT_ARM_APP)),)
    ifeq ($(wildcard $(WIRING_APP)),)
    ifeq ($(wildcard $(ENERGIA_18_APP)),)
    ifeq ($(wildcard $(MAPLE_APP)),)
        ifeq ($(wildcard $(TEENSY_APP)),)
        ifeq ($(wildcard $(GLOWDECK_APP)),)
        ifeq ($(wildcard $(DIGISTUMP_APP)),)
        ifeq ($(wildcard $(MICRODUINO_APP)),)
        ifeq ($(wildcard $(LIGHTBLUE_APP)),)
            ifeq ($(wildcard $(INTEL_APP)),)
            ifeq ($(wildcard $(ROBOTIS_APP)),)
            ifeq ($(wildcard $(RFDUINO_APP)),)
            ifeq ($(wildcard $(REDBEARLAB_APP)),)
                ifeq ($(wildcard $(LITTLEROBOTFRIENDS_APP)),)
                ifeq ($(wildcard $(PANSTAMP_AVR_APP)),)
                ifeq ($(wildcard $(MBED_APP)/*),) # */
                ifeq ($(wildcard $(EDISON_YOCTO_APP)/*),) # */
                    ifeq ($(wildcard $(SPARK_APP)/*),) # */
                    ifeq ($(wildcard $(ADAFRUIT_AVR_APP)),)
                        $(error Error: no application found)
                    endif
                    endif
                endif
                endif
                endif
                endif
            endif
            endif
            endif
            endif
        endif
        endif
        endif
        endif
        endif
    endif
    endif
    endif
    endif
endif
endif


# Arduino-related nightmares
# ----------------------------------
#
# Get Arduino release
# Gone Arduino 1.0, 1.5 Java 6 and 1.5 Java 7 triple release nightmare
#
ifneq ($(wildcard $(ARDUINO_APP)),) # */
#    s102 = $(ARDUINO_APP)/Contents/Resources/Java/lib/version.txt
    s103 = $(ARDUINO_APP)/Contents/Java/lib/version.txt
#    ifneq ($(wildcard $(s102)),)
#        ARDUINO_RELEASE := $(shell cat $(s102) | sed -e "s/\.//g")
#    else
        ARDUINO_RELEASE := $(shell cat $(s103) | sed -e "s/\.//g")
#    endif
    ARDUINO_MAJOR := $(shell echo $(ARDUINO_RELEASE) | cut -d. -f 1-2)
else
    ARDUINO_RELEASE := 0
    ARDUINO_MAJOR   := 0
endif


# Paths list for other genuine IDEs
#
MICRODUINO_PATH = $(MICRODUINO_APP)/Contents/Java
MICRODUINO_AVR_BOARDS       = $(MICRODUINO_PATH)/hardware/Microduino/avr/boards.txt

TEENSY_PATH     = $(TEENSY_APP)/Contents/Java
TEENSY_BOARDS   = $(TEENSY_PATH)/hardware/teensy/avr/boards.txt

MAPLE_PATH      = $(MAPLE_APP)/Contents/Resources/Java
MAPLE_BOARDS    = $(MAPLE_PATH)/hardware/leaflabs/boards.txt
WIRING_PATH     = $(WIRING_APP)/Contents/Java
WIRING_BOARDS   = $(WIRING_PATH)/hardware/Wiring/boards.txt

# Paths list for IDE-less platforms
#
MBED_PATH       = $(MBED_APP)


# Miscellaneous
# ----------------------------------
# Variables
#
TARGET      := embeddedcomputing
USER_FLAG   := false

# Builds directory
#
OBJDIR  = Builds

# Function PARSE_BOARD data retrieval from boards.txt
# result = $(call PARSE_BOARD 'boardname','parameter')
#
PARSE_BOARD = $(shell if [ -f $(BOARDS_TXT) ]; then grep ^$(1).$(2)= $(BOARDS_TXT) | cut -d = -f 2-; fi; )

# Function PARSE_FILE data retrieval from specified file
# result = $(call PARSE_FILE 'boardname','parameter','filename')
#
PARSE_FILE = $(shell if [ -f $(3) ]; then grep ^$(1).$(2) $(3) | cut -d = -f 2-; fi; )



# Clean if new BOARD_TAG
# ----------------------------------
#
NEW_TAG := $(strip $(OBJDIR)/$(BOARD_TAG).board) #
OLD_TAG := $(strip $(wildcard $(OBJDIR)/*.board)) # */

ifneq ($(OLD_TAG),$(NEW_TAG))
    CHANGE_FLAG := 1
else
    CHANGE_FLAG := 0
endif


# Identification and switch
# ----------------------------------
# Look if BOARD_TAG is listed as a Arduino/Arduino board
# Look if BOARD_TAG is listed as a Arduino/arduino/avr board *1.5
# Look if BOARD_TAG is listed as a Arduino/arduino/sam board *1.5
# Look if BOARD_TAG is listed as a chipKIT/PIC32 board
# Look if BOARD_TAG is listed as a Wiring/Wiring board
# Look if BOARD_TAG is listed as a Energia/MPS430 board
# Look if BOARD_TAG is listed as a MapleIDE/LeafLabs board
# Look if BOARD_TAG is listed as a Teensy/Teensy board
# Look if BOARD_TAG is listed as a Microduino/Microduino board
# Look if BOARD_TAG is listed as a Digistump/Digistump board
# Look if BOARD_TAG is listed as a IntelGalileo/arduino/x86 board
# Look if BOARD_TAG is listed as a Adafruit/Arduino board
# Look if BOARD_TAG is listed as a LittleRobotFriends board
# Look if BOARD_TAG is listed as a mbed board
# Look if BOARD_TAG is listed as a RedBearLab/arduino/RBL_nRF51822 board
# Look if BOARD_TAG is listed as a Spark board
#
# Order matters!
#
ifneq ($(MAKECMDGOALS),boards)
    ifneq ($(MAKECMDGOALS),clean)

        # Arduino
        ifneq ($(call PARSE_FILE,$(BOARD_TAG),name,$(ARDUINO_180_AVR_BOARDS)),)
            MAKEFILE_NAME = ArduinoAVR_166

        else ifneq ($(call PARSE_FILE,$(BOARD_TAG),name,$(ARDUINO_180_PATH)/hardware/arduino/avr/boards.txt),)
            MAKEFILE_NAME = ArduinoAVR_165
        else ifneq ($(call PARSE_FILE,$(BOARD_TAG1),name,$(ARDUINO_180_PATH)/hardware/arduino/avr/boards.txt),)
            MAKEFILE_NAME = ArduinoAVR_165

        else ifneq ($(call PARSE_FILE,$(BOARD_TAG),name,$(ARDUINO_180_SAM_BOARDS)),)
            MAKEFILE_NAME = ArduinoSAM_165
        else ifneq ($(call PARSE_FILE,$(BOARD_TAG),name,$(ARDUINO_180_SAMD_BOARDS)),)
            MAKEFILE_NAME = ArduinoSAMD_180

        # Energia 18
        else ifneq ($(call PARSE_FILE,$(BOARD_TAG_18),name,$(ENERGIA_19_MSP430_BOARDS)),)
            MAKEFILE_NAME = EnergiaMSP430_19
        else ifneq ($(call PARSE_FILE,$(BOARD_TAG_18),name,$(ENERGIA_18_MSP430_BOARDS)),)
            MAKEFILE_NAME = EnergiaMSP430_18
        else ifneq ($(call PARSE_FILE,$(BOARD_TAG_18),name,$(ENERGIA_18_TIVAC_BOARDS)),)
            MAKEFILE_NAME = EnergiaTIVAC_18
        else ifneq ($(call PARSE_FILE,$(BOARD_TAG_18),name,$(ENERGIA_18_CC3200_BOARDS)),)
            MAKEFILE_NAME = EnergiaCC3200_18
        else ifneq ($(call PARSE_FILE,$(BOARD_TAG_18),name,$(ENERGIA_18_CC1310_EMT_BOARDS)),)
            MAKEFILE_NAME = EnergiaCC1300EMT_18
        else ifneq ($(call PARSE_FILE,$(BOARD_TAG_18),name,$(ENERGIA_18_MSP432_EMT_BOARDS)),)
            MAKEFILE_NAME = EnergiaMSP432EMT_18
        else ifneq ($(call PARSE_FILE,$(BOARD_TAG_18),name,$(ENERGIA_18_CC3200_EMT_BOARDS)),)
            MAKEFILE_NAME = EnergiaCC3200EMT_18

        # Others boards for Arduino 1.8.0
        # ESP8266
        else ifneq ($(call PARSE_FILE,$(BOARD_TAG),name,$(ESP8266_BOARDS)),)
            MAKEFILE_NAME = ESP8266_165

        # Other boards
        else ifneq ($(call PARSE_FILE,$(BOARD_TAG),name,$(TEENSY_BOARDS)),)
            MAKEFILE_NAME = Teensy
        else ifneq ($(call PARSE_FILE,$(BOARD_TAG),name,$(GLOWDECK_BOARDS)),)
            MAKEFILE_NAME = Teensy

        # Other frameworks
        else
            UNKNOWN_BOARD = 1
#            $(error $(BOARD_TAG) board is unknown)
        endif
    endif
endif

# Information on makefile
#
include $(MAKEFILE_PATH)/$(MAKEFILE_NAME).mk
$(eval MAKEFILE_RELEASE = $(shell grep $(MAKEFILE_PATH)/$(MAKEFILE_NAME).mk -e '^# Last update' | rev | cut -d\  -f1-2 | rev ))

# List of sub-paths to be excluded
#
EXCLUDE_NAMES  = Example example Examples examples Archive archive Archives archives Documentation documentation Reference reference
EXCLUDE_NAMES += ArduinoTestSuite tests test
EXCLUDE_NAMES += $(EXCLUDE_LIBS)
EXCLUDE_LIST   = $(addprefix %,$(EXCLUDE_NAMES))

# Step 2
#
include $(MAKEFILE_PATH)/Step2.mk

