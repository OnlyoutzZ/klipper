// Hardware PWM support on N32H7
//
// Copyright (C) 2026  Nsing
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "autoconf.h" // CONFIG_CLOCK_FREQ
#include "command.h" // shutdown
#include "gpio.h" // gpio_pwm_write
#include "internal.h" // GPIO
#include "sched.h" // sched_shutdown

#define MAX_PWM (1 << 15)
DECL_CONSTANT("PWM_MAX", MAX_PWM);

struct gpio_pwm_info {
    TIM_Module *timer;
    uint8_t pin, channel, function;
};

// N32H76x timer alternate-function mappings supplied by the chip pin map.
// Only the normal CH1..CH4 outputs are listed (not complementary outputs).
static const struct gpio_pwm_info pwm_regs[] = {
    {ATIM1, GPIO('A', 8),  1, GPIO_FUNCTION(4)},
    {ATIM1, GPIO('E', 9),  1, GPIO_FUNCTION(6)},
    {ATIM1, GPIO('A', 9),  2, GPIO_FUNCTION(6)},
    {ATIM1, GPIO('E', 11), 2, GPIO_FUNCTION(5)},
    {ATIM1, GPIO('A', 10), 3, GPIO_FUNCTION(4)},
    {ATIM1, GPIO('E', 13), 3, GPIO_FUNCTION(5)},
    {ATIM1, GPIO('A', 11), 4, GPIO_FUNCTION(5)},
    {ATIM1, GPIO('E', 14), 4, GPIO_FUNCTION(6)},

    {ATIM2, GPIO('C', 6), 1, GPIO_FUNCTION(8)},
    {ATIM2, GPIO('I', 5), 1, GPIO_FUNCTION(8)},
    {ATIM2, GPIO('C', 7), 2, GPIO_FUNCTION(8)},
    {ATIM2, GPIO('I', 6), 2, GPIO_FUNCTION(7)},
    {ATIM2, GPIO('C', 8), 3, GPIO_FUNCTION(5)},
    {ATIM2, GPIO('I', 7), 3, GPIO_FUNCTION(8)},
    {ATIM2, GPIO('C', 9), 4, GPIO_FUNCTION(9)},
    {ATIM2, GPIO('I', 2), 4, GPIO_FUNCTION(7)},

    {ATIM3, GPIO('A', 6), 1, GPIO_FUNCTION(10)},
    {ATIM3, GPIO('F', 8), 1, GPIO_FUNCTION(6)},
    {ATIM3, GPIO('F', 9), 2, GPIO_FUNCTION(7)},
    {ATIM3, GPIO('H', 10), 2, GPIO_FUNCTION(6)},
    {ATIM3, GPIO('D', 8), 3, GPIO_FUNCTION(5)},
    {ATIM3, GPIO('F', 10), 3, GPIO_FUNCTION(5)},
    {ATIM3, GPIO('D', 14), 4, GPIO_FUNCTION(5)},
    {ATIM3, GPIO('F', 7), 4, GPIO_FUNCTION(5)},

    {ATIM4, GPIO('A', 7), 1, GPIO_FUNCTION(11)},
    {ATIM4, GPIO('F', 9), 1, GPIO_FUNCTION(8)},
    {ATIM4, GPIO('F', 10), 2, GPIO_FUNCTION(6)},
    {ATIM4, GPIO('J', 3), 2, GPIO_FUNCTION(4)},
    {ATIM4, GPIO('F', 7), 3, GPIO_FUNCTION(6)},
    {ATIM4, GPIO('J', 12), 3, GPIO_FUNCTION(5)},
    {ATIM4, GPIO('F', 8), 4, GPIO_FUNCTION(7)},
    {ATIM4, GPIO('J', 14), 4, GPIO_FUNCTION(3)},

    {GTIMA1, GPIO('A', 0), 1, GPIO_FUNCTION(6)},
    {GTIMA1, GPIO('A', 5), 1, GPIO_FUNCTION(9)},
    {GTIMA1, GPIO('A', 1), 2, GPIO_FUNCTION(4)},
    {GTIMA1, GPIO('B', 3), 2, GPIO_FUNCTION(8)},
    {GTIMA1, GPIO('A', 2), 3, GPIO_FUNCTION(3)},
    {GTIMA1, GPIO('B', 10), 3, GPIO_FUNCTION(7)},
    {GTIMA1, GPIO('A', 3), 4, GPIO_FUNCTION(7)},
    {GTIMA1, GPIO('B', 11), 4, GPIO_FUNCTION(4)},

    {GTIMA2, GPIO('A', 6), 1, GPIO_FUNCTION(11)},
    {GTIMA2, GPIO('B', 4), 1, GPIO_FUNCTION(10)},
    {GTIMA2, GPIO('A', 7), 2, GPIO_FUNCTION(12)},
    {GTIMA2, GPIO('B', 5), 2, GPIO_FUNCTION(8)},
    {GTIMA2, GPIO('B', 0), 3, GPIO_FUNCTION(7)},
    {GTIMA2, GPIO('B', 6), 3, GPIO_FUNCTION(4)},
    {GTIMA2, GPIO('B', 1), 4, GPIO_FUNCTION(9)},
    {GTIMA2, GPIO('B', 7), 4, GPIO_FUNCTION(5)},

    {GTIMA3, GPIO('B', 6), 1, GPIO_FUNCTION(5)},
    {GTIMA3, GPIO('D', 12), 1, GPIO_FUNCTION(3)},
    {GTIMA3, GPIO('B', 7), 2, GPIO_FUNCTION(6)},
    {GTIMA3, GPIO('D', 13), 2, GPIO_FUNCTION(3)},
    {GTIMA3, GPIO('B', 8), 3, GPIO_FUNCTION(7)},
    {GTIMA3, GPIO('D', 14), 3, GPIO_FUNCTION(6)},
    {GTIMA3, GPIO('B', 9), 4, GPIO_FUNCTION(6)},
    {GTIMA3, GPIO('D', 15), 4, GPIO_FUNCTION(6)},

    {GTIMA4, GPIO('A', 0), 1, GPIO_FUNCTION(8)},
    {GTIMA4, GPIO('H', 10), 1, GPIO_FUNCTION(7)},
    {GTIMA4, GPIO('A', 1), 2, GPIO_FUNCTION(5)},
    {GTIMA4, GPIO('H', 11), 2, GPIO_FUNCTION(6)},
    {GTIMA4, GPIO('A', 2), 3, GPIO_FUNCTION(4)},
    {GTIMA4, GPIO('H', 12), 3, GPIO_FUNCTION(8)},
    {GTIMA4, GPIO('A', 3), 4, GPIO_FUNCTION(8)},
    {GTIMA4, GPIO('I', 0), 4, GPIO_FUNCTION(7)},

    {GTIMA5, GPIO('F', 0), 1, GPIO_FUNCTION(7)},
    {GTIMA5, GPIO('F', 6), 1, GPIO_FUNCTION(8)},
    {GTIMA5, GPIO('F', 1), 2, GPIO_FUNCTION(8)},
    {GTIMA5, GPIO('F', 7), 2, GPIO_FUNCTION(7)},
    {GTIMA5, GPIO('F', 2), 3, GPIO_FUNCTION(8)},
    {GTIMA5, GPIO('F', 8), 3, GPIO_FUNCTION(8)},
    {GTIMA5, GPIO('F', 3), 4, GPIO_FUNCTION(8)},
    {GTIMA5, GPIO('F', 9), 4, GPIO_FUNCTION(9)},

    {GTIMA6, GPIO('F', 11), 1, GPIO_FUNCTION(4)},
    {GTIMA6, GPIO('K', 4), 1, GPIO_FUNCTION(4)},
    {GTIMA6, GPIO('F', 12), 2, GPIO_FUNCTION(5)},
    {GTIMA6, GPIO('K', 5), 2, GPIO_FUNCTION(4)},
    {GTIMA6, GPIO('F', 13), 3, GPIO_FUNCTION(6)},
    {GTIMA6, GPIO('K', 6), 3, GPIO_FUNCTION(4)},
    {GTIMA6, GPIO('F', 14), 4, GPIO_FUNCTION(5)},
    {GTIMA6, GPIO('K', 7), 4, GPIO_FUNCTION(4)},

    {GTIMA7, GPIO('B', 14), 1, GPIO_FUNCTION(8)},
    {GTIMA7, GPIO('H', 6), 1, GPIO_FUNCTION(7)},
    {GTIMA7, GPIO('B', 15), 2, GPIO_FUNCTION(8)},
    {GTIMA7, GPIO('H', 9), 2, GPIO_FUNCTION(6)},
    {GTIMA7, GPIO('I', 9), 3, GPIO_FUNCTION(5)},
    {GTIMA7, GPIO('I', 12), 3, GPIO_FUNCTION(5)},
    {GTIMA7, GPIO('I', 10), 4, GPIO_FUNCTION(6)},
    {GTIMA7, GPIO('I', 13), 4, GPIO_FUNCTION(5)},

    {GTIMB1, GPIO('A', 2), 1, GPIO_FUNCTION(5)},
    {GTIMB1, GPIO('C', 12), 1, GPIO_FUNCTION(9)},
    {GTIMB1, GPIO('A', 3), 2, GPIO_FUNCTION(9)},
    {GTIMB1, GPIO('E', 6), 2, GPIO_FUNCTION(5)},
    {GTIMB1, GPIO('E', 0), 3, GPIO_FUNCTION(7)},
    {GTIMB1, GPIO('E', 1), 4, GPIO_FUNCTION(6)},

    {GTIMB2, GPIO('B', 8), 1, GPIO_FUNCTION(8)},
    {GTIMB2, GPIO('D', 1), 1, GPIO_FUNCTION(4)},
    {GTIMB2, GPIO('D', 2), 2, GPIO_FUNCTION(9)},
    {GTIMB2, GPIO('F', 5), 2, GPIO_FUNCTION(8)},
    {GTIMB2, GPIO('D', 3), 3, GPIO_FUNCTION(6)},
    {GTIMB2, GPIO('F', 4), 3, GPIO_FUNCTION(9)},
    {GTIMB2, GPIO('D', 4), 4, GPIO_FUNCTION(6)},
    {GTIMB2, GPIO('F', 3), 4, GPIO_FUNCTION(9)},

    {GTIMB3, GPIO('B', 9), 1, GPIO_FUNCTION(7)},
    {GTIMB3, GPIO('F', 7), 1, GPIO_FUNCTION(8)},
    {GTIMB3, GPIO('H', 2), 2, GPIO_FUNCTION(7)},
    {GTIMB3, GPIO('J', 2), 2, GPIO_FUNCTION(5)},
    {GTIMB3, GPIO('H', 3), 3, GPIO_FUNCTION(6)},
    {GTIMB3, GPIO('J', 3), 3, GPIO_FUNCTION(5)},
    {GTIMB3, GPIO('H', 4), 4, GPIO_FUNCTION(5)},
    {GTIMB3, GPIO('J', 4), 4, GPIO_FUNCTION(5)},
};

struct gpio_pwm
gpio_pwm_setup(uint8_t pin, uint32_t cycle_time, uint32_t val)
{
    const struct gpio_pwm_info *p = pwm_regs;
    for (;; p++) {
        if (p >= &pwm_regs[ARRAY_SIZE(pwm_regs)])
            shutdown("Not a valid PWM pin");
        if (p->pin == pin)
            break;
    }
    gpio_peripheral(p->pin, p->function, 0);

    uint32_t pclk = get_pclock_frequency((uint32_t)p->timer);
    uint32_t pclock_div = CONFIG_CLOCK_FREQ / pclk;
    uint32_t pcycle_time = cycle_time / pclock_div;

    uint32_t hwpwm_ticks = pcycle_time, prescaler = 1, shift = 0;
    while (hwpwm_ticks > UINT16_MAX) {
        shift++;
        hwpwm_ticks = (pcycle_time + (1U << (shift - 1))) >> shift;
        prescaler = 1U << shift;
    }
    if (prescaler > UINT16_MAX + 1U) {
        prescaler = UINT16_MAX + 1U;
        hwpwm_ticks = UINT16_MAX;
    }
    if (hwpwm_ticks < 2)
        hwpwm_ticks = 2;

    if (!is_enabled_pclock((uint32_t)p->timer))
        enable_pclock((uint32_t)p->timer);
    if (p->timer->CTRL1 & TIM_CTRL1_CNTEN) {
        if (p->timer->PSC != (uint16_t)(prescaler - 1))
            shutdown("PWM already programmed at different speed");
        if (p->timer->AR != (uint16_t)(hwpwm_ticks - 1))
            shutdown("PWM already programmed with different pulse duration");
    } else {
        p->timer->PSC = prescaler - 1;
        p->timer->AR = hwpwm_ticks - 1;
        p->timer->EVTGEN = TIM_EVTGEN_UDGN;
    }

    struct gpio_pwm channel = { .hwpwm_ticks = hwpwm_ticks };
    uint32_t cshift = ((p->channel - 1) & 1) ? 8 : 0;
    volatile uint32_t *ccmod = p->channel <= 2
                               ? &p->timer->CCMOD1 : &p->timer->CCMOD2;
    uint32_t sel_mask = TIM_CCMOD1_CC1SEL << cshift;
    uint32_t mode_mask = TIM_CCMOD1_OC1MD << cshift;
    uint32_t pwm_mode = (TIM_CCMOD1_OC1MD_1 | TIM_CCMOD1_OC1MD_2
                         | TIM_CCMOD1_OC1PEN | TIM_CCMOD1_OC1FEN) << cshift;
    uint32_t enable = TIM_CCEN_CC1EN << (4 * (p->channel - 1));

    switch (p->channel) {
    case 1: channel.reg = (void *)&p->timer->CCDAT1; break;
    case 2: channel.reg = (void *)&p->timer->CCDAT2; break;
    case 3: channel.reg = (void *)&p->timer->CCDAT3; break;
    case 4: channel.reg = (void *)&p->timer->CCDAT4; break;
    default: shutdown("Invalid PWM channel");
    }
    p->timer->CCEN &= ~enable;
    *ccmod = (*ccmod & ~(sel_mask | mode_mask)) | pwm_mode;
    gpio_pwm_write(channel, val);
    p->timer->CCEN |= enable;

    // The official driver exposes MOE only on advanced timers and GTIMB.
    if (p->timer == ATIM1 || p->timer == ATIM2
        || p->timer == ATIM3 || p->timer == ATIM4
        || p->timer == GTIMB1 || p->timer == GTIMB2
        || p->timer == GTIMB3)
        p->timer->BKDT |= TIM_BKDT_MOEN;
    p->timer->CTRL1 |= TIM_CTRL1_CNTEN;
    return channel;
}

void
gpio_pwm_write(struct gpio_pwm g, uint32_t val)
{
    uint32_t r = DIV_ROUND_CLOSEST(val * g.hwpwm_ticks, MAX_PWM);
    volatile uint32_t *reg = g.reg;
    // CCDATx[31:16] is the down-counting compare value on N32H7.
    *reg = (*reg & 0xffff0000U) | r;
}
