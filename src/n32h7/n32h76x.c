// Main entry point for N32H76x
//
// Copyright (C) 2026  Nsing
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "autoconf.h" // CONFIG_MCU
#include "board/armcm_boot.h" // armcm_main
#include "command.h" // DECL_CONSTANT_STR
#include "generic/armcm_timer.h" // udelay
#include "internal.h" // n32h7_system_init
#include "sched.h" // sched_main

DECL_CONSTANT_STR("MCU", CONFIG_MCU);

// PLL1 input clock sources (PLL1CTRL1 bits [29:28])
#define RCC_PLL_SRC_HSI          ((uint32_t)0x00000000)
#define RCC_PLL_SRC_MSI          ((uint32_t)0x20000000U)
#define RCC_PLL_SRC_HSE          ((uint32_t)(0x10000000U | 0x20000000U))

// PLL parameter calculation bounds
#define REF_MIN  1000000
#define REF_MAX  64000000
#define VCO_MIN  300000000
#define VCO_MAX  1250000000
#define NR_MIN   1
#define NR_MAX   64
#define NF_MIN   2
#define NF_MAX   4095


/****************************************************************
 * Clock setup
 ****************************************************************/

// Resets the RCC clock configuration to the default reset state
static void
rcc_deinit(void)
{
    /* Set HSIEN bit */
    RCC->SRCCTRL1 |= (uint32_t)0x00000001U;

    /* Reset SCLKSW, MSIEN ,HSEBP, HSEEN bits */
    RCC->SRCCTRL1 &= (uint32_t)0xFCFFFFABU;

    /* Reset M7HYPSEL,AXIHYPSEL*/
    RCC->SRCCTRL2 &= (uint32_t)0xFFFCFFFFU;

    /* Reset SYSBUSDIV1,  SYSBUSDIV2 */
    RCC->SYSBUSDIV1 = 0x00000000U;
    RCC->SYSBUSDIV2 = 0x00000000U;

    /* Reset PLL1CTRL1 */
    RCC->PLL1CTRL1 = (uint32_t)0x19030004U;

    /* Reset PLL2CTRL1 */
    RCC->PLL2CTRL1 &= (uint32_t)0x19030005U;

    /* Reset PLL3CTRL1 */
    RCC->PLL3CTRL1 &= (uint32_t)0x19030005U;

    /* Reset CFG2 register */
    RCC->CFG2 = 0x55ACCCCCU;

    /* Reset CFG3 register */
    RCC->CFG3 = 0x00000044U;

    /* Reset CFG4 register */
    RCC->CFG4 = 0xFFFFFDFEU;

    /* Reset CFG5 register */
    RCC->CFG5 = 0x02C10000U;

    /* Disable all interrupts and clear pending bits  */
    RCC->CLKINT1 = 0x00000000U;
    RCC->CLKINT2 = 0x00000000U;
    RCC->CLKINT3 = 0x00000000U;
}

static int
rcc_calculate_pll_param(uint64_t fin, uint64_t fout, uint32_t* nr, uint32_t* nf, uint32_t* wb)
{
    int status;
    uint64_t err_t;
    uint32_t nr_t;
    uint32_t nf_all;
    uint32_t nf_int;
    uint64_t err = (uint64_t)0xFFFFFFFFU;

    /* get NR NF WB parameter */
    if ((fin > REF_MAX) || (fin < REF_MIN) || (fout > VCO_MAX) || (fout < VCO_MIN))
    {
        status = 0;
    }
    else
    {
        for (nr_t = NR_MAX; nr_t >= NR_MIN; nr_t--)
        {
            nf_all = (uint32_t)(fout * nr_t * 256 / fin);
            nf_int = nf_all / 256;

            if ((nf_int > NF_MAX) || (nf_int < NF_MIN))
            {
                continue;
            }

            if ((fin * nf_all / nr_t / 256) > fout)
            {
                err_t = ((fin * nf_all / nr_t / 256) - fout);
            }
            else
            {
                err_t = (fout - (fin * nf_all / nr_t / 256));
            }

            if (err_t <= err)
            {
                err = err_t;
                *nr = nr_t;
                *nf = nf_all;
            }
            else
            {
                /* In other cases, the cycle continues*/
            }
        }

        *wb = (uint32_t)((*nf / 2 / 256) - 1);

        if (*wb > 2047)
        {
            *wb = 2047;
        }

        *nr = *nr - 1;
        *nf = *nf * 64;
        status = 1;
    }

    return status;
}


// Select PLL1A as the system clock source.
//
// M7 runs at sysclk_freq; M4, AXI and AHB(1,2,5,6,9) run at
// sysclk_freq/2; APB(1,2,5,6) runs at sysclk_freq/4 when
// sysclk_freq > 300MHz, otherwise at sysclk_freq/2.
//
// pll_source selects the PLL entry clock: RCC_PLL_SRC_HSI,
// RCC_PLL_SRC_HSE or RCC_PLL_SRC_MSI.
static void
clock_setup_to_pll1(uint32_t pll_source, uint32_t sysclk_freq)
{
    uint64_t input_freq, pll_freq;
    uint32_t pll1a_divider;
    uint32_t nrtmp = 0, nftmp = 0, wbtmp = 0;
    uint32_t reg_value1;
    uint32_t reg_value2;
    uint32_t timeout;

    rcc_deinit();

    if(pll_source == RCC_PLL_SRC_HSE)
    {
        input_freq = HSE_VALUE;

        /* Reset HSERDCNTEN bit */
        RCC->SRCCTRL1 &= (~0x00000100U);

        /* Set HSEEN bit */
        RCC->SRCCTRL1 |= 0x00000004U;
        /* let HSE run before enabling the ready-detection counter */
        udelay(50);

        /* Set HSERDCNTEN bit */
        RCC->SRCCTRL1 |= (0x00000100U);

        /* Wait till HSE is ready */
        timeout = 100000;
        while ((RCC->SRCCTRL1 & 0x00000008U) == 0U && timeout--)
            ;
        if (!timeout)
            try_shutdown("HSE failed to start");
    }
    else if(pll_source == RCC_PLL_SRC_MSI)
    {
        input_freq = MSI_VALUE;

        /* Set MSIEN bit */
        RCC->SRCCTRL1 |= 0x00000040U;

        /* Wait till MSI is ready */
        timeout = 100000;
        while ((RCC->SRCCTRL1 & (0x00000080U | 0x80000000U)) == 0U && timeout--)
            ;
        if (!timeout)
            try_shutdown("MSI failed to start");
    }
    else
    {
        input_freq = HSI_VALUE;

        /* Set HSIEN bit */
        RCC->SRCCTRL1 |= 0x00000001U;

        /* Wait till HSI is ready */
        timeout = 100000;
        while ((RCC->SRCCTRL1 & (0x00000002U | 0x40000000U)) == 0U && timeout--)
            ;
        if (!timeout)
            try_shutdown("HSI failed to start");
    }

    /* Configure sys_div_clk is sys_clk(PLL1A) = M7 clock*/
    reg_value1 = RCC->SYSBUSDIV1;
    /* Clear SCLKDIV bits */
    reg_value1 &= ~(uint32_t)0x0000000FU;
    /* Set the SCLKDIV bits */
    reg_value1 |= 0x00000000U;
    /* Store the new value */
    RCC->SYSBUSDIV1 = reg_value1;

    /* Configure sys_bus_div_clk is sys_div_clk/2 = M4 clock = AHB1\2\5\9*/
    reg_value1 = RCC->SYSBUSDIV1;
    /* Clear BUSDIV bits */
    reg_value1 &= ~(uint32_t)0x00000F00U;
    /* Set the BUSDIV bits */
    reg_value1 |= 0x00000100U;
    /* Store the new value */
    RCC->SYSBUSDIV1 = reg_value1;

    /* Configure AXI clock is sys_div_clk/2 = AHB6*/
    reg_value1 = RCC->SYSBUSDIV1;
    /* Clear AXIDIV bits */
    reg_value1 &=  ~(uint32_t)0x0000F000U;
    /* Set the AXIDIV bits */
    reg_value1 |= 0x00001000U;
    /* Store the new value */
    RCC->SYSBUSDIV1 = reg_value1;

    reg_value1 = RCC->SYSBUSDIV1;
    /* Clear AXIHYPDIV bits */
    reg_value1 &= ~(uint32_t)0x0F000000U;
    /* Set the AXIHYPDIV bits */
    reg_value1 |= 0x01000000U;
    /* Store the new value */
    RCC->SYSBUSDIV1 = reg_value1;

    udelay(1);

    if(sysclk_freq >= 400000000)
    {
        pll_freq = sysclk_freq;
        pll1a_divider = 0x00000001U;
    }
    else if(sysclk_freq >= (400000000/2))
    {
        pll_freq = sysclk_freq * 2U;
        pll1a_divider = 0x00000002U;
    }
    else
    {
        pll_freq = sysclk_freq * 3U;
        pll1a_divider = 0x00000003U;
    }

    /* Read the APB1DIV bits  */
    reg_value1 = RCC->SYSBUSDIV2;
    /* Clear APB1DIV bits */
    reg_value1 &=  ~(uint32_t)(0x00000007U | 0x00000700U | 0x00070000U | 0x07000000U);
    if(sysclk_freq <= 300000000)
    {
        /* Set the APB1DIV bits */
        reg_value1 |= 0x00000000U;
    }
    else
    {
        /* Set the APB1DIV bits */
        reg_value1 |= (0x00000004U | 0x00000400U | 0x00040000U | 0x04000000U);
    }
    /* Store the new value */
    RCC->SYSBUSDIV2 = reg_value1;

    if (rcc_calculate_pll_param(input_freq, pll_freq, &nrtmp, &nftmp, &wbtmp) != 0U)
    {
        /* get the register value */
        reg_value1 = RCC->PLL1CTRL1;
        reg_value2 = RCC->PLL1CTRL2;

        /* Clear BWAJ[11:0] bits */
        reg_value1 &= ~0x00000FFFU;
        /* Clear CLKF[25:0] and CLKR[5:0] bits */
        reg_value2 &= 0x00000000;

        /* Set BWAJ[11:0] bits */
        reg_value1 |= (uint32_t)(wbtmp & 0xFFF);
        /* Set CLKF[25:0] and CLKR[5:0]  bits */
        reg_value2 |= (uint32_t)((nrtmp << 26) | (nftmp & 0x3FFFFFF));

        /* Store the new value */
        RCC->PLL1CTRL1  = reg_value1;
        RCC->PLL1CTRL2  = reg_value2;

        /* Enable PLL module power */
        RCC->PLL1CTRL1 |= 0x00080000U;
        udelay(10);

        /* Enable power to analog circuitry in PLL */
        RCC->PLL1CTRL1 &= ~0x00010000U;

        reg_value1 = RCC->PLL1CTRL1;
        reg_value1 &= ~0x30000000U;
        reg_value1 |= pll_source;
        RCC->PLL1CTRL1 = reg_value1;
        udelay(10);

        /* Clear PLL reset */
        RCC->PLL1CTRL1 &= ~0x00020000U;

        timeout = 100000;
        while ((RCC->PLL1CTRL1 & 0x00100000U) != 0x00100000U && timeout--)
            ;
        if (!timeout)
            try_shutdown("PLL1 failed to lock");

        /* Enable PLL */
        RCC->PLL1CTRL1 |= 0x00040000U;
        udelay(1);

        reg_value1 = RCC->PLL1DIV;
        /* Clear RCC_PLL1A_DIV bits */
        reg_value1 &= ~0x0000003FU;
        /* Set the DIV bits */
        reg_value1 |=  pll1a_divider;
        /* Store the new value */
        RCC->PLL1DIV = reg_value1;

        reg_value1 = RCC->SRCCTRL1;
        /* Clear SCLKSW bits */
        reg_value1 &= ~0x03000000U;
        /* Set SCLKSW bits */
        reg_value1 |=  0x03000000U;  
        /* Store the new value */
        RCC->SRCCTRL1 = reg_value1;

        timeout = 100000;
        while ((RCC->SRCCTRL1 & 0x0C000000U) != 0x0C000000U && timeout--)
            ;
        if (!timeout)
            try_shutdown("Failed to switch system clock to PLL1");
    }
    else
    {
        try_shutdown("Invalid PLL1 parameters");
    }
}


/****************************************************************
 * Bootloader
 ****************************************************************/

// Handle reboot requests
void
bootloader_request(void)
{
    NVIC_SystemReset();
}


/****************************************************************
 * Startup
 ****************************************************************/

// Main entry point - called from armcm_boot.c:ResetHandler()
void __noreturn
armcm_main(void)
{
    clock_setup_to_pll1(RCC_PLL_SRC_HSI, CONFIG_CLOCK_FREQ);

    sched_main();
    for (;;)
        ;
}



/****************************************************************
 * Peripheral clock map
 ****************************************************************/

// Map a peripheral address to its M7 enable/reset bits.
// Unknown peripherals map to an all-zero cline (no-op).
struct cline
lookup_clock_line(uint32_t periph_base)
{
    // Peripherals on the AHB1 bus (300MHz)
    if (periph_base >= ADC1_BASE && periph_base <= ADC3_BASE) {
        // ADC1/2/3 use the ADCxBUSEN bit; the reset uses ADCxRST
        uint32_t idx = (periph_base - ADC1_BASE) / 0x400;
        static const struct cline adc_clines[3] = {
            { .en=&RCC->AHB1EN1, .rst=&RCC->AHB1RST1,
              .bit=RCC_AHB1EN1_M7ADC1BUSEN, .rst_bit=RCC_AHB1RST1_ADC1RST },
            { .en=&RCC->AHB1EN4, .rst=&RCC->AHB1RST4,
              .bit=RCC_AHB1EN4_M7ADC2BUSEN, .rst_bit=RCC_AHB1RST4_ADC2RST },
            { .en=&RCC->AHB1EN4, .rst=&RCC->AHB1RST4,
              .bit=RCC_AHB1EN4_M7ADC3BUSEN, .rst_bit=RCC_AHB1RST4_ADC3RST },
        };
        return adc_clines[idx];
    }
    if (periph_base == DMAMUX1_BASE)
        return (struct cline){
            .en=&RCC->AHB1EN1, .rst=&RCC->AHB1RST1,
            .bit=RCC_AHB1EN1_M7DMAMUX1EN, .rst_bit=RCC_AHB1RST1_DMAMUX1RST};
    if (periph_base == DMA1_BASE)
        return (struct cline){
            .en=&RCC->AHB1EN3, .rst=&RCC->AHB1RST3,
            .bit=RCC_AHB1EN3_M7DMA1EN, .rst_bit=RCC_AHB1RST3_DMA1RST};
    if (periph_base == DMA2_BASE)
        return (struct cline){
            .en=&RCC->AHB1EN3, .rst=&RCC->AHB1RST3,
            .bit=RCC_AHB1EN3_M7DMA2EN, .rst_bit=RCC_AHB1RST3_DMA2RST};
    if (periph_base == DMA3_BASE)
        return (struct cline){
            .en=&RCC->AHB1EN3, .rst=&RCC->AHB1RST3,
            .bit=RCC_AHB1EN3_M7DMA3EN, .rst_bit=RCC_AHB1RST3_DMA3RST};
    if (periph_base == SDMMC2_BASE)
        return (struct cline){
            .en=&RCC->AHB1EN1, .rst=&RCC->AHB1RST1,
            .bit=RCC_AHB1EN1_M7SDMMC2EN, .rst_bit=RCC_AHB1RST1_SDMMC2RST};
    if (periph_base == USB_CTRL2_BASE)
        return (struct cline){
            .en=&RCC->AHB1EN1, .rst=&RCC->AHB1RST1,
            .bit=RCC_AHB1EN1_M7USB2EN, .rst_bit=RCC_AHB1RST1_USB2RST};
    if (periph_base == ETH2_BASE)
        return (struct cline){
            .en=&RCC->AHB1EN2, .rst=&RCC->AHB1RST2,
            .bit=RCC_AHB1EN2_M7ETH2MACEN, .rst_bit=RCC_AHB1RST2_ETH2RST};

    // Peripherals clocked via the AHB2 enable/reset registers (300MHz)
    if (periph_base == USB_CTRL1_BASE)
        return (struct cline){
            .en=&RCC->AHB2EN1, .rst=&RCC->AHB2RST1,
            .bit=RCC_AHB2EN1_M7USB1EN, .rst_bit=RCC_AHB2RST1_USB1RST};
    if (periph_base == ETH1_BASE)
        return (struct cline){
            .en=&RCC->AHB2EN2, .rst=&RCC->AHB2RST1,
            .bit=RCC_AHB2EN2_M7ETH1MACEN, .rst_bit=RCC_AHB2RST1_ETH1RST};

    // Peripherals on the AHB5 bus (300MHz) - GPIO handled in gpio.c
    if (periph_base == AFIO_BASE)
        return (struct cline){
            .en=&RCC->AHB5EN2, .rst=&RCC->AHB5RST2,
            .bit=RCC_AHB5EN2_M7AFIOEN, .rst_bit=RCC_AHB5RST2_AFIORST};

    // Peripherals on the APB1 bus (150MHz)
    if (periph_base == USART1_BASE)
        return (struct cline){
            .en=&RCC->APB1EN3, .rst=&RCC->APB1RST3,
            .bit=RCC_APB1EN3_M7USART1EN, .rst_bit=RCC_APB1RST3_USART1RST};
    if (periph_base == USART2_BASE)
        return (struct cline){
            .en=&RCC->APB1EN3, .rst=&RCC->APB1RST3,
            .bit=RCC_APB1EN3_M7USART2EN, .rst_bit=RCC_APB1RST3_USART2RST};
    if (periph_base == USART3_BASE)
        return (struct cline){
            .en=&RCC->APB1EN3, .rst=&RCC->APB1RST3,
            .bit=RCC_APB1EN3_M7USART3EN, .rst_bit=RCC_APB1RST3_USART3RST};
    if (periph_base == USART4_BASE)
        return (struct cline){
            .en=&RCC->APB1EN3, .rst=&RCC->APB1RST3,
            .bit=RCC_APB1EN3_M7USART4EN, .rst_bit=RCC_APB1RST3_USART4RST};
    if (periph_base == UART9_BASE)
        return (struct cline){
            .en=&RCC->APB1EN3, .rst=&RCC->APB1RST3,
            .bit=RCC_APB1EN3_M7UART9EN, .rst_bit=RCC_APB1RST3_UART9RST};
    if (periph_base == UART10_BASE)
        return (struct cline){
            .en=&RCC->APB1EN3, .rst=&RCC->APB1RST3,
            .bit=RCC_APB1EN3_M7UART10EN, .rst_bit=RCC_APB1RST3_UART10RST};
    if (periph_base == UART11_BASE)
        return (struct cline){
            .en=&RCC->APB1EN3, .rst=&RCC->APB1RST3,
            .bit=RCC_APB1EN3_M7UART11EN, .rst_bit=RCC_APB1RST3_UART11RST};
    if (periph_base == UART12_BASE)
        return (struct cline){
            .en=&RCC->APB1EN3, .rst=&RCC->APB1RST3,
            .bit=RCC_APB1EN3_M7UART12EN, .rst_bit=RCC_APB1RST3_UART12RST};

    // Timers on the APB1 bus
    if (periph_base == BTIM1_BASE)
        return (struct cline){
            .en=&RCC->APB1EN1, .rst=&RCC->APB1RST1,
            .bit=RCC_APB1EN1_M7BTIM1EN, .rst_bit=RCC_APB1RST1_BTIM1RST};
    if (periph_base == BTIM2_BASE)
        return (struct cline){
            .en=&RCC->APB1EN1, .rst=&RCC->APB1RST1,
            .bit=RCC_APB1EN1_M7BTIM2EN, .rst_bit=RCC_APB1RST1_BTIM2RST};
    if (periph_base == BTIM3_BASE)
        return (struct cline){
            .en=&RCC->APB1EN1, .rst=&RCC->APB1RST1,
            .bit=RCC_APB1EN1_M7BTIM3EN, .rst_bit=RCC_APB1RST1_BTIM3RST};
    if (periph_base == BTIM4_BASE)
        return (struct cline){
            .en=&RCC->APB1EN1, .rst=&RCC->APB1RST1,
            .bit=RCC_APB1EN1_M7BTIM4EN, .rst_bit=RCC_APB1RST1_BTIM4RST};
    if (periph_base == GTIMA4_BASE)
        return (struct cline){
            .en=&RCC->APB1EN1, .rst=&RCC->APB1RST1,
            .bit=RCC_APB1EN1_M7GTIMA4EN, .rst_bit=RCC_APB1RST1_GTIMA4RST};
    if (periph_base == GTIMA5_BASE)
        return (struct cline){
            .en=&RCC->APB1EN2, .rst=&RCC->APB1RST2,
            .bit=RCC_APB1EN2_M7GTIMA5EN, .rst_bit=RCC_APB1RST2_GTIMA5RST};
    if (periph_base == GTIMA6_BASE)
        return (struct cline){
            .en=&RCC->APB1EN2, .rst=&RCC->APB1RST2,
            .bit=RCC_APB1EN2_M7GTIMA6EN, .rst_bit=RCC_APB1RST2_GTIMA6RST};
    if (periph_base == GTIMA7_BASE)
        return (struct cline){
            .en=&RCC->APB1EN2, .rst=&RCC->APB1RST2,
            .bit=RCC_APB1EN2_M7GTIMA7EN, .rst_bit=RCC_APB1RST2_GTIMA7RST};
    if (periph_base == GTIMB1_BASE)
        return (struct cline){
            .en=&RCC->APB1EN1, .rst=&RCC->APB1RST1,
            .bit=RCC_APB1EN1_M7GTIMB1EN, .rst_bit=RCC_APB1RST1_GTIMB1RST};
    if (periph_base == GTIMB2_BASE)
        return (struct cline){
            .en=&RCC->APB1EN1, .rst=&RCC->APB1RST1,
            .bit=RCC_APB1EN1_M7GTIMB2EN, .rst_bit=RCC_APB1RST1_GTIMB2RST};
    if (periph_base == GTIMB3_BASE)
        return (struct cline){
            .en=&RCC->APB1EN1, .rst=&RCC->APB1RST1,
            .bit=RCC_APB1EN1_M7GTIMB3EN, .rst_bit=RCC_APB1RST1_GTIMB3RST};

    // SPI/I2C on the APB1 bus
    if (periph_base == SPI3_BASE)
        return (struct cline){
            .en=&RCC->APB1EN2, .rst=&RCC->APB1RST2,
            .bit=RCC_APB1EN2_M7SPI3EN, .rst_bit=RCC_APB1RST2_SPI3RST};
    if (periph_base == I2C1_BASE)
        return (struct cline){
            .en=&RCC->APB1EN4, .rst=&RCC->APB1RST4,
            .bit=RCC_APB1EN4_M7I2C1EN, .rst_bit=RCC_APB1RST4_I2C1RST};
    if (periph_base == I2C2_BASE)
        return (struct cline){
            .en=&RCC->APB1EN4, .rst=&RCC->APB1RST4,
            .bit=RCC_APB1EN4_M7I2C2EN, .rst_bit=RCC_APB1RST4_I2C2RST};
    if (periph_base == I2C3_BASE)
        return (struct cline){
            .en=&RCC->APB1EN4, .rst=&RCC->APB1RST4,
            .bit=RCC_APB1EN4_M7I2C3EN, .rst_bit=RCC_APB1RST4_I2C3RST};

    // FDCAN on the APB1 bus
    if (periph_base == FDCAN1_BASE)
        return (struct cline){
            .en=&RCC->APB1EN5, .rst=&RCC->APB1RST5,
            .bit=RCC_APB1EN5_M7FDCAN1EN, .rst_bit=RCC_APB1RST5_FDCAN1RST};
    if (periph_base == FDCAN2_BASE)
        return (struct cline){
            .en=&RCC->APB1EN5, .rst=&RCC->APB1RST5,
            .bit=RCC_APB1EN5_M7FDCAN2EN, .rst_bit=RCC_APB1RST5_FDCAN2RST};
    if (periph_base == FDCAN5_BASE)
        return (struct cline){
            .en=&RCC->APB1EN5, .rst=&RCC->APB1RST5,
            .bit=RCC_APB1EN5_M7FDCAN5EN, .rst_bit=RCC_APB1RST5_FDCAN5RST};
    if (periph_base == FDCAN6_BASE)
        return (struct cline){
            .en=&RCC->APB1EN5, .rst=&RCC->APB1RST5,
            .bit=RCC_APB1EN5_M7FDCAN6EN, .rst_bit=RCC_APB1RST5_FDCAN6RST};

    // Peripherals on the APB2 bus (150MHz)
    if (periph_base == USART5_BASE)
        return (struct cline){
            .en=&RCC->APB2EN3, .rst=&RCC->APB2RST3,
            .bit=RCC_APB2EN3_M7USART5EN, .rst_bit=RCC_APB2RST3_USART5RST};
    if (periph_base == USART6_BASE)
        return (struct cline){
            .en=&RCC->APB2EN3, .rst=&RCC->APB2RST3,
            .bit=RCC_APB2EN3_M7USART6EN, .rst_bit=RCC_APB2RST3_USART6RST};
    if (periph_base == USART7_BASE)
        return (struct cline){
            .en=&RCC->APB2EN3, .rst=&RCC->APB2RST3,
            .bit=RCC_APB2EN3_M7USART7EN, .rst_bit=RCC_APB2RST3_USART7RST};
    if (periph_base == USART8_BASE)
        return (struct cline){
            .en=&RCC->APB2EN3, .rst=&RCC->APB2RST3,
            .bit=RCC_APB2EN3_M7USART8EN, .rst_bit=RCC_APB2RST3_USART8RST};
    if (periph_base == UART13_BASE)
        return (struct cline){
            .en=&RCC->APB2EN3, .rst=&RCC->APB2RST3,
            .bit=RCC_APB2EN3_M7UART13EN, .rst_bit=RCC_APB2RST3_UART13RST};
    if (periph_base == UART14_BASE)
        return (struct cline){
            .en=&RCC->APB2EN3, .rst=&RCC->APB2RST3,
            .bit=RCC_APB2EN3_M7UART14EN, .rst_bit=RCC_APB2RST3_UART14RST};
    if (periph_base == UART15_BASE)
        return (struct cline){
            .en=&RCC->APB2EN3, .rst=&RCC->APB2RST3,
            .bit=RCC_APB2EN3_M7UART15EN, .rst_bit=RCC_APB2RST3_UART15RST};
    if (periph_base == SPI1_BASE)
        return (struct cline){
            .en=&RCC->APB2EN2, .rst=&RCC->APB2RST2,
            .bit=RCC_APB2EN2_M7SPI1EN, .rst_bit=RCC_APB2RST2_SPI1RST};
    if (periph_base == SPI2_BASE)
        return (struct cline){
            .en=&RCC->APB2EN2, .rst=&RCC->APB2RST2,
            .bit=RCC_APB2EN2_M7SPI2EN, .rst_bit=RCC_APB2RST2_SPI2RST};

    // Timers on the APB2 bus
    if (periph_base == ATIM1_BASE)
        return (struct cline){
            .en=&RCC->APB2EN1, .rst=&RCC->APB2RST1,
            .bit=RCC_APB2EN1_M7ATIM1EN, .rst_bit=RCC_APB2RST1_ATIM1RST};
    if (periph_base == ATIM2_BASE)
        return (struct cline){
            .en=&RCC->APB2EN1, .rst=&RCC->APB2RST1,
            .bit=RCC_APB2EN1_M7ATIM2EN, .rst_bit=RCC_APB2RST1_ATIM2RST};
    if (periph_base == GTIMA1_BASE)
        return (struct cline){
            .en=&RCC->APB2EN1, .rst=&RCC->APB2RST1,
            .bit=RCC_APB2EN1_M7GTIMA1EN, .rst_bit=RCC_APB2RST1_GTIMA1RST};
    if (periph_base == GTIMA2_BASE)
        return (struct cline){
            .en=&RCC->APB2EN1, .rst=&RCC->APB2RST1,
            .bit=RCC_APB2EN1_M7GTIMA2EN, .rst_bit=RCC_APB2RST1_GTIMA2RST};
    if (periph_base == GTIMA3_BASE)
        return (struct cline){
            .en=&RCC->APB2EN1, .rst=&RCC->APB2RST1,
            .bit=RCC_APB2EN1_M7GTIMA3EN, .rst_bit=RCC_APB2RST1_GTIMA3RST};

    // I2C on the APB2 bus
    if (periph_base == I2C4_BASE)
        return (struct cline){
            .en=&RCC->APB2EN2, .rst=&RCC->APB2RST2,
            .bit=RCC_APB2EN2_M7I2C4EN, .rst_bit=RCC_APB2RST2_I2C4RST};
    if (periph_base == I2C5_BASE)
        return (struct cline){
            .en=&RCC->APB2EN2, .rst=&RCC->APB2RST2,
            .bit=RCC_APB2EN2_M7I2C5EN, .rst_bit=RCC_APB2RST2_I2C5RST};
    if (periph_base == I2C6_BASE)
        return (struct cline){
            .en=&RCC->APB2EN2, .rst=&RCC->APB2RST2,
            .bit=RCC_APB2EN2_M7I2C6EN, .rst_bit=RCC_APB2RST2_I2C6RST};

    // FDCAN on the APB2 bus
    if (periph_base == FDCAN3_BASE)
        return (struct cline){
            .en=&RCC->APB2EN4, .rst=&RCC->APB2RST4,
            .bit=RCC_APB2EN4_M7FDCAN3EN, .rst_bit=RCC_APB2RST4_FDCAN3RST};
    if (periph_base == FDCAN4_BASE)
        return (struct cline){
            .en=&RCC->APB2EN4, .rst=&RCC->APB2RST4,
            .bit=RCC_APB2EN4_M7FDCAN4EN, .rst_bit=RCC_APB2RST4_FDCAN4RST};
    if (periph_base == FDCAN7_BASE)
        return (struct cline){
            .en=&RCC->APB2EN4, .rst=&RCC->APB2RST4,
            .bit=RCC_APB2EN4_M7FDCAN7EN, .rst_bit=RCC_APB2RST4_FDCAN7RST};
    if (periph_base == FDCAN8_BASE)
        return (struct cline){
            .en=&RCC->APB2EN4, .rst=&RCC->APB2RST4,
            .bit=RCC_APB2EN4_M7FDCAN8EN, .rst_bit=RCC_APB2RST4_FDCAN8RST};

    // Peripherals on the APB5 bus (150MHz)
    if (periph_base == ATIM3_BASE)
        return (struct cline){
            .en=&RCC->APB5EN1, .rst=&RCC->APB5RST1,
            .bit=RCC_APB5EN1_M7ATIM3EN, .rst_bit=RCC_APB5RST1_ATIM3RST};
    if (periph_base == ATIM4_BASE)
        return (struct cline){
            .en=&RCC->APB5EN1, .rst=&RCC->APB5RST1,
            .bit=RCC_APB5EN1_M7ATIM4EN, .rst_bit=RCC_APB5RST1_ATIM4RST};
    if (periph_base == SPI4_BASE)
        return (struct cline){
            .en=&RCC->APB5EN1, .rst=&RCC->APB5RST1,
            .bit=RCC_APB5EN1_M7SPI4EN, .rst_bit=RCC_APB5RST1_SPI4RST};
    if (periph_base == SPI5_BASE)
        return (struct cline){
            .en=&RCC->APB5EN1, .rst=&RCC->APB5RST1,
            .bit=RCC_APB5EN1_M7SPI5EN, .rst_bit=RCC_APB5RST1_SPI5RST};
    if (periph_base == SPI6_BASE)
        return (struct cline){
            .en=&RCC->APB5EN1, .rst=&RCC->APB5RST1,
            .bit=RCC_APB5EN1_M7SPI6EN, .rst_bit=RCC_APB5RST1_SPI6RST};
    if (periph_base == SPI7_BASE)
        return (struct cline){
            .en=&RCC->APB5EN1, .rst=&RCC->APB5RST1,
            .bit=RCC_APB5EN1_M7SPI7EN, .rst_bit=RCC_APB5RST1_SPI7RST};
    if (periph_base == I2C7_BASE)
        return (struct cline){
            .en=&RCC->APB5EN2, .rst=&RCC->APB5RST2,
            .bit=RCC_APB5EN2_M7I2C7EN, .rst_bit=RCC_APB5RST2_I2C7RST};
    if (periph_base == I2C8_BASE)
        return (struct cline){
            .en=&RCC->APB5EN2, .rst=&RCC->APB5RST2,
            .bit=RCC_APB5EN2_M7I2C8EN, .rst_bit=RCC_APB5RST2_I2C8RST};
    if (periph_base == I2C9_BASE)
        return (struct cline){
            .en=&RCC->APB5EN2, .rst=&RCC->APB5RST2,
            .bit=RCC_APB5EN2_M7I2C9EN, .rst_bit=RCC_APB5RST2_I2C9RST};
    if (periph_base == I2C10_BASE)
        return (struct cline){
            .en=&RCC->APB5EN2, .rst=&RCC->APB5RST2,
            .bit=RCC_APB5EN2_M7I2C10EN, .rst_bit=RCC_APB5RST2_I2C10RST};

    struct cline empty = {0};
    return empty;
}

// Return the frequency of a peripheral's kernel clock.
//
// With the 600MHz mode-0 configuration:
//   sys_clk=M7=600MHz, sys_bus_div_clk=AHB1=300MHz (BUSDIV=2),
//   APB1/APB2/APB5=150MHz (APBxDIV=2).
// USART/SPI/I2C/TIM/FDCAN default to their APB bus clock (150MHz);
// ADC/DMA/SDMMC/USB/ETH sit on the AHB bus (300MHz).
//
// The bus is determined by the enable register that lookup_clock_line()
// returns; an address-range test is ambiguous because the USB1 and ETH1
// clock enable bits live in the AHB2 registers while USB_CTRL1 sits in
// the AHB1 address region (0x40050000).
uint32_t
get_pclock_frequency(uint32_t periph_base)
{
    // USART1/USART2 kernel clocks come from HCLK (AHB1) through the
    // RCC_APB1DIV1.APB1USARTDIV prescaler (reset value /1), unlike
    // USART3-8 which use their APB bus clock directly.
    if (periph_base == USART1_BASE || periph_base == USART2_BASE)
        return CONFIG_CLOCK_FREQ / 2;   // AHB1 clock (300MHz)

    struct cline cl = lookup_clock_line(periph_base);
    // Enable registers of the AHB buses run at 300MHz
    if (cl.en == &RCC->AHB1EN1 || cl.en == &RCC->AHB1EN2
        || cl.en == &RCC->AHB1EN3 || cl.en == &RCC->AHB1EN4
        || cl.en == &RCC->AHB2EN1 || cl.en == &RCC->AHB2EN2
        || cl.en == &RCC->AHB5EN2)
        return CONFIG_CLOCK_FREQ / 2;
    // APB1/APB2/APB5 peripherals: the APBxDIV prescalers are set to /1
    // (APB = sysclk/2) for sysclk <= 300MHz and to /2 (APB = sysclk/4)
    // otherwise, matching the divider setup in clock_setup_to_pll1().
    if (CONFIG_CLOCK_FREQ > 300000000)
        return CONFIG_CLOCK_FREQ / 4;
    return CONFIG_CLOCK_FREQ / 2;
}
