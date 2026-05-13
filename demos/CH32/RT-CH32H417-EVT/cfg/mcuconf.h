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
#define CH32_ADC_USE_ADC1                  FALSE
#define CH32_ADC_USE_ADC2                  FALSE
#define CH32_ADC_ADC1_DMA_ID               120
#define CH32_ADC_ADC2_DMA_ID               121
#define CH32_ADC_ADC1_DMA_PRIORITY         2
#define CH32_ADC_ADC2_DMA_PRIORITY         2

/*
 * CAN driver system settings.
 */
#define CH32_CAN_USE_CAN1                  FALSE
#define CH32_CAN_USE_CAN2                  FALSE

/*
 * DAC driver system settings.
 */
#define CH32_DAC_USE_DAC1_CH1              FALSE
#define CH32_DAC_USE_DAC1_CH2              FALSE

/*
 * GPT driver system settings.
 */
#define CH32_GPT_USE_TIM1                  FALSE
#define CH32_GPT_USE_TIM2                  FALSE
#define CH32_GPT_USE_TIM3                  FALSE
#define CH32_GPT_USE_TIM4                  FALSE
#define CH32_GPT_USE_TIM5                  FALSE
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
 * I2C driver system settings.
 */
#define CH32_I2C_USE_I2C1                  FALSE
#define CH32_I2C_USE_I2C2                  FALSE
#define CH32_I2C_USE_I2C3                  FALSE
#define CH32_I2C_USE_I2C4                  FALSE

/*
 * SPI driver system settings.
 */
#define STM32_SPI_USE_SPI1                  FALSE
#define STM32_SPI_USE_SPI2                  FALSE
#define STM32_SPI_USE_SPI3                  FALSE
#define STM32_SPI_USE_SPI4                  FALSE

/*
 * ICU driver system settings.
 */
#define STM32_ICU_USE_TIM1                  FALSE
#define STM32_ICU_USE_TIM2                  FALSE
#define STM32_ICU_USE_TIM3                  FALSE
#define STM32_ICU_USE_TIM4                  FALSE
#define STM32_ICU_USE_TIM5                  FALSE
#define STM32_ICU_USE_TIM8                  FALSE
#define STM32_ICU_USE_TIM9                  FALSE
#define STM32_ICU_USE_TIM10                 FALSE
#define STM32_ICU_USE_TIM11                 FALSE
#define STM32_ICU_USE_TIM12                 FALSE
#define STM32_ICU_USE_TIM13                 FALSE
#define STM32_ICU_USE_TIM14                 FALSE

/*
 * PWM driver system settings.
 */
#define STM32_PWM_USE_TIM1                  FALSE
#define STM32_PWM_USE_TIM2                  FALSE
#define STM32_PWM_USE_TIM3                  FALSE
#define STM32_PWM_USE_TIM4                  FALSE
#define STM32_PWM_USE_TIM5                  FALSE
#define STM32_PWM_USE_TIM8                  FALSE
#define STM32_PWM_USE_TIM9                  FALSE
#define STM32_PWM_USE_TIM10                 FALSE
#define STM32_PWM_USE_TIM11                 FALSE
#define STM32_PWM_USE_TIM12                 FALSE
#define STM32_PWM_USE_TIM13                 FALSE
#define STM32_PWM_USE_TIM14                 FALSE

/*
 * RTC driver system settings.
 */
#define STM32_PWM_USE_RTC                   FALSE

/*
 * SERIAL driver system settings.
 */
#define STM32_SERIAL_USE_USART1             FALSE
#define STM32_SERIAL_USE_USART2             TRUE
#define STM32_SERIAL_USE_USART3             FALSE
#define STM32_SERIAL_USE_USART4             FALSE
#define STM32_SERIAL_USE_USART5             FALSE
#define STM32_SERIAL_USE_USART6             FALSE
#define STM32_SERIAL_USE_USART7             FALSE
#define STM32_SERIAL_USE_USART8             FALSE

/*
 * ST driver system settings.
 */

/*
 * UART driver system settings.
 */
#define STM32_UART_USE_USART1             FALSE
#define STM32_UART_USE_USART2             TRUE
#define STM32_UART_USE_USART3             FALSE
#define STM32_UART_USE_USART4             FALSE
#define STM32_UART_USE_USART5             FALSE
#define STM32_UART_USE_USART6             FALSE
#define STM32_UART_USE_USART7             FALSE
#define STM32_UART_USE_USART8             FALSE

#endif /* MCUCONF_H */
