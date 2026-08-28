// ADC functions on N32H7
//
// Copyright (C) 2026  Nsing
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "board/irq.h" // irq_save
#include "board/misc.h" // timer_from_us
#include "command.h" // shutdown
#include "compiler.h" // ARRAY_SIZE
#include "generic/armcm_timer.h" // udelay
#include "gpio.h" // gpio_adc_setup
#include "internal.h" // GPIO
#include "sched.h" // sched_shutdown

DECL_CONSTANT("ADC_MAX", 4095);

#define ADC_TEMPERATURE_PIN 0xfe
DECL_ENUMERATION("pin", "ADC_TEMPERATURE", ADC_TEMPERATURE_PIN);

static const uint8_t adc_pins[] = {
    GPIO('A', 0), GPIO('A', 1), GPIO('A', 2), GPIO('A', 3),
    GPIO('A', 4), GPIO('A', 5), GPIO('A', 6), GPIO('A', 7),
    GPIO('B', 0), GPIO('B', 1), GPIO('C', 0), GPIO('C', 1),
    GPIO('C', 2), GPIO('C', 3), GPIO('C', 4), GPIO('C', 5),

#if CONFIG_MACH_N32H7
    ADC_TEMPERATURE_PIN,
#endif

#if CONFIG_MACH_N32H7
    0x00, 0x00, 0x00, 0x00,
    GPIO('F', 6), GPIO('F', 7), GPIO('F', 8), GPIO('F', 9),
    GPIO('F', 10), GPIO('F', 3), 0x00, 0x00,
    0x00, 0x00, GPIO('F', 4), GPIO('F', 5),
#endif
};

// ADC timing:
static void
adc_calibrate(ADC_Module *adc)
{
    adc->CTRL2 = ADC_CTRL2_ON;
    while (!(adc->STS& ADC_STS_RDY))
        ;
    adc->CTRL3 &= ~ADC_CTRL3_BPCAL;
    udelay(10);
    adc->CTRL2 |= ADC_CTRL2_ENCAL;
    while (adc->CTRL2 & ADC_CTRL2_ENCAL)
        ;
}

struct gpio_adc
gpio_adc_setup(uint32_t pin)
{
    // Find pin in adc_pins table
    int chan;
    for (chan=0; ; chan++) {
        if (chan >= ARRAY_SIZE(adc_pins))
            shutdown("Not a valid ADC pin");
        if (adc_pins[chan] == pin)
            break;
    }

    // Determine which ADC block to use
    ADC_Module *adc = ADC1;
    uint32_t adc_base = ADC1_BASE;
    if (chan >= 19) {
        // Channels 19 and above are only available from ADC3
        adc = ADC3;
        adc_base = ADC3_BASE;
        chan -= 19;
    }

    // Enable the ADC
    if (!is_enabled_pclock(adc_base)) {
        enable_pclock(adc_base);
        adc_calibrate(adc);
        uint32_t aticks = 4; // mid-range sample time (see datasheet)
        adc->SAMPT1 = (aticks | (aticks << 4) | (aticks << 8) | (aticks << 12)
                       | (aticks << 16) | (aticks << 20) | (aticks << 24)
                       | (aticks << 28));
        adc->SAMPT2 = adc->SAMPT1;
        adc->SAMPT3 = (aticks | (aticks << 4) | (aticks << 8) | (aticks << 12));
    }

    if (pin == ADC_TEMPERATURE_PIN) {
        ADC1->CTRL2 |= ADC_CTRL2_TEMPEN;
        enable_pclock(AFIO_BASE);
        AFIO->ADCSW_CFG |= AFIO_ADCSW_CFG_SW_TEMP;
    } else {
        gpio_peripheral(pin, GPIO_ANALOG, 0);
    }

    return (struct gpio_adc){ .adc = adc, .chan = chan };
}

// Try to sample a value. Returns zero if sample ready, otherwise
// returns the number of clock ticks the caller should wait before
// retrying this function.
uint32_t
gpio_adc_sample(struct gpio_adc g)
{
    ADC_Module *adc = g.adc;
    uint32_t sr = adc->STS;
    if (sr & ADC_STS_STR) {
        if (!(sr & ADC_STS_ENDC) || adc->RSEQ4 != g.chan)
            // Conversion still in progress or busy on another channel
            goto need_delay;
        // Conversion ready
        return 0;
    }
    // Start sample
    adc->RSEQ4 = g.chan;
    adc->CTRL3 |= ADC_CTRL3_RSTART;
    adc->CTRL2 |= ADC_CTRL2_SWSTRRCH;

need_delay:
    return timer_from_us(20);
}

// Read a value; use only after gpio_adc_sample() returns zero
uint16_t
gpio_adc_read(struct gpio_adc g)
{
    ADC_Module *adc = g.adc;
    adc->STS = (ADC_STS_ENDC | ADC_STS_ENDCA | ADC_STS_STR);
    return adc->DAT;
}

// Cancel a sample that may have been started with gpio_adc_sample()
void
gpio_adc_cancel_sample(struct gpio_adc g)
{
    ADC_Module *adc = g.adc;
    irqstatus_t flag = irq_save();
    if (adc->STS & ADC_STS_STR && adc->RSEQ4 == g.chan)
        gpio_adc_read(g);
    irq_restore(flag);
}
