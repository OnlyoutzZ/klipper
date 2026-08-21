// Code to enable clock lines on N32H7
//
// Copyright (C) 2026  Nsing
//
// This file may be distributed under the terms of the GNU GPLv3 license.
//
// This is the chip-independent part of the clock enable logic.  The actual
// peripheral -> enable/reset bit map and the bus clock frequencies are
// chip-specific and implemented in n32h76x.c (lookup_clock_line and
// get_pclock_frequency), following the same split used by the stm32 port.

#include "board/irq.h" // irq_save
#include "internal.h" // enable_pclock

// Enable a peripheral clock and reset it so it starts in a known state
void
enable_pclock(uint32_t periph_base)
{
    struct cline cl = lookup_clock_line(periph_base);
    if (!cl.en)
        return;
    irqstatus_t flag = irq_save();
    *cl.en |= cl.bit;
    *cl.en; // Pause 2 cycles to ensure peripheral is enabled
    if (cl.rst) {
        // Reset peripheral
        *cl.rst |= cl.rst_bit;
        *cl.rst &= ~cl.rst_bit;
    }
    irq_restore(flag);
}

// Check if a peripheral clock has been enabled
int
is_enabled_pclock(uint32_t periph_base)
{
    struct cline cl = lookup_clock_line(periph_base);
    if (!cl.en)
        return 0;
    return *cl.en & cl.bit;
}

