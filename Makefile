# ============================================================
# Makefile for UEF Driver Tests - WINDOWS VERSION
# ============================================================

# Toolchain
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# Project directories
SRC_DIR = App
BUILD_DIR = Build
UEF_DIR = UEF
STARTUP_DIR = UEF/Ports/STM32/STM32F411RE/vendor
LD_DIR = UEF/Ports/STM32/STM32F411RE/vendor

# ============================================================
# Select which test to build
# ============================================================
# Uncomment ONE of these to build a specific test

# TEST = test_gpio
# TEST = test_uart
 TEST = test_spi
# TEST = test_i2c
# TEST = test_adc
# TEST = test_gpio_interrupt
# TEST = test_ALL
# TEST = debug_uart_pins
# TEST = main

# ============================================================
# Source files
# ============================================================

# SRCS = $(SRC_DIR)/$(TEST).c

# ===========================================================
# For Examples inside APP Dir (comment out if executing main.c)
# ===========================================================

 SRCS += $(SRC_DIR)/Examples/$(TEST).c

# UEF Drivers
SRCS += $(UEF_DIR)/UEF_Drivers/UEF_GPIO/UEF_gpio.c
SRCS += $(UEF_DIR)/UEF_Drivers/UEF_UART/UEF_uart.c
SRCS += $(UEF_DIR)/UEF_Drivers/UEF_SPI/UEF_spi.c
SRCS += $(UEF_DIR)/UEF_Drivers/UEF_I2C/UEF_i2c.c
SRCS += $(UEF_DIR)/UEF_Drivers/UEF_ADC/UEF_adc.c
SRCS += $(UEF_DIR)/UEF_Drivers/UEF_CLOCK/UEF_clock.c
SRCS += $(UEF_DIR)/UEF_Drivers/UEF_TIMER/UEF_timer.c

# UEF Services
SRCS += $(UEF_DIR)/UEF_Services/RingBuffer/UEF_ringbuffer.c
SRCS += $(UEF_DIR)/UEF_Services/Logger/UEF_logger.c

# Startup and syscalls
SRCS += $(VENDOR_DIR)/STM32/STM32F411RE/vendor/startup_stm32f411xe.s
SRCS += $(STARTUP_DIR)/syscalls.c

# ============================================================
# Include directories
# ============================================================

INC = -I$(UEF_DIR)/UEF_Drivers/UEF_GPIO
INC += -I$(UEF_DIR)/UEF_Drivers/UEF_UART
INC += -I$(UEF_DIR)/UEF_Drivers/UEF_SPI
INC += -I$(UEF_DIR)/UEF_Drivers/UEF_I2C
INC += -I$(UEF_DIR)/UEF_Drivers/UEF_ADC
INC += -I$(UEF_DIR)/UEF_Drivers/UEF_CLOCK
INC += -I$(UEF_DIR)/UEF_Drivers/UEF_TIMER

INC += -I$(UEF_DIR)/UEF_Services/RingBuffer
INC += -I$(UEF_DIR)/UEF_Services/Logger

INC += -I$(UEF_DIR)/Ports/STM32/STM32F411RE/ChipLink
INC += -I$(UEF_DIR)/Ports/STM32/STM32F411RE

# ============================================================
# MCU specific
# ============================================================

MCU = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS = -Wall -Wextra -O0 -g $(MCU) $(INC) -std=c11 -ffunction-sections -fdata-sections

# Change the .ld file if using different MCUs

# Linker script
LDSCRIPT = $(LD_DIR)/stm32f411.ld
LDFLAGS = -T $(LDSCRIPT) $(MCU) -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/firmware.map -Wl,--no-warn-rwx-segments

# ============================================================
# Object files
# ============================================================

OBJS = $(addprefix $(BUILD_DIR)/, $(notdir $(SRCS:.c=.o)))
OBJS := $(OBJS:.s=.o)

# ============================================================
# VPATH (Source Code Search Path)
# ============================================================
# WHY WE NEED THIS: 
# Our object files are built inside the flat 'Build/' directory, 
# which strips away the original folder paths of our .c files.
#
# VPATH acts like a GPS for 'make'. When it tries to compile 
# an object file (like Build/uef_gpio.o) and looks for the 
# matching source (uef_gpio.c), VPATH tells it exactly which 
# subdirectories to search through to find that source file.
VPATH = $(SRC_DIR) \
        $(SRC_DIR)/Examples \
        $(UEF_DIR)/UEF_Drivers/UEF_GPIO \
        $(UEF_DIR)/UEF_Drivers/UEF_UART \
        $(UEF_DIR)/UEF_Drivers/UEF_SPI \
        $(UEF_DIR)/UEF_Drivers/UEF_I2C \
        $(UEF_DIR)/UEF_Drivers/UEF_ADC \
        $(UEF_DIR)/UEF_Drivers/UEF_CLOCK \
        $(UEF_DIR)/UEF_Drivers/UEF_TIMER \
        $(UEF_DIR)/UEF_Drivers/UEF_INTERRUPT \
        $(UEF_DIR)/UEF_Services/RingBuffer \
        $(UEF_DIR)/UEF_Services/Logger \
        $(STARTUP_DIR)

# ============================================================
# Targets
# ============================================================

all: $(BUILD_DIR)/firmware.bin

$(BUILD_DIR):
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/firmware.elf: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@
	$(SIZE) $@

$(BUILD_DIR)/firmware.bin: $(BUILD_DIR)/firmware.elf
	$(OBJCOPY) -O binary $< $@
	@echo ========================================
	@echo ✅ Firmware built: $(BUILD_DIR)/firmware.bin
	@echo Test: $(TEST)
	@echo ========================================

clean:
	if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)

# ============================================================
# Flash targets
# ============================================================

flash: $(BUILD_DIR)/firmware.bin
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(BUILD_DIR)/firmware.bin 0x08000000 verify reset exit"

program: all flash

help:
	@echo ========================================
	@echo UEF Test System
	@echo ========================================
	@echo Available tests:
	@echo   test_gpio          - GPIO LED and button test
	@echo   test_uart          - UART echo and command test
	@echo   test_spi           - SPI loopback test
	@echo   test_i2c           - I2C scanner test
	@echo   test_adc           - ADC reading test
	@echo   test_gpio_interrupt - GPIO interrupt test
	@echo   test_all           - All drivers test
	@echo ========================================
	@echo make all            - Build firmware
	@echo make flash          - Flash firmware
	@echo make program        - Build and flash
	@echo make clean          - Clean build files
	@echo make help           - Show this help
	@echo ========================================

.PHONY: all clean flash program help