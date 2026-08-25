#ifndef __N32H7_INTERNAL_H
#define __N32H7_INTERNAL_H
// Local definitions for N32H7 code.

#include <stdint.h>
#include "autoconf.h"
#include "n32h7xx.h"
#include "system_n32h7xx.h"

#define GPIO(PORT, NUM) (((PORT) - 'A') * 16 + (NUM))
#define GPIO2PORT(PIN) ((PIN) / 16)
#define GPIO2BIT(PIN) (1U << ((PIN) % 16))

#define GPIO_INPUT 0U
#define GPIO_OUTPUT 1U
#define GPIO_FUNCTION(fn) (2U | ((fn) << 4))
#define GPIO_ANALOG 3U
#define GPIO_OPEN_DRAIN 0x100U  // default is push-pull
#define GPIO_LOW_SLEW_RATE 0x200U // default is high slew rate
#define GPIO_DS_LEVEL0  0x000U
#define GPIO_DS_LEVEL1  0x400U
#define GPIO_DS_LEVEL2  0x800U
#define GPIO_DS_LEVEL3  0xC00U
#define GPIO_PULL_NONE  0x00U
#define GPIO_PULL_UP    0x01U
#define GPIO_PULL_DOWN  0x02U

struct cline {
    volatile uint32_t *en;
    volatile uint32_t *rst;
    uint32_t bit;
    uint32_t rst_bit;
};

struct cline lookup_clock_line(uint32_t periph_base);
void n32h7_system_init(void);
void gpio_peripheral(uint32_t gpio, uint32_t mode, int pullup);
GPIO_Module *gpio_pin_to_regs(uint32_t pin);
void gpio_clock_enable(GPIO_Module *regs);
void enable_pclock(uint32_t periph_base);
int is_enabled_pclock(uint32_t periph_base);
uint32_t get_pclock_frequency(uint32_t periph_base);

#endif // internal.h
