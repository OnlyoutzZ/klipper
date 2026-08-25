// Serial port support for N32H7
//
// Copyright (C) 2026  Nsing
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "autoconf.h" // CONFIG_SERIAL_BAUD
#include "board/armcm_boot.h" // armcm_enable_irq
#include "board/serial_irq.h" // serial_rx_byte
#include "command.h" // DECL_CONSTANT_STR
#include "internal.h" // enable_pclock
#include "sched.h" // DECL_INIT

// Select the configured serial port
#if CONFIG_N32H7_SERIAL_USART1
  DECL_CONSTANT_STR("RESERVE_PINS_serial", "PA10,PA9");
  #define GPIO_Rx GPIO('A', 10)
  #define GPIO_Tx GPIO('A', 9)
  #define GPIO_RX_FUNCTION 5 // USART1_RX on PA10
  #define GPIO_TX_FUNCTION 7 // USART1_TX on PA9
  #define USARTx USART1
  #define USARTx_IRQn USART1_IRQn
  #define USARTx_IRQHandler USART1_IRQHandler
#elif CONFIG_N32H7_SERIAL_USART1_ALT_PB7_PB6
  DECL_CONSTANT_STR("RESERVE_PINS_serial", "PB7,PB6");
  #define GPIO_Rx GPIO('B', 7)
  #define GPIO_Tx GPIO('B', 6)
  #define GPIO_RX_FUNCTION 8 // USART1_RX on PB7
  #define GPIO_TX_FUNCTION 7 // USART1_TX on PB6
  #define USARTx USART1
  #define USARTx_IRQn USART1_IRQn
  #define USARTx_IRQHandler USART1_IRQHandler
#elif CONFIG_N32H7_SERIAL_USART1_ALT_PF1_PF0
  DECL_CONSTANT_STR("RESERVE_PINS_serial", "PF1,PF0");
  #define GPIO_Rx GPIO('F', 1)
  #define GPIO_Tx GPIO('F', 0)
  #define GPIO_RX_FUNCTION 9 // USART1_RX on PF1
  #define GPIO_TX_FUNCTION 8 // USART1_TX on PF0
  #define USARTx USART1
  #define USARTx_IRQn USART1_IRQn
  #define USARTx_IRQHandler USART1_IRQHandler
#elif CONFIG_N32H7_SERIAL_USART2
  DECL_CONSTANT_STR("RESERVE_PINS_serial", "PA3,PA2");
  #define GPIO_Rx GPIO('A', 3)
  #define GPIO_Tx GPIO('A', 2)
  #define GPIO_RX_FUNCTION 11 // USART2_RX on PA3
  #define GPIO_TX_FUNCTION 7 // USART2_TX on PA2
  #define USARTx USART2
  #define USARTx_IRQn USART2_IRQn
  #define USARTx_IRQHandler USART2_IRQHandler
#elif CONFIG_N32H7_SERIAL_USART2_ALT_PD6_PD5
  DECL_CONSTANT_STR("RESERVE_PINS_serial", "PD6,PD5");
  #define GPIO_Rx GPIO('D', 6)
  #define GPIO_Tx GPIO('D', 5)
  #define GPIO_RX_FUNCTION 7 // USART2_RX on PD6
  #define GPIO_TX_FUNCTION 6 // USART2_TX on PD5
  #define USARTx USART2
  #define USARTx_IRQn USART2_IRQn
  #define USARTx_IRQHandler USART2_IRQHandler
#elif CONFIG_N32H7_SERIAL_USART3
  DECL_CONSTANT_STR("RESERVE_PINS_serial", "PB11,PB10");
  #define GPIO_Rx GPIO('B', 11)
  #define GPIO_Tx GPIO('B', 10)
  #define GPIO_RX_FUNCTION 6 // USART3_RX on PB11
  #define GPIO_TX_FUNCTION 9 // USART3_TX on PB10
  #define USARTx USART3
  #define USARTx_IRQn USART3_IRQn
  #define USARTx_IRQHandler USART3_IRQHandler
#elif CONFIG_N32H7_SERIAL_USART3_ALT_PD9_PD8
  DECL_CONSTANT_STR("RESERVE_PINS_serial", "PD9,PD8");
  #define GPIO_Rx GPIO('D', 9)
  #define GPIO_Tx GPIO('D', 8)
  #define GPIO_RX_FUNCTION 5 // USART3_RX on PD9
  #define GPIO_TX_FUNCTION 6 // USART3_TX on PD8
  #define USARTx USART3
  #define USARTx_IRQn USART3_IRQn
  #define USARTx_IRQHandler USART3_IRQHandler
#elif CONFIG_N32H7_SERIAL_USART3_ALT_PC11_PC10
  DECL_CONSTANT_STR("RESERVE_PINS_serial", "PC11,PC10");
  #define GPIO_Rx GPIO('C', 11)
  #define GPIO_Tx GPIO('C', 10)
  #define GPIO_RX_FUNCTION 7 // USART3_RX on PC11
  #define GPIO_TX_FUNCTION 7 // USART3_TX on PC10
  #define USARTx USART3
  #define USARTx_IRQn USART3_IRQn
  #define USARTx_IRQHandler USART3_IRQHandler
#elif CONFIG_N32H7_SERIAL_USART4
  DECL_CONSTANT_STR("RESERVE_PINS_serial", "PC7,PC6");
  #define GPIO_Rx GPIO('C', 7)
  #define GPIO_Tx GPIO('C', 6)
  #define GPIO_RX_FUNCTION 10 // USART4_RX on PC7
  #define GPIO_TX_FUNCTION 10 // USART4_TX on PC6
  #define USARTx USART4
  #define USARTx_IRQn USART4_IRQn
  #define USARTx_IRQHandler USART4_IRQHandler
#elif CONFIG_N32H7_SERIAL_USART5
  DECL_CONSTANT_STR("RESERVE_PINS_serial", "PE2,PE3");
  #define GPIO_Rx GPIO('E', 2)
  #define GPIO_Tx GPIO('E', 3)
  #define GPIO_RX_FUNCTION 6 // USART5_RX on PE2
  #define GPIO_TX_FUNCTION 4 // USART5_TX on PE3
  #define USARTx USART5
  #define USARTx_IRQn USART5_IRQn
  #define USARTx_IRQHandler USART5_IRQHandler
#elif CONFIG_N32H7_SERIAL_USART6
  DECL_CONSTANT_STR("RESERVE_PINS_serial", "PE4,PE5");
  #define GPIO_Rx GPIO('E', 4)
  #define GPIO_Tx GPIO('E', 5)
  #define GPIO_RX_FUNCTION 6 // USART6_RX on PE4
  #define GPIO_TX_FUNCTION 6 // USART6_TX on PE5
  #define USARTx USART6
  #define USARTx_IRQn USART6_IRQn
  #define USARTx_IRQHandler USART6_IRQHandler
#elif CONFIG_N32H7_SERIAL_USART6_ALT_PH14_PH15
  DECL_CONSTANT_STR("RESERVE_PINS_serial", "PH14,PH15");
  #define GPIO_Rx GPIO('H', 14)
  #define GPIO_Tx GPIO('H', 15)
  #define GPIO_RX_FUNCTION 7 // USART6_RX on PH14
  #define GPIO_TX_FUNCTION 8 // USART6_TX on PH15
  #define USARTx USART6
  #define USARTx_IRQn USART6_IRQn
  #define USARTx_IRQHandler USART6_IRQHandler
#elif CONFIG_N32H7_SERIAL_USART7
  DECL_CONSTANT_STR("RESERVE_PINS_serial", "PH4,PH5");
  #define GPIO_Rx GPIO('H', 4)
  #define GPIO_Tx GPIO('H', 5)
  #define GPIO_RX_FUNCTION 6 // USART7_RX on PH4
  #define GPIO_TX_FUNCTION 6 // USART7_TX on PH5
  #define USARTx USART7
  #define USARTx_IRQn USART7_IRQn
  #define USARTx_IRQHandler USART7_IRQHandler
#elif CONFIG_N32H7_SERIAL_USART7_ALT_PF14_PF15
  DECL_CONSTANT_STR("RESERVE_PINS_serial", "PF14,PF15");
  #define GPIO_Rx GPIO('F', 14)
  #define GPIO_Tx GPIO('F', 15)
  #define GPIO_RX_FUNCTION 6 // USART7_RX on PF14
  #define GPIO_TX_FUNCTION 7 // USART7_TX on PF15
  #define USARTx USART7
  #define USARTx_IRQn USART7_IRQn
  #define USARTx_IRQHandler USART7_IRQHandler
#elif CONFIG_N32H7_SERIAL_USART8
  DECL_CONSTANT_STR("RESERVE_PINS_serial", "PI2,PI3");
  #define GPIO_Rx GPIO('I', 2)
  #define GPIO_Tx GPIO('I', 3)
  #define GPIO_RX_FUNCTION 8 // USART8_RX on PI2
  #define GPIO_TX_FUNCTION 8 // USART8_TX on PI3
  #define USARTx USART8
  #define USARTx_IRQn USART8_IRQn
  #define USARTx_IRQHandler USART8_IRQHandler
#elif CONFIG_N32H7_SERIAL_USART8_ALT_PI4_PI5
  DECL_CONSTANT_STR("RESERVE_PINS_serial", "PI4,PI5");
  #define GPIO_Rx GPIO('I', 4)
  #define GPIO_Tx GPIO('I', 5)
  #define GPIO_RX_FUNCTION 8 // USART8_RX on PI4
  #define GPIO_TX_FUNCTION 9 // USART8_TX on PI5
  #define USARTx USART8
  #define USARTx_IRQn USART8_IRQn
  #define USARTx_IRQHandler USART8_IRQHandler
#endif

#define CR1_FLAGS (USART_CTRL1_UEN | USART_CTRL1_RXEN | USART_CTRL1_TXEN \
                   | USART_CTRL1_RXDNEIEN)

void
USARTx_IRQHandler(void)
{
    uint32_t sr = USARTx->STS;
    // OREF and RXDNE are both cleared by reading STS followed by DAT.
    // While OREF is set the DAT register stops updating, so it must be
    // cleared or reception deadlocks.
    if (sr & (USART_STS_RXDNE | USART_STS_OREF))
        serial_rx_byte(USARTx->DAT);
    if (sr & USART_STS_TXDE && USARTx->CTRL1 & USART_CTRL1_TXDEIEN) {
        uint8_t data;
        int ret = serial_get_tx_byte(&data);
        if (ret)
            USARTx->CTRL1 = CR1_FLAGS;
        else
            USARTx->DAT = data;
    }
}

void
serial_enable_tx_irq(void)
{
    USARTx->CTRL1 = CR1_FLAGS | USART_CTRL1_TXDEIEN;
}

void
serial_init(void)
{
    enable_pclock((uint32_t)USARTx);

    uint32_t pclk = get_pclock_frequency((uint32_t)USARTx);
    uint32_t div = DIV_ROUND_CLOSEST(pclk, CONFIG_SERIAL_BAUD);
    USARTx->BRCF = (((div / 16) << 4) | (div % 16));
    USARTx->CTRL2 = 0;
    USARTx->CTRL3 = 0;
    USARTx->CTRL1 = CR1_FLAGS;
    armcm_enable_irq(USARTx_IRQHandler, USARTx_IRQn, 0);

    gpio_peripheral(GPIO_Rx, GPIO_FUNCTION(GPIO_RX_FUNCTION), GPIO_PULL_UP);
    gpio_peripheral(GPIO_Tx, GPIO_FUNCTION(GPIO_TX_FUNCTION), GPIO_PULL_NONE);
}
DECL_INIT(serial_init);
