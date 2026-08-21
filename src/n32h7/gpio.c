// GPIO functions on n32h7
//
// Copyright (C) 2026  Nsing
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "board/irq.h" // irq_save
#include "command.h" // DECL_ENUMERATION_RANGE
#include "generic/io.h" // readw, writew
#include "gpio.h" // gpio_out_setup
#include "internal.h" // gpio_peripheral
#include "sched.h" // shutdown

// Note: the N32H7 reference manual requires that GPIOx_SR, GPIOx_PID,
// GPIOx_POD, GPIOx_PBSC and GPIOx_PBC only be accessed as 16-bit
// words.  All other GPIO registers support 32-bit accesses.

static uint16_t
gpio_readw(volatile uint32_t *reg)
{
    return readw((const void *)reg);
}
static void
gpio_writew(volatile uint32_t *reg, uint16_t val)
{
    writew((void *)reg, val);
}

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
    uint32_t speed = (mode & GPIO_HIGH_SPEED) ? 0U : 1U;
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
    // GPIOx_SR only supports 16-bit accesses
    uint16_t sr = gpio_readw(&regs->SR);
    gpio_writew(&regs->SR, (sr & ~(1U << pos)) | (speed << pos));
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
        gpio_writew(&regs->PBSC, g.bit);
    else
        gpio_writew(&regs->PBC, g.bit);
    gpio_peripheral(pin, GPIO_OUTPUT, 0);
    irq_restore(flag);
}

void
gpio_out_toggle_noirq(struct gpio_out g)
{
    GPIO_Module *regs = g.regs;
    // POD holds the last value written; toggle via a set or clear write
    if (gpio_readw(&regs->POD) & g.bit)
        gpio_writew(&regs->PBC, g.bit);
    else
        gpio_writew(&regs->PBSC, g.bit);
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
        gpio_writew(&regs->PBSC, g.bit);
    else
        gpio_writew(&regs->PBC, g.bit);
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
    return !!(gpio_readw(&regs->PID) & g.bit);
}
