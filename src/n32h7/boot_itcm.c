// N32H76x ITCM boot support
//
// Copyright (C) 2026  Nsing
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <stddef.h>
#include <stdint.h>
#include "autoconf.h" // CONFIG_CLOCK_FREQ
#include "board/armcm_boot.h" // DECL_ARMCM_IRQ
#include "board/internal.h" // n32h7_system_init
#include "board/misc.h" // dynmem_start
#include "compiler.h" // __always_inline

extern uint32_t __image_copy_load_start__, __image_copy_start__;
extern uint32_t __image_copy_end__;
extern uint32_t _data_flash, _data_start, _data_end;
extern uint32_t _bss_start, _bss_end, _stack_end, _boot_stack_end;
extern uint32_t _dynmem_end, __tcm_vector_start__;

void ResetHandler(void);
void DefaultHandler(void);
void N32SysTick_Handler(void); // boot-stage SysTick stub - see definition below

// Flash (boot) vector table. 
const void * const BootVector[] __visible __section(".isr_vector_boot") = {
    &_boot_stack_end,          // [0]  initial SP (AHB_SRAM top)
    ResetHandler,              // [1]  reset
    DefaultHandler,            // [2]  NMI
    DefaultHandler,            // [3]  HardFault
    DefaultHandler,            // [4]  MemManage
    DefaultHandler,            // [5]  BusFault
    DefaultHandler,            // [6]  UsageFault
    0,                         // [7]  reserved
    0,                         // [8]  reserved
    0,                         // [9]  reserved
    0,                         // [10] reserved
    DefaultHandler,            // [11] SVC
    DefaultHandler,            // [12] DebugMon
    0,                         // [13] reserved
    DefaultHandler,            // [14] PendSV
    N32SysTick_Handler,        // [15] SysTick -> flash stub (no ITCM access)
};

static void __always_inline __section(".text.reset")
boot_memcpy(void *dest, const void *src, size_t n)
{
    const uint32_t *s = src;
    volatile uint32_t *d = dest;
    while (n) {
        *d++ = *s++;
        n -= sizeof(*d);
    }
}

static void __always_inline __section(".text.reset")
boot_memset(void *dest, int c, size_t n)
{
    volatile uint32_t *d = dest;
    while (n) {
        *d++ = c;
        n -= sizeof(*d);
    }
}

// Early system setup - runs in flash on the boot image before the runtime
// image is copied to ITCM (so it must stay out of ITCM).
void __section(".text.reset")
n32h7_system_init(void)
{
    RCC->AHB5EN2 |= RCC_AHB5EN2_PWREN;

    SystemInit();
    SystemCoreClock = CONFIG_CLOCK_FREQ;
    SCB->CPACR |= (0xfUL << 20);
    SCB->SCR |= SCB_SCR_SEVONPEND_Msk;

    __DSB();
    __ISB();
}

static void __noreturn __section(".text.reset")
run_from_itcm(void)
{
    uint32_t count;

    n32h7_system_init();

    count = (&__image_copy_end__ - &__image_copy_start__) * 4;
    boot_memcpy(&__image_copy_start__, &__image_copy_load_start__, count);

    count = (&_data_end - &_data_start) * 4;
    boot_memcpy(&_data_start, &_data_flash, count);

    count = (&_bss_end - &_bss_start) * 4;
    boot_memset(&_bss_start, 0, count);

    SCB->VTOR = (uint32_t)&__tcm_vector_start__;
    __DSB();
    __ISB();

    __set_MSP((uint32_t)&_stack_end);
    __enable_irq();

    armcm_main();
    for (;;)
        ;
}

void __visible __section(".text.reset")
ResetHandler(void)
{
    __disable_irq();

    for (int i = 0; i < (int)ARRAY_SIZE(NVIC->ICER); i++) {
        NVIC->ICER[i] = 0xffffffff;
        __DSB();
        NVIC->ICPR[i] = 0xffffffff;
    }

    for (int i = 0; i < (int)ARRAY_SIZE(NVIC->IP); i++)
        NVIC->IP[i] = 0;

    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk;
    __DSB();
    SCB->ICSR = SCB_ICSR_PENDSVCLR_Msk | SCB_ICSR_PENDSTCLR_Msk;

    for (int i = 0; i < (int)ARRAY_SIZE(SCB->SHPR); i++)
        SCB->SHPR[i] = 0;

    asm volatile("mov sp, %0\n bx %1"
                 : : "r"(&_boot_stack_end), "r"(run_from_itcm));
}
DECL_ARMCM_IRQ(ResetHandler, -15);

void __section(".text.Default_Handler")
DefaultHandler(void)
{
    for (;;)
        ;
}

// Flash-resident stub for the boot-stage SysTick vector.
//
// The flash BootVector[15] must point here instead of to the real
// SysTick_Handler (from generic/armcm_timer.c): before SCB->VTOR is
// switched to ITCM, the CPU fetches vectors from flash and the runtime
// vector table (with the real handler) has not been copied yet.  SysTick
// is disabled during boot (ResetHandler only sets CLKSOURCE), so this
// stub should never be invoked - it exists only so that the flash vector
// table never points into the not-yet-copied ITCM image.
void __section(".text.Default_Handler")
N32SysTick_Handler(void)
{
}

void *
dynmem_start(void)
{
    return &_bss_end;
}

void *
dynmem_end(void)
{
    return &_dynmem_end;
}
