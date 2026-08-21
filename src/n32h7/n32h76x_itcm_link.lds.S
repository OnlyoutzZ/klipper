// Linker script for N32H76x Flash boot with ITCM runtime execution
//
// Copyright (C) 2026  Nsing
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "autoconf.h"

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(ResetHandler)

MEMORY
{
  flash (rx)  : ORIGIN = CONFIG_FLASH_APPLICATION_ADDRESS, LENGTH = CONFIG_FLASH_SIZE
  itcm  (xrw) : ORIGIN = CONFIG_N32H7_ITCM_START, LENGTH = CONFIG_N32H7_ITCM_SIZE
  dtcm  (rw)  : ORIGIN = CONFIG_N32H7_DTCM_START, LENGTH = CONFIG_N32H7_DTCM_SIZE
  axi   (rw)  : ORIGIN = CONFIG_N32H7_AXI_SRAM_START, LENGTH = CONFIG_N32H7_AXI_SRAM_SIZE
  ram   (rw)  : ORIGIN = CONFIG_RAM_START, LENGTH = CONFIG_RAM_SIZE
  ram5  (rw)  : ORIGIN = CONFIG_N32H7_AHB_SRAM5_START, LENGTH = CONFIG_N32H7_AHB_SRAM5_SIZE
}

SECTIONS
{
    .isr_vector_boot : {
        . = ALIGN(4);
        KEEP(*(.isr_vector_boot))
        . = ALIGN(4);
    } > flash

    .text.boot : {
        . = ALIGN(4);
        KEEP(*(.text.reset))
        KEEP(*(.text.Default_Handler))
        *(.boot_text)
        *(.boot_text.*)
        *(.rodata.boot)
        *(.rodata.boot.*)
        . = ALIGN(4);
    } > flash

    .runtime_image : {
        . = ALIGN(4);
        __image_copy_start__ = .;
        _text_vectortable_start = .;
        __tcm_vector_start__ = .;
        KEEP(*(.vector_table))
        __tcm_vector_end__ = .;
        _text_vectortable_end = .;

        *(.text .text.*)
        *(.rodata .rodata.*)
        *(.glue_7)
        *(.glue_7t)
        *(.gnu.linkonce.t*)

        . = ALIGN(4);
        __image_copy_end__ = .;
    } > itcm AT > flash

    __image_copy_load_start__ = LOADADDR(.runtime_image);

    .data : AT (LOADADDR(.runtime_image) + SIZEOF(.runtime_image)) {
        . = ALIGN(4);
        _data_start = .;
        _sdata = .;
        *(.ramfunc .ramfunc.*)
        *(.data .data.*)
        *(.gnu.linkonce.d*)
        . = ALIGN(4);
        _data_end = .;
        _edata = .;
    } > ram

    _data_flash = LOADADDR(.data);
    _sidata = LOADADDR(.data);

    .bss (NOLOAD) : {
        . = ALIGN(4);
        _bss_start = .;
        _sbss = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
        _ebss = .;
    } > ram

    _boot_stack_start = CONFIG_RAM_START + CONFIG_RAM_SIZE - 1024;
    .boot_stack _boot_stack_start (NOLOAD) : {
        . = . + 1024;
        _boot_stack_end = .;
        _estack = .;
    } > ram

    _dynmem_end = _boot_stack_start;

    .stack (NOLOAD) : {
        . = ALIGN(8);
        _stack_start = .;
        _app_sstack = .;
        . = . + CONFIG_STACK_SIZE;
        . = ALIGN(8);
        _stack_end = .;
        _app_estack = .;
    } > axi

    /DISCARD/ : {
        *(.init)
        *(.fini)
        *(.ARM.extab)
        *(.ARM.exidx)
    }
}
