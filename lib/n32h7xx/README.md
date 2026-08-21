# Nsing N32H7xx CMSIS Device Layer

This directory is the Nsing N32H7xx device layer used by the Klipper
`src/n32h7` MCU port.

The files in this directory should be maintained as a complete CMSIS
device source drop, similar to the STM32 device directories in Klipper:

```text
include/n32h7xx.h
include/system_n32h7xx.h
system_n32h7xx.c
```

Klipper-specific startup, linker, vector-table relocation, UART, GPIO,
and scheduler integration belong in `src/n32h7/`, not in this directory.

## Local Integration Notes

The N32H76x Klipper port boots from Flash at `0x15000000`, copies the
runtime image to ITCM at `0x00000000`, then switches `SCB->VTOR` to the
Klipper-generated runtime vector table in ITCM. The vendor device layer
must therefore remain generic; the ITCM relocation policy is owned by
`src/n32h7/boot_itcm.c` and `src/n32h7/n32h76x_itcm_link.lds.S`.

The current local delta in `system_n32h7xx.c` is intentionally small:

```text
TCM_SIZE_VALUE is made overrideable by the build system.
N32H7_SYSTEM_CLOCK_HZ can override the default SystemCoreClock value.
```

Keep any future local changes similarly small and document them here.

## Upstream Requirement

Before submitting this directory to Klipper upstream, Nsing should
provide a redistributable, GPLv3-compatible release of these CMSIS device
files. Do not submit SDK packages, IDE projects, examples, generated
build output, or external absolute-path references.
