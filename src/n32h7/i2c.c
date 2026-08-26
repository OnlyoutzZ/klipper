// I2C functions on n32h7
//
// Copyright (C) 2026  Nsing
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "board/misc.h" // timer_is_before
#include "command.h" // shutdown
#include "gpio.h" // i2c_setup
#include "internal.h" // GPIO
#include "sched.h" // sched_shutdown
#include "i2ccmds.h" // I2C_BUS_SUCCESS

// The N32H76x I2C peripheral is register-compatible with the STM32F0/F7/L4/H7
// I2C peripheral, but the vendor header (n32h7xx.h) renames the registers and
// only provides field masks, not the shift positions used below.
#define I2C_BUSTM_CKDIV_Pos   28
#define I2C_BUSTM_DSCL_Pos    20
#define I2C_BUSTM_DSDA_Pos    16
#define I2C_BUSTM_HSCL_Pos    8
#define I2C_BUSTM_LSCL_Pos    0
#define I2C_CTRL2_BYTECNT_Pos 16

struct i2c_info {
    I2C_Module *i2c;
    uint8_t scl_pin, sda_pin;
    uint8_t scl_function, sda_function;
};

// I2C1
DECL_ENUMERATION("i2c_bus", "i2c1_PG5_PG4", 0);
DECL_CONSTANT_STR("BUS_PINS_i2c1_PG5_PG4", "PG5,PG4");
DECL_ENUMERATION("i2c_bus", "i2c1_PD4_PD5", 1);
DECL_CONSTANT_STR("BUS_PINS_i2c1_PD4_PD5", "PD4,PD5");
DECL_ENUMERATION("i2c_bus", "i2c1_PB6_PB7", 2);
DECL_CONSTANT_STR("BUS_PINS_i2c1_PB6_PB7", "PB6,PB7");
// I2C2
DECL_ENUMERATION("i2c_bus", "i2c2_PF1_PF0", 3);
DECL_CONSTANT_STR("BUS_PINS_i2c2_PF1_PF0", "PF1,PF0");
DECL_ENUMERATION("i2c_bus", "i2c2_PH4_PH5", 4);
DECL_CONSTANT_STR("BUS_PINS_i2c2_PH4_PH5", "PH4,PH5");
DECL_ENUMERATION("i2c_bus", "i2c2_PB10_PB11", 5);
DECL_CONSTANT_STR("BUS_PINS_i2c2_PB10_PB11", "PB10,PB11");
// I2C3
DECL_ENUMERATION("i2c_bus", "i2c3_PH7_PH8", 6);
DECL_CONSTANT_STR("BUS_PINS_i2c3_PH7_PH8", "PH7,PH8");
DECL_ENUMERATION("i2c_bus", "i2c3_PK0_PK1", 7);
DECL_CONSTANT_STR("BUS_PINS_i2c3_PK0_PK1", "PK0,PK1");
DECL_ENUMERATION("i2c_bus", "i2c3_PA8_PC9", 8);
DECL_CONSTANT_STR("BUS_PINS_i2c3_PA8_PC9", "PA8,PC9");
// I2C4
DECL_ENUMERATION("i2c_bus", "i2c4_PF14_PF15", 9);
DECL_CONSTANT_STR("BUS_PINS_i2c4_PF14_PF15", "PF14,PF15");
DECL_ENUMERATION("i2c_bus", "i2c4_PH11_PH12", 10);
DECL_CONSTANT_STR("BUS_PINS_i2c4_PH11_PH12", "PH11,PH12");
DECL_ENUMERATION("i2c_bus", "i2c4_PD12_PD13", 11);
DECL_CONSTANT_STR("BUS_PINS_i2c4_PD12_PD13", "PD12,PD13");
// I2C5
DECL_ENUMERATION("i2c_bus", "i2c5_PF1_PF0", 12);
DECL_CONSTANT_STR("BUS_PINS_i2c5_PF1_PF0", "PF1,PF0");
DECL_ENUMERATION("i2c_bus", "i2c5_PA8_PC9", 13);
DECL_CONSTANT_STR("BUS_PINS_i2c5_PA8_PC9", "PA8,PC9");
DECL_ENUMERATION("i2c_bus", "i2c5_PC11_PC10", 14);
DECL_CONSTANT_STR("BUS_PINS_i2c5_PC11_PC10", "PC11,PC10");
// I2C6
DECL_ENUMERATION("i2c_bus", "i2c6_PF4_PF3", 15);
DECL_CONSTANT_STR("BUS_PINS_i2c6_PF4_PF3", "PF4,PF3");
DECL_ENUMERATION("i2c_bus", "i2c6_PF12_PF11", 16);
DECL_CONSTANT_STR("BUS_PINS_i2c6_PF12_PF11", "PF12,PF11");
DECL_ENUMERATION("i2c_bus", "i2c6_PE14_PE13", 17);
DECL_CONSTANT_STR("BUS_PINS_i2c6_PE14_PE13", "PE14,PE13");
// I2C7
DECL_ENUMERATION("i2c_bus", "i2c7_PI13_PI12", 18);
DECL_CONSTANT_STR("BUS_PINS_i2c7_PI13_PI12", "PI13,PI12");
DECL_ENUMERATION("i2c_bus", "i2c7_PE8_PE7", 19);
DECL_CONSTANT_STR("BUS_PINS_i2c7_PE8_PE7", "PE8,PE7");
DECL_ENUMERATION("i2c_bus", "i2c7_PD9_PD8", 20);
DECL_CONSTANT_STR("BUS_PINS_i2c7_PD9_PD8", "PD9,PD8");
// I2C8
DECL_ENUMERATION("i2c_bus", "i2c8_PD15_PD14", 21);
DECL_CONSTANT_STR("BUS_PINS_i2c8_PD15_PD14", "PD15,PD14");
DECL_ENUMERATION("i2c_bus", "i2c8_PJ9_PJ8", 22);
DECL_CONSTANT_STR("BUS_PINS_i2c8_PJ9_PJ8", "PJ9,PJ8");
DECL_ENUMERATION("i2c_bus", "i2c8_PG5_PG4", 23);
DECL_CONSTANT_STR("BUS_PINS_i2c8_PG5_PG4", "PG5,PG4");
// I2C9
DECL_ENUMERATION("i2c_bus", "i2c9_PI11_PI10", 24);
DECL_CONSTANT_STR("BUS_PINS_i2c9_PI11_PI10", "PI11,PI10");
DECL_ENUMERATION("i2c_bus", "i2c9_PJ1_PJ0", 25);
DECL_CONSTANT_STR("BUS_PINS_i2c9_PJ1_PJ0", "PJ1,PJ0");
DECL_ENUMERATION("i2c_bus", "i2c9_PJ4_PJ3", 26);
DECL_CONSTANT_STR("BUS_PINS_i2c9_PJ4_PJ3", "PJ4,PJ3");
// I2C10
DECL_ENUMERATION("i2c_bus", "i2c10_PI1_PI0", 27);
DECL_CONSTANT_STR("BUS_PINS_i2c10_PI1_PI0", "PI1,PI0");
DECL_ENUMERATION("i2c_bus", "i2c10_PJ14_PJ13", 28);
DECL_CONSTANT_STR("BUS_PINS_i2c10_PJ14_PJ13", "PJ14,PJ13");
DECL_ENUMERATION("i2c_bus", "i2c10_PK4_PK3", 29);
DECL_CONSTANT_STR("BUS_PINS_i2c10_PK4_PK3", "PK4,PK3");

static const struct i2c_info i2c_bus[] = {
    // I2C1
    { I2C1, GPIO('G', 5), GPIO('G', 4), GPIO_FUNCTION(10), GPIO_FUNCTION(9) },
    { I2C1, GPIO('D', 4), GPIO('D', 5), GPIO_FUNCTION(9), GPIO_FUNCTION(8) },
    { I2C1, GPIO('B', 6), GPIO('B', 7), GPIO_FUNCTION(11), GPIO_FUNCTION(10) },
    // I2C2
    { I2C2, GPIO('F', 1), GPIO('F', 0), GPIO_FUNCTION(11), GPIO_FUNCTION(10) },
    { I2C2, GPIO('H', 4), GPIO('H', 5), GPIO_FUNCTION(9), GPIO_FUNCTION(9) },
    { I2C2, GPIO('B', 10), GPIO('B', 11), GPIO_FUNCTION(10), GPIO_FUNCTION(7) },
    // I2C3
    { I2C3, GPIO('H', 7), GPIO('H', 8), GPIO_FUNCTION(9), GPIO_FUNCTION(11) },
    { I2C3, GPIO('K', 0), GPIO('K', 1), GPIO_FUNCTION(6), GPIO_FUNCTION(6) },
    { I2C3, GPIO('A', 8), GPIO('C', 9), GPIO_FUNCTION(9), GPIO_FUNCTION(12) },
    // I2C4
    { I2C4, GPIO('F', 14), GPIO('F', 15), GPIO_FUNCTION(8), GPIO_FUNCTION(9) },
    { I2C4, GPIO('H', 11), GPIO('H', 12), GPIO_FUNCTION(8), GPIO_FUNCTION(10) },
    { I2C4, GPIO('D', 12), GPIO('D', 13), GPIO_FUNCTION(10), GPIO_FUNCTION(8) },
    // I2C5
    { I2C5, GPIO('F', 1), GPIO('F', 0), GPIO_FUNCTION(12), GPIO_FUNCTION(11) },
    { I2C5, GPIO('A', 8), GPIO('C', 9), GPIO_FUNCTION(10), GPIO_FUNCTION(13) },
    { I2C5, GPIO('C', 11), GPIO('C', 10), GPIO_FUNCTION(9), GPIO_FUNCTION(9) },
    // I2C6
    { I2C6, GPIO('F', 4), GPIO('F', 3), GPIO_FUNCTION(12), GPIO_FUNCTION(11) },
    { I2C6, GPIO('F', 12), GPIO('F', 11), GPIO_FUNCTION(7), GPIO_FUNCTION(5) },
    { I2C6, GPIO('E', 14), GPIO('E', 13), GPIO_FUNCTION(10), GPIO_FUNCTION(8) },
    // I2C7
    { I2C7, GPIO('I', 13), GPIO('I', 12), GPIO_FUNCTION(7), GPIO_FUNCTION(7) },
    { I2C7, GPIO('E', 8), GPIO('E', 7), GPIO_FUNCTION(9), GPIO_FUNCTION(8) },
    { I2C7, GPIO('D', 9), GPIO('D', 8), GPIO_FUNCTION(6), GPIO_FUNCTION(7) },
    // I2C8
    { I2C8, GPIO('D', 15), GPIO('D', 14), GPIO_FUNCTION(9), GPIO_FUNCTION(9) },
    { I2C8, GPIO('J', 9), GPIO('J', 8), GPIO_FUNCTION(9), GPIO_FUNCTION(9) },
    { I2C8, GPIO('G', 5), GPIO('G', 4), GPIO_FUNCTION(11), GPIO_FUNCTION(10) },
    // I2C9
    { I2C9, GPIO('I', 11), GPIO('I', 10), GPIO_FUNCTION(7), GPIO_FUNCTION(9) },
    { I2C9, GPIO('J', 1), GPIO('J', 0), GPIO_FUNCTION(9), GPIO_FUNCTION(8) },
    { I2C9, GPIO('J', 4), GPIO('J', 3), GPIO_FUNCTION(8), GPIO_FUNCTION(9) },
    // I2C10
    { I2C10, GPIO('I', 1), GPIO('I', 0), GPIO_FUNCTION(11), GPIO_FUNCTION(8) },
    { I2C10, GPIO('J', 14), GPIO('J', 13), GPIO_FUNCTION(4), GPIO_FUNCTION(6) },
    { I2C10, GPIO('K', 4), GPIO('K', 3), GPIO_FUNCTION(5), GPIO_FUNCTION(5) },
};

struct i2c_config
i2c_setup(uint32_t bus, uint32_t rate, uint8_t addr)
{
    // Lookup requested i2c bus
    if (bus >= ARRAY_SIZE(i2c_bus))
        shutdown("Unsupported i2c bus");
    const struct i2c_info *ii = &i2c_bus[bus];
    I2C_Module *i2c = ii->i2c;

    if (!is_enabled_pclock((uint32_t)i2c)) {
        // Enable i2c clock and gpio
        enable_pclock((uint32_t)i2c);
        gpio_peripheral(ii->scl_pin, ii->scl_function | GPIO_OPEN_DRAIN, 1);
        gpio_peripheral(ii->sda_pin, ii->sda_function | GPIO_OPEN_DRAIN, 1);

        // Set 100Khz frequency and enable
        uint32_t nom_i2c_clock = 12000000; // 12mhz internal clock (83.3ns tick)
        uint32_t scll = 60; // 60 * 83.3ns = 5us
        uint32_t sclh = 48; // 48 * 83.3ns = 4us
        uint32_t sdadel = 6; // 6 * 83.3ns = 500ns
        uint32_t scldel = 15; // 15 * 83.3ns = 1250ns
        // Clamp the rate to 400Khz
        if (rate >= 400000) {
            scll = 15; // 15 * 83.3ns = 1250ns
            sclh = 6; // 6 * 83.3 = 500ns
            sdadel = 4; // 4 * 83.3 = 333ns
            scldel = 6; // 6 * 83.3 = 500ns
        }
        //I2C1～3
        uint32_t reg_tmp = RCC->APB1DIV1;
        reg_tmp &= (~RCC_APB1DIV1_APB1I2CDIV);
        reg_tmp |= (RCC_APB1DIV1_APB1I2CDIV_DIV2);// I2C clk = 300/2 = 150M
        RCC->APB1DIV1 = reg_tmp;
        //I2C4～6
        reg_tmp = RCC->APB2DIV1;
        reg_tmp &= (~RCC_APB2DIV1_APB2I2CDIV);
        reg_tmp |= (RCC_APB1DIV1_APB1I2CDIV_DIV2<<8);// I2C clk = 300/2 = 150M
        RCC->APB2DIV1 = reg_tmp;
        //I2C7～10
        reg_tmp = RCC->APB5DIV1;
        reg_tmp &= (~RCC_APB5DIV1_APB5I2CDIV);
        reg_tmp |= (RCC_APB1DIV1_APB1I2CDIV_DIV2<<24);// I2C clk = 300/2 = 150M
        RCC->APB5DIV1 = reg_tmp;

        uint32_t pclk = (CONFIG_CLOCK_FREQ / 4);//Get 600/2/2 = 150M
        uint32_t presc = DIV_ROUND_UP(pclk, nom_i2c_clock);
        i2c->BUSTM = (((presc - 1) << I2C_BUSTM_CKDIV_Pos)
                      | ((scll - 1) << I2C_BUSTM_LSCL_Pos)
                      | ((sclh - 1) << I2C_BUSTM_HSCL_Pos)
                      | (sdadel << I2C_BUSTM_DSDA_Pos)
                      | ((scldel - 1) << I2C_BUSTM_DSCL_Pos));
        i2c->CTRL1 = I2C_CTRL1_I2CEN;
    }

    return (struct i2c_config){ .i2c=i2c, .addr=addr<<1 };
}

static int
i2c_wait(I2C_Module *i2c, uint32_t set, uint32_t timeout)
{
    for (;;) {
        uint32_t isr = i2c->STSINT;
        if (isr & set)
            return I2C_BUS_SUCCESS;
        if (isr & I2C_STSINT_NAKF) {
            i2c->INTCLR = I2C_INTCLR_NAKCLR;
            return I2C_BUS_NACK;
        }
        if (!timer_is_before(timer_read_time(), timeout))
            return I2C_BUS_TIMEOUT;
    }
}

int
i2c_write(struct i2c_config config, uint8_t write_len, uint8_t *write)
{
    I2C_Module *i2c = config.i2c;
    uint32_t timeout = timer_read_time() + timer_from_us(5000);
    int ret = I2C_BUS_SUCCESS;
    uint8_t *write_orig = write;

    // Send start and address
    i2c->CTRL2 = (I2C_CTRL2_START | config.addr |
                  (write_len << I2C_CTRL2_BYTECNT_Pos) | I2C_CTRL2_AUTOSTOP);
    while (write_len--) {
        ret = i2c_wait(i2c, I2C_STSINT_WRAVL, timeout);
        if (ret != I2C_BUS_SUCCESS)
            goto abrt;
        i2c->WDR = *write++;
    }
    return i2c_wait(i2c, I2C_STSINT_WRE, timeout);
abrt:
    if (write == write_orig && ret == I2C_BUS_NACK)
        ret = I2C_BUS_START_NACK;
    i2c->CTRL2 |= I2C_CTRL2_STOP;
    return ret;
}

int
i2c_read(struct i2c_config config, uint8_t reg_len, uint8_t *reg
         , uint8_t read_len, uint8_t *read)
{
    I2C_Module *i2c = config.i2c;
    uint32_t timeout = timer_read_time() + timer_from_us(5000);
    int ret = I2C_BUS_SUCCESS;
    uint8_t *write_orig = reg;
    uint8_t *read_orig = read;

    if (reg_len) {
        // Send start, address, reg
        i2c->CTRL2 = (I2C_CTRL2_START | config.addr |
                     (reg_len << I2C_CTRL2_BYTECNT_Pos));
        while (reg_len--) {
            ret = i2c_wait(i2c, I2C_STSINT_WRAVL, timeout);
            if (ret != I2C_BUS_SUCCESS)
                goto abrt;
            i2c->WDR = *reg++;
        }
        i2c_wait(i2c, I2C_STSINT_TFC, timeout);
    }

    // send restart, read data
    i2c->CTRL2 = (I2C_CTRL2_START | I2C_CTRL2_RWN | config.addr |
                 (read_len << I2C_CTRL2_BYTECNT_Pos) | I2C_CTRL2_AUTOSTOP);
    while (read_len--) {
        ret = i2c_wait(i2c, I2C_STSINT_RDAVL, timeout);
        if (ret != I2C_BUS_SUCCESS)
            goto abrt_read;
        *read++ = i2c->RDR;
    }
    return i2c_wait(i2c, I2C_STSINT_STOPF, timeout);
abrt_read:
    if (read == read_orig && ret == I2C_BUS_NACK)
        ret = I2C_BUS_START_READ_NACK;
abrt:
    if (reg == write_orig && ret == I2C_BUS_NACK)
        ret = I2C_BUS_START_NACK;
    i2c->CTRL2 |= I2C_CTRL2_STOP;
    return ret;
}
