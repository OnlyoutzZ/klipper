// SPI functions on n32h7
//
// Copyright (C) 2026  Nsing
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "board/io.h" // readb, writeb
#include "command.h" // shutdown
#include "gpio.h" // spi_setup
#include "internal.h" // gpio_peripheral, SPI_Module
#include "sched.h" // sched_shutdown

struct spi_info {
    SPI_Module *spi;
    uint8_t miso_pin, mosi_pin, sck_pin, miso_func, mosi_func, sck_func;
};

// The alternate function number differs per pin/peripheral on the N32H7
// (see the AFIO user manual "SPIx/I2Sx复用功能重映射" tables), so each
// pin stores its own AF number rather than assuming a shared value.
#define SPI_FUNCTION(miso_af, mosi_af, sck_af) GPIO_FUNCTION(miso_af), \
    GPIO_FUNCTION(mosi_af), GPIO_FUNCTION(sck_af)

DECL_ENUMERATION("spi_bus", "spi1_PA6_PA7_PA5", 0);
DECL_CONSTANT_STR("BUS_PINS_spi1_PA6_PA7_PA5", "PA6,PA7,PA5");
DECL_ENUMERATION("spi_bus", "spi2_PB14_PB15_PB13", 1);
DECL_CONSTANT_STR("BUS_PINS_spi2_PB14_PB15_PB13", "PB14,PB15,PB13");
DECL_ENUMERATION("spi_bus", "spi3_PC11_PC12_PC10", 2);
DECL_CONSTANT_STR("BUS_PINS_spi3_PC11_PC12_PC10", "PC11,PC12,PC10");
DECL_ENUMERATION("spi_bus", "spi4_PA6_PA7_PA5", 3);
DECL_CONSTANT_STR("BUS_PINS_spi4_PA6_PA7_PA5", "PA6,PA7,PA5");
DECL_ENUMERATION("spi_bus", "spi5_PF8_PF9_PF7", 4);
DECL_CONSTANT_STR("BUS_PINS_spi5_PF8_PF9_PF7", "PF8,PF9,PF7");
DECL_ENUMERATION("spi_bus", "spi6_PE13_PE14_PE12", 5);
DECL_CONSTANT_STR("BUS_PINS_spi6_PE13_PE14_PE12", "PE13,PE14,PE12");
DECL_ENUMERATION("spi_bus", "spi7_PI13_PI14_PI8", 6);
DECL_CONSTANT_STR("BUS_PINS_spi7_PI13_PI14_PI8", "PI13,PI14,PI8");

static const struct spi_info spi_bus[] = {
    { SPI1, GPIO('A', 6), GPIO('A', 7), GPIO('A', 5), SPI_FUNCTION(4, 5, 4) },
    { SPI2, GPIO('B', 14), GPIO('B', 15), GPIO('B', 13), SPI_FUNCTION(4, 4, 4) },
    { SPI3, GPIO('C', 11), GPIO('C', 12), GPIO('C', 10), SPI_FUNCTION(4, 5, 5) },
    { SPI4, GPIO('A', 6), GPIO('A', 7), GPIO('A', 5), SPI_FUNCTION(5, 6, 5) },
    { SPI5, GPIO('F', 8), GPIO('F', 9), GPIO('F', 7), SPI_FUNCTION(5, 5, 4) },
    { SPI6, GPIO('E', 13), GPIO('E', 14), GPIO('E', 12), SPI_FUNCTION(4, 5, 5) },
    { SPI7, GPIO('I', 13), GPIO('I', 14), GPIO('I', 8), SPI_FUNCTION(4, 4, 3) },
};

struct spi_config
spi_setup(uint32_t bus, uint8_t mode, uint32_t rate)
{
    if (bus >= ARRAY_SIZE(spi_bus))
        shutdown("Invalid spi bus");

    // Enable SPI
    SPI_Module *spi = spi_bus[bus].spi;
    if (!is_enabled_pclock((uint32_t)spi)) {
        enable_pclock((uint32_t)spi);
        gpio_peripheral(spi_bus[bus].miso_pin, spi_bus[bus].miso_func, 1);
        gpio_peripheral(spi_bus[bus].mosi_pin, spi_bus[bus].mosi_func, 0);
        gpio_peripheral(spi_bus[bus].sck_pin, spi_bus[bus].sck_func, 0);
        // 8-bit data frame (DATFF=0), MSB first (LSBFF=0), SPI mode (I2SMOD=0)
        spi->CTRL1 = 0;
        // Note: on the N32H7 the SPI enable bit lives in CTRL2, not CTRL1
        spi->CTRL2 = SPI_CTRL2_SPIEN;
    }

    // Calculate CTRL1 register
    uint32_t pclk = get_pclock_frequency((uint32_t)spi);
    uint32_t div = 0;
    while ((pclk >> (div + 1)) > rate && div < 7)
        div++;
    // Klipper mode is bit0=CPHA bit1=CPOL, but CLKPOL is bit4 and CLKPHA is
    // bit5 on the N32H7 (the reverse of the STM32 layout), so map explicitly.
    uint32_t ctrl1 = (div & SPI_CTRL1_BR)
        | (mode & 1 ? SPI_CTRL1_CLKPHA : 0)
        | (mode & 2 ? SPI_CTRL1_CLKPOL : 0)
        | SPI_CTRL1_MSEL | SPI_CTRL1_SSMEN | SPI_CTRL1_SSEL;

    return (struct spi_config){ .spi = spi, .spi_cr1 = ctrl1 };
}

void
spi_prepare(struct spi_config config)
{
    SPI_Module *spi = config.spi;
    uint32_t ctrl1 = spi->CTRL1;
    if (ctrl1 == config.spi_cr1)
        return;
    // The SPI must be disabled before changing the mode bits
    spi->CTRL2 &= ~SPI_CTRL2_SPIEN;
    spi->CTRL2; // Force flush of previous write
    spi->CTRL1 = config.spi_cr1;
    spi->CTRL2 |= SPI_CTRL2_SPIEN;
}

void
spi_transfer(struct spi_config config, uint8_t receive_data,
             uint8_t len, uint8_t *data)
{
    SPI_Module *spi = config.spi;
    uint8_t *end = data + len;
    while (data < end) {
        writeb((void*)&spi->DAT, *data);
        while (!(spi->STS & SPI_STS_RNE))
            ;
        uint8_t rdata = readb((void*)&spi->DAT);
        if (receive_data)
            *data = rdata;
        data++;
    }
    // Wait for any remaining SCLK updates before returning
    while ((spi->STS & (SPI_STS_TE | SPI_STS_BUSY)) != SPI_STS_TE)
        ;
}
