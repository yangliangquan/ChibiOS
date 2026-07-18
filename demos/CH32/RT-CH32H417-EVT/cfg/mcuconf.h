/*
    ChibiOS - Copyright (C) 2006-2026 Giovanni Di Sirio.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef MCUCONF_H
#define MCUCONF_H

/*
 * Platform drivers configuration.
 * The following settings override the default settings present in
 * the various device driver implementation headers.
 * Note that the settings for each driver only have effect if the whole
 * driver is enabled in halconf.h.
 */

#define CH32H41x_MCUCONF
#define CH32H417_MCUCONF

#include "ch32h417.h"

/*
 * HAL driver system settings.
 */
#define CH32_NO_INIT                       FALSE
#define CH32_PVD_ENABLE                    FALSE
#define CH32_HSI_ENABLED                   TRUE
#define CH32_LSI_ENABLED                   TRUE
#define CH32_HSE_ENABLED                   TRUE
#define CH32_LSE_ENABLED                   FALSE

#define CH32_SW                            RCC_SW_PLL
#define CH32_SYSPLL                        RCC_SYSPLL_PLL
#define CH32_PLLSRC                        RCC_PLLSRC_HSE
#define CH32_PLL_SRC_DIV                   RCC_PLL_SRC_DIV1
#define CH32_PLL_MUL                       RCC_PLLMUL16

#define CH32_HPRE                          RCC_HPRE_DIV1
#define CH32_HPRF                          RCC_FPRE_DIV4

/*
 * IRQ system settings.
 */

/*
 * ADC driver system settings.
 */
#define CH32_ADC_PRE2_DIV                  RCC_PPRE2_ADCL_DIV16
#define CH32_ADC_ADCPRE                    RCC_ADCPRE_ADCH_DIV8
#define CH32_ADC_USE_ADC1                  TRUE
#define CH32_ADC_USE_ADC2                  TRUE

/*
 * CAN driver system settings.
 */
#define CH32_CAN_USE_CAN1                  TRUE
#define CH32_CAN_USE_CAN2                  FALSE
#define CH32_CAN_USE_CAN3                  FALSE
#define CH32_CAN_CAN1_IRQ_PRIORITY         11
#define CH32_CAN_CAN2_IRQ_PRIORITY         11
#define CH32_CAN_CAN3_IRQ_PRIORITY         11

/*
 * DAC driver system settings.
 */
#define CH32_DAC_USE_DAC1_CH1              TRUE
#define CH32_DAC_USE_DAC1_CH2              FALSE

/*
 * GPT driver system settings.
 */
#define CH32_GPT_USE_TIM1                  FALSE
#define CH32_GPT_USE_TIM2                  TRUE
#define CH32_GPT_USE_TIM3                  FALSE
#define CH32_GPT_USE_TIM4                  FALSE
#define CH32_GPT_USE_TIM5                  TRUE
#define CH32_GPT_USE_TIM6                  FALSE
#define CH32_GPT_USE_TIM7                  FALSE
#define CH32_GPT_USE_TIM8                  FALSE
#define CH32_GPT_USE_TIM9                  FALSE
#define CH32_GPT_USE_TIM10                 FALSE
#define CH32_GPT_USE_TIM11                 FALSE
#define CH32_GPT_USE_TIM12                 FALSE
#define CH32_GPT_USE_TIM13                 FALSE
#define CH32_GPT_USE_TIM14                 FALSE

/*
 * PWM driver system settings.
 */
#define CH32_PWM_USE_TIM1                  FALSE
#define CH32_PWM_USE_TIM2                  FALSE
#define CH32_PWM_USE_TIM3                  TRUE
#define CH32_PWM_USE_TIM4                  FALSE
#define CH32_PWM_USE_TIM5                  FALSE
#define CH32_PWM_USE_TIM6                  FALSE
#define CH32_PWM_USE_TIM7                  FALSE
#define CH32_PWM_USE_TIM8                  FALSE
#define CH32_PWM_USE_TIM9                  FALSE
#define CH32_PWM_USE_TIM10                 FALSE
#define CH32_PWM_USE_TIM11                 FALSE
#define CH32_PWM_USE_TIM12                 FALSE
#define CH32_PWM_USE_TIM13                 FALSE
#define CH32_PWM_USE_TIM14                 FALSE

/*
 * ICU driver system settings.
 */
#define CH32_ICU_USE_TIM1                  FALSE
#define CH32_ICU_USE_TIM2                  FALSE
#define CH32_ICU_USE_TIM3                  FALSE
#define CH32_ICU_USE_TIM4                  TRUE
#define CH32_ICU_USE_TIM5                  FALSE
#define CH32_ICU_USE_TIM6                  FALSE
#define CH32_ICU_USE_TIM7                  FALSE
#define CH32_ICU_USE_TIM8                  FALSE
#define CH32_ICU_USE_TIM9                  FALSE
#define CH32_ICU_USE_TIM10                 FALSE
#define CH32_ICU_USE_TIM11                 FALSE
#define CH32_ICU_USE_TIM12                 FALSE
#define CH32_ICU_USE_TIM13                 FALSE
#define CH32_ICU_USE_TIM14                 FALSE

/*
 * I2C driver system settings.
 */
#define CH32_I2C_USE_I2C1                  TRUE
#define CH32_I2C_USE_I2C2                  FALSE
#define CH32_I2C_USE_I2C3                  FALSE
#define CH32_I2C_USE_I2C4                  FALSE

/*
 * I2C clock frequency (Hz). Must match the HB1 bus clock frequency
 * driving the I2C peripheral. The FREQ field in CTLR2 is limited to
 * 6 bits (max 63), so this value must be <= 63000000 for correct
 * I2C operation.
 */
#define CH32_I2C_CLK_FREQ                 96000000

/*
 * I2C DMA stream settings.
 */
#define CH32_I2C_I2C1_RX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#define CH32_I2C_I2C1_TX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#define CH32_I2C_I2C2_RX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#define CH32_I2C_I2C2_TX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#define CH32_I2C_I2C3_RX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#define CH32_I2C_I2C3_TX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#define CH32_I2C_I2C4_RX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#define CH32_I2C_I2C4_TX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY

/*
 * I2C DMA priority settings.
 */
#define CH32_I2C_I2C1_DMA_PRIORITY         1
#define CH32_I2C_I2C2_DMA_PRIORITY         1
#define CH32_I2C_I2C3_DMA_PRIORITY         1
#define CH32_I2C_I2C4_DMA_PRIORITY         1

/*
 * I2C IRQ priority settings.
 */
#define CH32_I2C_I2C1_IRQ_PRIORITY         10
#define CH32_I2C_I2C2_IRQ_PRIORITY         10
#define CH32_I2C_I2C3_IRQ_PRIORITY         10
#define CH32_I2C_I2C4_IRQ_PRIORITY         10

/*
 * I2C DMA error hook.
 */
#define CH32_I2C_DMA_ERROR_HOOK(i2cp)      osalSysHalt("DMA failure")

/*
 * SPI driver system settings.
 */
#define CH32_SPI_USE_SPI1                  TRUE
#define CH32_SPI_USE_SPI2                  FALSE
#define CH32_SPI_USE_SPI3                  FALSE
#define CH32_SPI_USE_SPI4                  FALSE

/*
 * SPI DMA stream settings.
 */
#define CH32_SPI_SPI1_RX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#define CH32_SPI_SPI1_TX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#define CH32_SPI_SPI2_RX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#define CH32_SPI_SPI2_TX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#define CH32_SPI_SPI3_RX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#define CH32_SPI_SPI3_TX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#define CH32_SPI_SPI4_RX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#define CH32_SPI_SPI4_TX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY

/*
 * SPI DMA priority settings.
 */
#define CH32_SPI_SPI1_DMA_PRIORITY         3
#define CH32_SPI_SPI2_DMA_PRIORITY         3
#define CH32_SPI_SPI3_DMA_PRIORITY         3
#define CH32_SPI_SPI4_DMA_PRIORITY         3

/*
 * SPI IRQ priority settings.
 */
#define CH32_SPI_SPI1_IRQ_PRIORITY         10
#define CH32_SPI_SPI2_IRQ_PRIORITY         10
#define CH32_SPI_SPI3_IRQ_PRIORITY         10
#define CH32_SPI_SPI4_IRQ_PRIORITY         10


/*
 * I2S driver system settings.
 */
#define CH32_I2S_USE_I2S1                  FALSE
#define CH32_I2S_USE_I2S2                  TRUE
#define CH32_I2S_SPI1_MODE                 (CH32_I2S_MODE_MASTER |          \
                                            CH32_I2S_MODE_TX)
#define CH32_I2S_SPI2_MODE                 (CH32_I2S_MODE_MASTER |          \
                                            CH32_I2S_MODE_TX)
#define CH32_I2S_SPI1_IRQ_PRIORITY         1
#define CH32_I2S_SPI2_IRQ_PRIORITY         1
#define CH32_I2S_SPI1_DMA_PRIORITY         1
#define CH32_I2S_SPI2_DMA_PRIORITY         1
#define CH32_I2S_SPI1_RX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#define CH32_I2S_SPI1_TX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#define CH32_I2S_SPI2_RX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#define CH32_I2S_SPI2_TX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#define CH32_I2S_DMA_ERROR_HOOK(i2sp)      osalSysHalt("DMA failure")

/*
 * RTC driver system settings.
 */
#define CH32_RTC_USE_RTC1                   TRUE


/*
 * SIO driver system settings.
 */
#define CH32_SIO_USE_USART1             FALSE
#define CH32_SIO_USE_USART2             TRUE
#define CH32_SIO_USE_USART3             FALSE
#define CH32_SIO_USE_USART4             FALSE
#define CH32_SIO_USE_USART5             FALSE
#define CH32_SIO_USE_USART6             FALSE
#define CH32_SIO_USE_USART7             FALSE
#define CH32_SIO_USE_USART8             FALSE

/*
 * SERIAL driver system settings.
 */
#define CH32_SERIAL_USE_USART1             TRUE
#define CH32_SERIAL_USE_USART2             FALSE
#define CH32_SERIAL_USE_USART3             FALSE
#define CH32_SERIAL_USE_USART4             FALSE
#define CH32_SERIAL_USE_USART5             FALSE
#define CH32_SERIAL_USE_USART6             FALSE
#define CH32_SERIAL_USE_USART7             FALSE
#define CH32_SERIAL_USE_USART8             FALSE

/*
 * ST driver system settings.
 */

/*
 * UART driver system settings.
 */
#define CH32_UART_USE_USART1             FALSE
#define CH32_UART_USE_USART2             FALSE
#define CH32_UART_USE_USART3             TRUE
#define CH32_UART_USE_USART4             FALSE
#define CH32_UART_USE_USART5             FALSE
#define CH32_UART_USE_USART6             FALSE
#define CH32_UART_USE_USART7             FALSE
#define CH32_UART_USE_USART8             FALSE

/*
 * CRY (crypto) driver system settings.
 */
#define CH32_CRY_USE_CRY1                  TRUE

/*
 * SDC driver system settings.
 */
#define CH32_SDC_USE_SDC1                  TRUE
#define CH32_SDC_USE_SDIO                  FALSE
#define CH32_SDC_SDMMC_CLOCK               600000000
#define CH32_SDC_SDIO_CLOCK                96000000
#define CH32_SDC_SDMMC_IRQ_PRIORITY        9
#define CH32_SDC_SDIO_IRQ_PRIORITY         9

/*
 * USB (OTG) driver system settings.
 */
#define CH32_OTG_USE_USB1                  TRUE
#define CH32_USB_OTG1_IRQ_PRIORITY         14

#endif /* MCUCONF_H */
