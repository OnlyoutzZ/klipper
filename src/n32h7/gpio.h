#ifndef __N32H7_GPIO_H
#define __N32H7_GPIO_H

#include <stdint.h> // uint32_t

struct gpio_out {
    void *regs;
    uint32_t bit;
};
struct gpio_out gpio_out_setup(uint32_t pin, uint32_t val);
void gpio_out_reset(struct gpio_out g, uint32_t val);
void gpio_out_toggle_noirq(struct gpio_out g);
void gpio_out_toggle(struct gpio_out g);
void gpio_out_write(struct gpio_out g, uint32_t val);

struct gpio_in {
    void *regs;
    uint32_t bit;
};
struct gpio_in gpio_in_setup(uint32_t pin, int32_t pull_up);
void gpio_in_reset(struct gpio_in g, int32_t pull_up);
uint8_t gpio_in_read(struct gpio_in g);

struct gpio_pwm {
    void *reg;
    uint32_t hwpwm_ticks;
};
struct gpio_pwm gpio_pwm_setup(uint8_t pin, uint32_t cycle_time, uint32_t val);
void gpio_pwm_write(struct gpio_pwm g, uint32_t val);

struct spi_config {
    void *spi;
    uint32_t spi_cr1;
};
struct spi_config spi_setup(uint32_t bus, uint8_t mode, uint32_t rate);
void spi_prepare(struct spi_config config);
void spi_transfer(struct spi_config config, uint8_t receive_data
                  , uint8_t len, uint8_t *data);

// i2c_config is only referenced by i2ccmds.h (via sensor code that also uses
// SPI); the N32H7 I2C driver is not ported yet, so the type is defined but
// unused unless CONFIG_HAVE_GPIO_I2C is enabled.
struct i2c_config {
    void *i2c;
    uint8_t addr;
};

#endif // gpio.h
