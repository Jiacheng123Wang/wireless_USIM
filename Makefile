# if softdevice stack is used, 1: used, 0: not used
IF_SOFTDEIVE_USED := 1

# if LIB file used, 1: used, 0: not used
IF_USE_LIBS := 0

OUTPUT_DIRECTORY := _build
SDK_ROOT := ../../..
PROJ_DIR := .

ifeq ($(IF_SOFTDEIVE_USED), 1)
  TARGETS        := ble_wirelessUSIM_nrf52832_xxaa
  $(OUTPUT_DIRECTORY)/$(TARGETS).out: \
  LINKER_SCRIPT  := $(PROJ_DIR)/wireless_uicc_terminal/ble_wireless_usim_gcc_nrf52.ld
else
  TARGETS        := blank_wirelessUSIM_nrf52832_xxaa
  $(OUTPUT_DIRECTORY)/$(TARGETS).out: \
	LINKER_SCRIPT  := $(PROJ_DIR)/wireless_uicc_terminal/blank_wireless_usim_gcc_nrf52.ld
endif

# Source files common to all targets
SRC_FILES += \
  $(PROJ_DIR)/main.c \
  $(SDK_ROOT)/components/toolchain/gcc/gcc_startup_nrf52.S \
  $(SDK_ROOT)/components/toolchain/system_nrf52.c \
  $(SDK_ROOT)/components/libraries/util/app_util_platform.c \
  $(SDK_ROOT)/components/drivers_nrf/ble_flash/ble_flash.c \
  $(SDK_ROOT)/components/drivers_nrf/ppi/nrf_drv_ppi.c \

# Source files for	IF_SOFTDEIVE_USED = 1
ifeq ($(IF_SOFTDEIVE_USED), 1)	
SRC_FILES += \
  $(SDK_ROOT)/components/drivers_nrf/timer/nrf_drv_timer.c \
  $(SDK_ROOT)/components/drivers_nrf/clock/nrf_drv_clock.c \
  $(SDK_ROOT)/components/drivers_nrf/common/nrf_drv_common.c \
  $(SDK_ROOT)/components/libraries/atomic_fifo/nrf_atfifo.c \
  $(SDK_ROOT)/components/libraries/util/sdk_mapped_flags.c \
  $(SDK_ROOT)/components/libraries/util/nrf_assert.c \
  $(SDK_ROOT)/components/libraries/experimental_section_vars/nrf_section_iter.c \
  $(SDK_ROOT)/components/libraries/strerror/nrf_strerror.c \
  $(SDK_ROOT)/components/libraries/util/app_error.c \
  $(SDK_ROOT)/components/libraries/util/app_error_weak.c \
  $(SDK_ROOT)/components/libraries/crc16/crc16.c \
  $(SDK_ROOT)/components/libraries/hardfault/hardfault_implementation.c \
  $(SDK_ROOT)/components/libraries/fstorage/nrf_fstorage.c \
  $(SDK_ROOT)/components/libraries/fstorage/nrf_fstorage_sd.c \
  $(SDK_ROOT)/components/libraries/pwr_mgmt/nrf_pwr_mgmt.c \
  $(SDK_ROOT)/components/libraries/timer/app_timer.c \
  $(SDK_ROOT)/components/libraries/scheduler/app_scheduler.c \
  $(SDK_ROOT)/components/libraries/fds/fds.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh_ble.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh_soc.c \
  $(SDK_ROOT)/components/ble/ble_services/ble_ancs_c/ancs_app_attr_get.c \
  $(SDK_ROOT)/components/ble/ble_services/ble_ancs_c/ancs_attr_parser.c \
  $(SDK_ROOT)/components/ble/ble_services/ble_ancs_c/ancs_tx_buffer.c \
  $(SDK_ROOT)/components/ble/ble_services/ble_ancs_c/nrf_ble_ancs_c.c \
  $(SDK_ROOT)/components/ble/ble_services/ble_nus/ble_nus.c \
  $(SDK_ROOT)/components/ble/common/ble_advdata.c \
  $(SDK_ROOT)/components/ble/ble_advertising/ble_advertising.c \
  $(SDK_ROOT)/components/ble/common/ble_conn_params.c \
  $(SDK_ROOT)/components/ble/common/ble_conn_state.c \
  $(SDK_ROOT)/components/ble/ble_db_discovery/ble_db_discovery.c \
  $(SDK_ROOT)/components/ble/common/ble_srv_common.c \
  $(SDK_ROOT)/components/ble/peer_manager/gatt_cache_manager.c \
  $(SDK_ROOT)/components/ble/peer_manager/gatts_cache_manager.c \
  $(SDK_ROOT)/components/ble/peer_manager/id_manager.c \
  $(SDK_ROOT)/components/ble/nrf_ble_gatt/nrf_ble_gatt.c \
  $(SDK_ROOT)/components/ble/peer_manager/peer_data_storage.c \
  $(SDK_ROOT)/components/ble/peer_manager/peer_database.c \
  $(SDK_ROOT)/components/ble/peer_manager/peer_id.c \
  $(SDK_ROOT)/components/ble/peer_manager/peer_manager.c \
  $(SDK_ROOT)/components/ble/peer_manager/pm_buffer.c \
  $(SDK_ROOT)/components/ble/peer_manager/pm_mutex.c \
  $(SDK_ROOT)/components/ble/peer_manager/security_dispatcher.c \
  $(SDK_ROOT)/components/ble/peer_manager/security_manager.c \
  $(PROJ_DIR)/ble/sd_ble_ancs_nus.c 
endif

# Source files for wireless-SIM
ifeq ($(IF_USE_LIBS), 0)
SRC_FILES += \
  $(PROJ_DIR)/wireless_uicc_terminal/src/uart_printf.c \
  $(PROJ_DIR)/wireless_uicc_terminal/src/flash_data_write.c \
  $(PROJ_DIR)/wireless_uicc_terminal/src/time_slot.c \
  $(PROJ_DIR)/wireless_uicc_terminal/src/usim_server_sim_interface.c \
  $(PROJ_DIR)/wireless_uicc_terminal/src/wireless_sim_phone.c \
  $(PROJ_DIR)/wireless_uicc_terminal/src/comm_uicc_terminal_interface.c \
  $(PROJ_DIR)/wireless_uicc_terminal/src/comm_phone_command.c \
  $(PROJ_DIR)/wireless_uicc_terminal/src/comm_wireless_interface.c \
  $(PROJ_DIR)/wireless_uicc_terminal/src/usim_server_command_sim.c \
  $(PROJ_DIR)/wireless_uicc_terminal/src/comm_initial.c \
  $(PROJ_DIR)/wireless_uicc_terminal/src/phone_wireless_interface.c \
  $(PROJ_DIR)/wireless_uicc_terminal/src/phone_command_sim.c \
  $(PROJ_DIR)/wireless_uicc_terminal/src/usim_server_wireless_interface.c
endif

# Include folders common to all targets
INC_FOLDERS += \
  $(PROJ_DIR)/config \
  $(PROJ_DIR)/ble  \
  $(SDK_ROOT)/components/ble/ble_services/ble_ancs_c \
  $(SDK_ROOT)/components/softdevice/s132/headers/nrf52 \
  $(SDK_ROOT)/components/libraries/fstorage \
  $(SDK_ROOT)/components/libraries/mutex \
  $(SDK_ROOT)/components/libraries/experimental_log/src \
  $(SDK_ROOT)/components/libraries/experimental_memobj \
  $(SDK_ROOT)/components/drivers_nrf/common \
  $(SDK_ROOT)/components/ble/ble_advertising \
  $(SDK_ROOT)/components/toolchain/cmsis/include \
  $(SDK_ROOT)/components/ble/common \
  $(SDK_ROOT)/components/ble/ble_db_discovery \
  $(SDK_ROOT)/components/libraries/experimental_section_vars \
  $(SDK_ROOT)/components/softdevice/s132/headers \
  $(SDK_ROOT)/components/ble/ble_services/ble_ans_c \
  $(SDK_ROOT)/components/libraries/mem_manager \
  $(SDK_ROOT)/components/drivers_nrf/hal \
  $(SDK_ROOT)/components/ble/ble_services/ble_nus_c \
  $(SDK_ROOT)/components/drivers_nrf/rtc \
  $(SDK_ROOT)/components/softdevice/common \
  $(SDK_ROOT)/components/libraries/ecc \
  $(SDK_ROOT)/components/drivers_nrf/ppi \
  $(SDK_ROOT)/components/ble/ble_services/ble_dfu \
  $(SDK_ROOT)/components/libraries/atomic \
  $(SDK_ROOT)/components \
  $(SDK_ROOT)/components/libraries/scheduler \
  $(SDK_ROOT)/components/libraries/experimental_log \
  $(SDK_ROOT)/components/drivers_nrf/delay \
  $(SDK_ROOT)/components/libraries/crc16 \
  $(SDK_ROOT)/components/drivers_nrf/timer \
  $(SDK_ROOT)/components/libraries/util \
  $(SDK_ROOT)/components/libraries/balloc \
  $(SDK_ROOT)/components/libraries/hardfault \
  $(SDK_ROOT)/components/libraries/timer \
  $(SDK_ROOT)/components/drivers_nrf/rng \
  $(SDK_ROOT)/components/drivers_nrf/power \
  $(SDK_ROOT)/components/toolchain \
  $(SDK_ROOT)/components/ble/ble_services/ble_nus \
  $(SDK_ROOT)/components/libraries/strerror \
  $(SDK_ROOT)/components/drivers_nrf/pdm \
  $(SDK_ROOT)/components/libraries/crc32 \
  $(SDK_ROOT)/components/ble/peer_manager \
  $(SDK_ROOT)/components/device \
  $(SDK_ROOT)/components/ble/nrf_ble_gatt \
  $(SDK_ROOT)/components/libraries/atomic_fifo \
  $(SDK_ROOT)/components/ble/ble_racp \
  $(SDK_ROOT)/components/toolchain/gcc \
  $(SDK_ROOT)/components/libraries/fds \
  $(SDK_ROOT)/components/libraries/twi \
  $(SDK_ROOT)/components/drivers_nrf/clock \
  $(SDK_ROOT)/components/drivers_nrf/ble_flash \

# ifeq ($(IF_USE_LIBS), 0)
INC_FOLDERS += \
  $(PROJ_DIR)/wireless_uicc_terminal/inc  
# endif
	
LIB_FILENAME := $(PROJ_DIR)/wireless_uicc_terminal/lib/$(TARGETS).a
# OBJ files
OBJECT_DIRECTORY = $(OUTPUT_DIRECTORY)/$(TARGETS)
OBJECT_FILES += \
  $(OBJECT_DIRECTORY)/uart_printf.c.o  \
  $(OBJECT_DIRECTORY)/flash_data_write.c.o \
  $(OBJECT_DIRECTORY)/time_slot.c.o \
  $(OBJECT_DIRECTORY)/usim_server_sim_interface.c.o \
  $(OBJECT_DIRECTORY)/wireless_sim_phone.c.o \
  $(OBJECT_DIRECTORY)/comm_uicc_terminal_interface.c.o \
  $(OBJECT_DIRECTORY)/comm_phone_command.c.o \
  $(OBJECT_DIRECTORY)/comm_wireless_interface.c.o \
  $(OBJECT_DIRECTORY)/usim_server_command_sim.c.o \
  $(OBJECT_DIRECTORY)/comm_initial.c.o \
  $(OBJECT_DIRECTORY)/phone_wireless_interface.c.o \
  $(OBJECT_DIRECTORY)/phone_command_sim.c.o \
  $(OBJECT_DIRECTORY)/usim_server_wireless_interface.c.o \

# Libraries common to all targets
LIB_FILES += \

# Optimization flags
OPT = -O3 -g3

# C flags common to all targets
CFLAGS += $(OPT)
CFLAGS += -DNRF52
CFLAGS += -DNRF52832_XXAA
CFLAGS += -DNRF52_PAN_74
CFLAGS += -DNRF_SD_BLE_API_VERSION=5
CFLAGS += -DS132
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS +=  -Wall -Werror
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in a separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums 

# C++ flags common to all targets
CXXFLAGS += $(OPT)

# Assembler flags common to all targets
ASMFLAGS += -g3
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
ASMFLAGS += -DNRF52
ASMFLAGS += -DNRF52832_XXAA
ASMFLAGS += -DNRF52_PAN_74
ASMFLAGS += -DNRF_SD_BLE_API_VERSION=5
ASMFLAGS += -DS132
ASMFLAGS += -DSOFTDEVICE_PRESENT

# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -mthumb -mabi=aapcs -L $(TEMPLATE_PATH) -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs 

# if use lib
ifeq ($(IF_USE_LIBS), 1)
LIB_FILES += $(LIB_FILENAME)
endif

# Add standard libraries at the very end of the linker input, after all objects
# that may need symbols provided by these libraries.
LIB_FILES += -lc -lnosys -lm

.PHONY: default help

# Default target - first one defined
default: $(TARGETS)

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo $(OUTPUT_DIRECTORY)/$(TARGETS)

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc

include $(TEMPLATE_PATH)/Makefile.common

$(foreach target, $(TARGETS), $(call define_target, $(target)))

.PHONY: flash flash_softdevice erase

# Flash the program
flash: $(OUTPUT_DIRECTORY)/$(TARGETS).hex
	@echo Flashing: $<
	nrfjprog -f nrf52 --program $< --sectorerase
	nrfjprog -f nrf52 --reset

# Flash softdevice
flash_softdevice:
ifeq ($(IF_SOFTDEIVE_USED), 1)	
	@echo Flashing: s132_nrf52_5.0.0_softdevice.hex
	nrfjprog -f nrf52 --program $(SDK_ROOT)/components/softdevice/s132/hex/s132_nrf52_5.0.0_softdevice.hex --sectorerase
	nrfjprog -f nrf52 --reset
else
	@echo IF_SOFTDEIVE_USED = 0, softdevice is not flashed
endif

erase:
	nrfjprog -f nrf52 --eraseall
	
recover:
	nrfjprog --recover -f nrf52
	
archieve: 
	$(NO_ECHO) $(AR) $(LIB_FILENAME) $(OBJECT_FILES)
	
