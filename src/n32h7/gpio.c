// GPIO functions on n32h7
//
// Copyright (C) 2026  Nsing
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "board/irq.h" // irq_save
#include "command.h" // DECL_ENUMERATION_RANGE
#include "gpio.h" // gpio_out_setup
#include "internal.h" // gpio_peripheral
#include "sched.h" // shutdown

DECL_ENUMERATION_RANGE("pin", "PA0", GPIO('A', 0), 16);
DECL_ENUMERATION_RANGE("pin", "PB0", GPIO('B', 0), 16);
DECL_ENUMERATION_RANGE("pin", "PC0", GPIO('C', 0), 16);
DECL_ENUMERATION_RANGE("pin", "PD0", GPIO('D', 0), 16);
DECL_ENUMERATION_RANGE("pin", "PE0", GPIO('E', 0), 16);
DECL_ENUMERATION_RANGE("pin", "PF0", GPIO('F', 0), 16);
DECL_ENUMERATION_RANGE("pin", "PG0", GPIO('G', 0), 16);
DECL_ENUMERATION_RANGE("pin", "PH0", GPIO('H', 0), 16);
DECL_ENUMERATION_RANGE("pin", "PI0", GPIO('I', 0), 16);
DECL_ENUMERATION_RANGE("pin", "PJ0", GPIO('J', 0), 16);
DECL_ENUMERATION_RANGE("pin", "PK0", GPIO('K', 0), 8); // port K has 8 pins

static GPIO_Module * const digital_regs[] = {
    GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF,
    GPIOG, GPIOH, GPIOI, GPIOJ, GPIOK
};

static const struct gpio_clock {
    volatile uint32_t *en;
    uint32_t bit;
} gpio_clocks[] = {
    { &RCC->AHB5EN1, RCC_AHB5EN1_M7GPIOAEN },
    { &RCC->AHB5EN1, RCC_AHB5EN1_M7GPIOBEN },
    { &RCC->AHB5EN1, RCC_AHB5EN1_M7GPIOCEN },
    { &RCC->AHB5EN1, RCC_AHB5EN1_M7GPIODEN },
    { &RCC->AHB5EN1, RCC_AHB5EN1_M7GPIOEEN },
    { &RCC->AHB5EN1, RCC_AHB5EN1_M7GPIOFEN },
    { &RCC->AHB5EN1, RCC_AHB5EN1_M7GPIOGEN },
    { &RCC->AHB5EN1, RCC_AHB5EN1_M7GPIOHEN },
    { &RCC->AHB5EN2, RCC_AHB5EN2_M7GPIOIEN },
    { &RCC->AHB5EN2, RCC_AHB5EN2_M7GPIOJEN },
    { &RCC->AHB5EN2, RCC_AHB5EN2_M7GPIOKEN },
};

// Convert a register and bit location back to an integer pin identifier
static int
regs_to_pin(GPIO_Module *regs, uint32_t bit)
{
    for (int i = 0; i < (int)ARRAY_SIZE(digital_regs); i++)
        if (digital_regs[i] == regs)
            return GPIO('A' + i, __builtin_ctz(bit));
    return 0;
}

// Verify that a gpio is a valid pin and return its hardware register
GPIO_Module *
gpio_pin_to_regs(uint32_t pin)
{
    uint32_t port = GPIO2PORT(pin);
    if (port >= ARRAY_SIZE(digital_regs) || pin >= GPIO('K', 8))
        shutdown("Not a valid pin");
    return digital_regs[port];
}

void
gpio_clock_enable(GPIO_Module *regs)
{
    for (int i = 0; i < (int)ARRAY_SIZE(digital_regs); i++) {
        if (digital_regs[i] == regs) {
            *gpio_clocks[i].en |= gpio_clocks[i].bit;
            *gpio_clocks[i].en; // flush register write
            return;
        }
    }
}


// AFIO 模拟信号通道使能寄存器 (ANAEN_CFG0..6, 见 gpio.md)
static volatile uint32_t * const AN_regs[] = {
    &AFIO->ANAEN_CFG0, &AFIO->ANAEN_CFG1, &AFIO->ANAEN_CFG2,
    &AFIO->ANAEN_CFG3, &AFIO->ANAEN_CFG4, &AFIO->ANAEN_CFG5,
    &AFIO->ANAEN_CFG6,
};

// 模拟信号通道引脚映射 (参考 gpio.md): {GPIO引脚, AN_regs索引, 位}
// 注: md 中的 PAx_C/PCx_C 变体引脚未纳入(不是标准 Klipper 引脚)
struct an_io {
    uint8_t pin;
    uint8_t reg;
    uint8_t bit;
};
static const struct an_io An_io[] = {
    // ANAEN_CFG0: PA0/PA1/PA2..PA7 (bit1/bit3/bit4..bit9)
    { GPIO('A',0), 0, 1 }, { GPIO('A',1), 0, 3 },
    { GPIO('A',2), 0, 4 }, { GPIO('A',3), 0, 5 },
    { GPIO('A',4), 0, 6 }, { GPIO('A',5), 0, 7 },
    { GPIO('A',6), 0, 8 }, { GPIO('A',7), 0, 9 },
    // ANAEN_CFG1: PB0/PB1
    { GPIO('B',0), 1, 0 }, { GPIO('B',1), 1, 1 },
    // ANAEN_CFG2: PC0..PC6/PC8/PC13 (PC2_C/PC3_C 变体不用)
    { GPIO('C',0), 2, 0 }, { GPIO('C',1), 2, 1 },
    { GPIO('C',2), 2, 2 }, { GPIO('C',3), 2, 4 },
    { GPIO('C',4), 2, 6 }, { GPIO('C',5), 2, 7 },
    { GPIO('C',6), 2, 8 }, { GPIO('C',8), 2, 9 },
    { GPIO('C',13), 2, 10 },
    // ANAEN_CFG3: PF3..PF14
    { GPIO('F',3), 3, 0 }, { GPIO('F',4), 3, 1 },
    { GPIO('F',5), 3, 2 }, { GPIO('F',6), 3, 3 },
    { GPIO('F',7), 3, 4 }, { GPIO('F',8), 3, 5 },
    { GPIO('F',9), 3, 6 }, { GPIO('F',10), 3, 7 },
    { GPIO('F',11), 3, 8 }, { GPIO('F',12), 3, 9 },
    { GPIO('F',13), 3, 10 }, { GPIO('F',14), 3, 11 },
    // ANAEN_CFG4: PH2..PH5
    { GPIO('H',2), 4, 0 }, { GPIO('H',3), 4, 1 },
    { GPIO('H',4), 4, 2 }, { GPIO('H',5), 4, 3 },
    // ANAEN_CFG5: PI8/PI15
    { GPIO('I',8), 5, 0 }, { GPIO('I',15), 5, 1 },
    // ANAEN_CFG6: PJ0/PJ3..PJ7
    { GPIO('J',0), 6, 0 }, { GPIO('J',3), 6, 1 },
    { GPIO('J',4), 6, 2 }, { GPIO('J',5), 6, 3 },
    { GPIO('J',6), 6, 4 }, { GPIO('J',7), 6, 5 },
};

// 查询gpio是否在An_io表中, 命中返回下标, 否则返回-1
static int
An_io_find(uint32_t gpio)
{
    for (unsigned int i = 0; i < ARRAY_SIZE(An_io); i++)
        if (An_io[i].pin == gpio)
            return i;
    return -1;
}


void
gpio_peripheral(uint32_t gpio, uint32_t mode, int pullup)
{
    GPIO_Module *regs = gpio_pin_to_regs(gpio);
    gpio_clock_enable(regs);

    uint32_t pos = gpio % 16;
    uint32_t mode_bits = mode & 0x0f;
    uint32_t func = (mode >> 4) & 0x0f;
    uint32_t od = (mode & GPIO_OPEN_DRAIN) ? 1U : 0U;
    uint32_t pup = pullup ? (pullup > 0 ? 1U : 2U) : 0U;
    uint32_t speed = (mode & GPIO_LOW_SLEW_RATE) ? 1U : 0U;
    uint32_t ds = (mode >> 10) & 0x03;
    uint32_t shift = pos * 2;
    uint32_t mask2 = 3U << shift;
    uint32_t af_shift = (pos % 8) * 4;
    uint32_t af_mask = 0x0fU << af_shift;

    // Alternate function selection: AF0..AF15 (reset value is AF15)
    if (pos < 8)
        regs->AFL = (regs->AFL & ~af_mask) | (func << af_shift);
    else
        regs->AFH = (regs->AFH & ~af_mask) | (func << af_shift);

    regs->PMODE = (regs->PMODE & ~mask2) | (mode_bits << shift);
    regs->PUPD = (regs->PUPD & ~mask2) | (pup << shift);
    regs->POTYPE = (regs->POTYPE & ~(1U << pos)) | (od << pos);
    regs->SR = (regs->SR & ~(1U << pos)) | (speed << pos);
    regs->DS = (regs->DS & ~mask2) | (ds << shift);

    if (mode == GPIO_ANALOG) {
        // 模拟输入: 若该IO是模拟信号通道(见gpio.md), 使能对应的ANAEN寄存器位
        int idx = An_io_find(gpio);
        if (idx >= 0) {
            if (!is_enabled_pclock(AFIO_BASE))
                enable_pclock(AFIO_BASE);
            *AN_regs[An_io[idx].reg] |= (1U << An_io[idx].bit);
        }
    }
}

struct gpio_out
gpio_out_setup(uint32_t pin, uint32_t val)
{
    GPIO_Module *regs = gpio_pin_to_regs(pin);
    struct gpio_out g = { .regs = regs, .bit = GPIO2BIT(pin) };
    gpio_out_reset(g, val);
    return g;
}

void
gpio_out_reset(struct gpio_out g, uint32_t val)
{
    GPIO_Module *regs = g.regs;
    int pin = regs_to_pin(regs, g.bit);
    irqstatus_t flag = irq_save();
    // Program the output level before switching the pin to output mode
    if (val)
        regs->PBSC = g.bit;
    else
        regs->PBC = g.bit;
    gpio_peripheral(pin, GPIO_OUTPUT, GPIO_PULL_NONE);
    irq_restore(flag);
}

void
gpio_out_toggle_noirq(struct gpio_out g)
{
    GPIO_Module *regs = g.regs;
    // Toggle the POD output data bit directly
    regs->POD ^= g.bit;
}

void
gpio_out_toggle(struct gpio_out g)
{
    irqstatus_t flag = irq_save();
    gpio_out_toggle_noirq(g);
    irq_restore(flag);
}

void
gpio_out_write(struct gpio_out g, uint32_t val)
{
    GPIO_Module *regs = g.regs;
    if (val)
        regs->PBSC = g.bit;
    else
        regs->PBC = g.bit;
}

struct gpio_in
gpio_in_setup(uint32_t pin, int32_t pull_up)
{
    GPIO_Module *regs = gpio_pin_to_regs(pin);
    struct gpio_in g = { .regs = regs, .bit = GPIO2BIT(pin) };
    gpio_in_reset(g, pull_up);
    return g;
}

void
gpio_in_reset(struct gpio_in g, int32_t pull_up)
{
    GPIO_Module *regs = g.regs;
    int pin = regs_to_pin(regs, g.bit);
    irqstatus_t flag = irq_save();
    gpio_peripheral(pin, GPIO_INPUT, pull_up);
    irq_restore(flag);
}

uint8_t
gpio_in_read(struct gpio_in g)
{
    GPIO_Module *regs = g.regs;
    return !!(regs->PID & g.bit);
}
