#==========================================================================
#
# Copyright (C) 2020 Oleksii Ivashchenko according to example from Peter Shevchenko <developer@cxemotexnika.org>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#==========================================================================
.PHONY:	test project all clean

#--------------------------------------------------------------------------
#	start user settings 
#--------------------------------------------------------------------------
VERSION					:= Debug
#VERSION				:= Release

PRJ_NAME					:= K45

SRC						:= main.c
SRC						+= DEV_Config.c
SRC						+= bcm2835.c
SRC						+= pigpio.c
SRC						+= command.c
SRC						+= commonUnit.c
SRC						+= Executive/SPI1.c
SRC						+= Executive/modulSPI.c
SRC						+= Executive/nativePWM.c
SRC						+= Executive/additionalMeasurements.c
SRC						+= Keyboard_Indicator/lcd_variables.c
SRC						+= Keyboard_Indicator/Keypad.c
SRC						+= Keyboard_Indicator/Indicator_Variables.c
SRC						+= i2c_lcd/i2c_lcd.c
SRC						+= i2c_lcd/lcd.c
SRC						+= ADC/ADC_Unit.c
SRC						+= ADC/ADS1256.c
SRC						+= ThermoControlLibrary/Interpolations.c
SRC						+= ThermoControlLibrary/PIDCalculs.c
SRC						+= ThermoControlLibrary/Interface.c
SRC						+= ThermoControlLibrary/ThermoControlVariables.c
SRC						+= UART/uart.c
SRC						+= diskIO.c
SRC						+= globalvardefs.c


#VPATH					:= 
#INC 						:= 'f:/WORK/Infis/K45/Eclipse/Library_bcm2835-1.68/src'
INC 						+= 'C:/SysGCC/raspberry/arm-linux-gnueabihf/sysroot/usr/include'

DEFINE					:= 
LIB						:= stdc++

OPTIMIZE_LEVEL			:= 2
DEBUG_LEVEL				:= gdb

TOOL_PATH				:= 'C:/SysGCC/raspberry/arm-linux-gnueabihf'
#SHELL						:= 'C:/MinGW/msys/1.0/bin/sh.exe'
#--------------------------------------------------------------------------
#	end user settings
#--------------------------------------------------------------------------

ROOT					:= $(shell pwd)

CROSS_PREFIX		:= arm-linux-gnueabihf-
CC						:= $(CROSS_PREFIX)gcc
AS						:= $(CROSS_PREFIX)gcc
LD						:= $(CROSS_PREFIX)gcc
CPPC					:= $(CROSS_PREFIX)g++
AR						:= $(CROSS_PREFIX)ar
RANLIB				:= $(CROSS_PREFIX)ranlib
SIZE					:= $(CROSS_PREFIX)size
OBJCOPY				:= $(CROSS_PREFIX)objcopy
OBJDUMP				:= $(CROSS_PREFIX)objdump

BIN_DIR					:= bin
DEBUG_DIR				:= Debug
RELEASE_DIR				:= Release
INC_DIR					:= inc
OBJ_DIR					:= obj
SRC_DIR					:= src
# ---------------------------------------------------------------------------------
SRC_KEYIND_DIR			:= Keyboard_Indicator
I2C_LCD					:= i2c_lcd
SRC_EXECUTIVE_DIR		:= Executive
SRC_ADC_DIR				:= ADC
SRC_TEMPERCONTR_DIR	:= ThermoControlLibrary

#VPATH						+= $(INC_DIR) $(OBJ_DIR) $(SRC_DIR)
INCLUDE_PATH			:= $(INC) $(INC_DIR) $(COMM_INC_DIR) $(TOOL_PATH)/include
#INCLUDE_PATH			+='C:/SysGCC/raspberry/arm-linux-gnueabihf/sysroot/usr/include'
LIB_PATH					:= $(TOOL_PATH)/lib


LIBFLAGS					:= $(addprefix -L,$(LIB_PATH)) $(addprefix -l,$(LIB))	
ARFLAGS					:= rcs
 
CCFLAGS					:= -Wall
CCFLAGS					+= $(addprefix -D,$(DEFINE)) $(addprefix -I,$(INCLUDE_PATH))
CCFLAGS					+= -ffunction-sections -fdata-sections
CCFLAGS					+= -pthread
 
ASFLAGS					:= -Wall
ASFLAGS					+= $(addprefix -D,$(DEFINE)) $(addprefix -I,$(INCLUDE_PATH))

ifeq ($(VERSION),Debug)
IMAGE					:= $(BIN_DIR)/$(DEBUG_DIR)/$(PRJ_NAME)
CCFLAGS					+= -g$(DEBUG_LEVEL) -O0 -DDEBUG
ASFLAGS					+= -g$(DEBUG_LEVEL) -O0 -DDEBUG
endif

ifeq ($(VERSION),Release)
IMAGE					:= $(BIN_DIR)/$(RELEASE_DIR)/$(PRJ_NAME)
CCFLAGS					+= -O$(OPTIMIZE_LEVEL)
ASFLAGS					+= -O0
endif

CPPCFLAGS				:= $(CCFLAGS)
CPPCFLAGS				+= -x c++ -Weffc++
CPPCFLAGS				+= -std=gnu++0x
CPPCFLAGS				+= -pthread

LDFLAGS					:= -Wl,-Map,$(IMAGE).map,--cref -Wl,--gc-sections

OBJECTS					:= $(addprefix $(OBJ_DIR)/$(VERSION)/,$(patsubst %.c, %.o,$(filter %.c,$(SRC))))
OBJECTS					+= $(addprefix $(OBJ_DIR)/$(VERSION)/,$(patsubst %.cpp, %.o,$(filter %.cpp,$(SRC))))
OBJECTS					+= $(addprefix $(OBJ_DIR)/$(VERSION)/,$(patsubst %.s, %.o,$(filter %.s,$(SRC))))
OBJECTS					+= $(addprefix $(OBJ_DIR)/$(VERSION)/,$(patsubst %.S, %.o,$(filter %.S,$(SRC))))

#--------------------------------------------------------------------------
#	targets
#--------------------------------------------------------------------------

test: 
	$(CC) --version
	$(MAKE) --version
	@echo $(OBJECTS)
		
project:
	test  -d $(BIN_DIR) || mkdir $(BIN_DIR) $(BIN_DIR)/$(DEBUG_DIR) $(BIN_DIR)/$(RELEASE_DIR)
	test  -d $(OBJ_DIR) || mkdir $(OBJ_DIR) $(OBJ_DIR)/$(DEBUG_DIR) $(OBJ_DIR)/$(RELEASE_DIR)
	test  -d $(INC_DIR) || mkdir $(INC_DIR)
	test  -d $(SRC_DIR) || mkdir $(SRC_DIR) $(SRC_DIR)/$(SRC_KEYIND_DIR) $(SRC_DIR)/$(I2C_LCD) $(SRC_DIR)/$(SRC_KEYIND_DIR) $(SRC_DIR)/$(I2C_LCD) $(SRC_DIR)/$(SRC_EXECUTIVE_DIR) $(SRC_DIR)/$(SRC_ADC_DIR) $(SRC_DIR)/$(SRC_TEMPERCONTR_DIR)

all: $(VERSION)
	
$(VERSION): exe lst size
	@echo "--------------------- COMPLETE -----------------------"

exe:$(IMAGE)

lst:$(IMAGE).lst

size:$(IMAGE)
	@echo
	@echo $@
	@echo "------------------------------------------------------"
	$(SIZE) $(IMAGE)

$(IMAGE).lst:$(IMAGE)
	@echo
	@echo $@
	@echo "------------------------------------------------------"
	$(OBJDUMP) -h -S -z $<  > $@

$(IMAGE):$(OBJECTS)
	@echo
	@echo $@
	@echo "------------------------------------------------------"
	$(LD) $(CCFLAGS) $(LDFLAGS) $^ -o $@ $(LIBFLAGS)

$(OBJ_DIR)/$(VERSION)/%.o:$(SRC_DIR)/%.c
	@echo
	@echo $<
	@echo "------------------------------------------------------"
	$(CC) $(CCFLAGS) -MD -c $< -o $@

$(OBJ_DIR)/$(VERSION)/%.o:$(SRC_DIR)/%.cpp
	@echo
	@echo $<
	@echo "------------------------------------------------------"
	$(CPPC) $(CPPCFLAGS) -MD -c $< -o $@

$(OBJ_DIR)/$(VERSION)/%.o:$(SRC_DIR)/%.s
	@echo
	@echo $<
	@echo "------------------------------------------------------"
	$(AS) $(ASFLAGS) -c $< -o $@
	
$(OBJ_DIR)/$(VERSION)/%.o:$(SRC_DIR)/%.S
	@echo
	@echo $<
	@echo "------------------------------------------------------"
	$(AS) $(ASFLAGS) -c $< -o $@

include $(wildcard $(OBJ_DIR)/$(VERSION)/*.d)

clean:
	rm -f $(OBJECTS)
	rm -f $(patsubst %.o, %.d,$(OBJECTS))
	rm -f $(IMAGE) $(IMAGE).map $(IMAGE).lst
	@echo "--------------------- COMPLETE -----------------------"
	
#==========================================================================
#	End Of File
#==========================================================================