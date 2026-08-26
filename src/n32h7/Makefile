# N32H7 build rules
#
# Copyright (C) 2026  Nsing
#
# This file may be distributed under the terms of the GNU GPLv3 license.

# Setup the toolchain
CROSS_PREFIX=arm-none-eabi-
CPP=$(CROSS_PREFIX)gcc -E -x c

dirs-y += src/n32h7 src/generic lib/n32h7xx

MCU := $(shell echo $(CONFIG_MCU))
MCU_UPPER := $(shell echo $(CONFIG_MCU) | tr a-z A-Z | tr X x)

CFLAGS-$(CONFIG_MACH_N32H76X) += -mcpu=cortex-m7 -Ilib/n32h7xx/include

CFLAGS += $(CFLAGS-y) -D$(MCU_UPPER) -mthumb
CFLAGS += -mfpu=fpv5-d16 -mfloat-abi=hard
CFLAGS += -Isrc/n32h7 -Ilib/cmsis-core
CFLAGS += -DCORE_CM7 -DUSING_TCM
CFLAGS += -D__ICACHE_PRESENT=1 -D__DCACHE_PRESENT=1
CFLAGS += -DTCM_SIZE_VALUE=$(CONFIG_N32H7_TCM_SIZE_VALUE)
CFLAGS += -DN32H7_SYSTEM_CLOCK_HZ=$(CONFIG_CLOCK_FREQ)

CFLAGS_klipper.elf += -nostdlib -lgcc -lc_nano
CFLAGS_klipper.elf += -T $(OUT)src/n32h7/n32h76x_itcm_link.ld
# Generate a linker map file for debugging the Flash/ITCM/RAM layout
CFLAGS_klipper.elf += -Wl,-Map=$(OUT)klipper.map
$(OUT)klipper.elf: $(OUT)src/n32h7/n32h76x_itcm_link.ld

# Add source files
src-y += n32h7/boot_itcm.c
src-y += n32h7/clockline.c
src-y += n32h7/n32h76x.c
src-y += n32h7/gpio.c
src-$(CONFIG_WANT_HARD_PWM) += n32h7/hard_pwm.c
src-$(CONFIG_SERIAL) += n32h7/serial.c generic/serial_irq.c
src-$(CONFIG_WANT_I2C) += n32h7/i2c.c
src-$(CONFIG_WANT_SPI) += n32h7/spi.c

src-y += ../lib/n32h7xx/system_n32h7xx.c

src-y += generic/armcm_irq.c
src-y += generic/armcm_timer.c
src-y += generic/armcm_reset.c
src-y += generic/crc16_ccitt.c

# Build the binary output file
target-y += $(OUT)klipper.bin

$(OUT)klipper.bin: $(OUT)klipper.elf
	@echo "  Creating bin file $@"
	$(Q)$(OBJCOPY) -O binary $< $@
