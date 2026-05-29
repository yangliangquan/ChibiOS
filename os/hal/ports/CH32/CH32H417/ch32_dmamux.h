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

/**
 * @file    CH32H417/ch32_dmamux.h
 * @brief   CH32H417 DMAMUX driver header.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef CH32_DMAMUX_H
#define CH32_DMAMUX_H

/*===========================================================================*/
/* DMAMUX channel assignment enumerations.                                   */
/*===========================================================================*/

/**
 * @name    DMAMUX request source identifiers
 * @{
 */
typedef enum {
  DMA_MUX_TIM1_CH1 = 1,
  DMA_MUX_TIM1_CH2,
  DMA_MUX_TIM1_CH3,
  DMA_MUX_TIM1_CH4,
  DMA_MUX_TIM1_UP,
  DMA_MUX_TIM1_COM,
  DMA_MUX_TIM1_TRIG,
  DMA_MUX_TIM2_CH1,
  DMA_MUX_TIM2_CH2,
  DMA_MUX_TIM2_CH3,
  DMA_MUX_TIM2_CH4,
  DMA_MUX_TIM2_UP,
  DMA_MUX_TIM2_TRIG,
  DMA_MUX_TIM3_CH1,
  DMA_MUX_TIM3_CH2,
  DMA_MUX_TIM3_CH3,
  DMA_MUX_TIM3_CH4,
  DMA_MUX_TIM3_UP,
  DMA_MUX_TIM3_TRIG,
  DMA_MUX_TIM4_CH1,
  DMA_MUX_TIM4_CH2,
  DMA_MUX_TIM4_CH3,
  DMA_MUX_TIM4_CH4,
  DMA_MUX_TIM4_UP,
  DMA_MUX_TIM4_TRIG,
  DMA_MUX_TIM5_CH1,
  DMA_MUX_TIM5_CH2,
  DMA_MUX_TIM5_CH3,
  DMA_MUX_TIM5_CH4,
  DMA_MUX_TIM5_UP,
  DMA_MUX_TIM5_TRIG,
  DMA_MUX_TIM8_CH1,
  DMA_MUX_TIM8_CH2,
  DMA_MUX_TIM8_CH3,
  DMA_MUX_TIM8_CH4,
  DMA_MUX_TIM8_UP,
  DMA_MUX_TIM8_COM,
  DMA_MUX_TIM8_TRIG,
  DMA_MUX_TIM9_CH1,
  DMA_MUX_TIM9_CH2,
  DMA_MUX_TIM9_CH3,
  DMA_MUX_TIM9_CH4,
  DMA_MUX_TIM9_UP,
  DMA_MUX_TIM9_TRIG,
  DMA_MUX_TIM10_CH1,
  DMA_MUX_TIM10_CH2,
  DMA_MUX_TIM10_CH3,
  DMA_MUX_TIM10_CH4,
  DMA_MUX_TIM10_UP,
  DMA_MUX_TIM10_TRIG,
  DMA_MUX_TIM11_CH1,
  DMA_MUX_TIM11_CH2,
  DMA_MUX_TIM11_CH3,
  DMA_MUX_TIM11_CH4,
  DMA_MUX_TIM11_UP,
  DMA_MUX_TIM11_TRIG,
  DMA_MUX_TIM12_CH1,
  DMA_MUX_TIM12_CH2,
  DMA_MUX_TIM12_CH3,
  DMA_MUX_TIM12_CH4,
  DMA_MUX_TIM12_UP,
  DMA_MUX_TIM12_TRIG,
  DMA_MUX_SPI1_TX,
  DMA_MUX_SPI1_RX,
  DMA_MUX_SPI2_TX,
  DMA_MUX_SPI2_RX,
  DMA_MUX_SPI3_TX,
  DMA_MUX_SPI3_RX,
  DMA_MUX_SPI4_TX,
  DMA_MUX_SPI4_RX,
  DMA_MUX_QSPI1_DMA,
  DMA_MUX_QSPI2_DMA,
  DMA_MUX_I2C1_TX,
  DMA_MUX_I2C1_RX,
  DMA_MUX_I2C2_TX,
  DMA_MUX_I2C2_RX,
  DMA_MUX_I2C3_TX,
  DMA_MUX_I2C3_RX,
  DMA_MUX_I2C4_TX,
  DMA_MUX_I2C4_RX,
  DMA_MUX_I3C_RS,
  DMA_MUX_I3C_TC,
  DMA_MUX_I3C_TX,
  DMA_MUX_I3X_RX,
  DMA_MUX_USART1_TX,
  DMA_MUX_USART1_RX,
  DMA_MUX_USART2_TX,
  DMA_MUX_USART2_RX,
  DMA_MUX_USART3_TX,
  DMA_MUX_USART3_RX,
  DMA_MUX_USART4_TX,
  DMA_MUX_USART4_RX,
  DMA_MUX_USART5_TX,
  DMA_MUX_USART5_RX,
  DMA_MUX_USART6_TX,
  DMA_MUX_USART6_RX,
  DMA_MUX_USART7_TX,
  DMA_MUX_USART7_RX,
  DMA_MUX_USART8_TX,
  DMA_MUX_USART8_RX,
  DMA_MUX_SWPMI_TX,
  DMA_MUX_SWPMI_RX,
  DMA_MUX_DAC1,
  DMA_MUX_DAC2,
  DMA_MUX_RS0,
  DMA_MUX_RS1,
  DMA_MUX_DFSDM_DMA0,
  DMA_MUX_DFSDM_DMA1,
  DMA_MUX_RS2,
  DMA_MUX_RS3,
  DMA_MUX_SDIO,
  DMA_MUX_SAI_A_TX,
  DMA_MUX_SAI_A_RX,
  DMA_MUX_SAI_B_TX,
  DMA_MUX_SAI_B_RX,
  DMA_MUX_RS4,
  DMA_MUX_RS5,
  DMA_MUX_RS6,
  DMA_MUX_RS7,
  DMA_MUX_ADC1,
  DMA_MUX_ADC2,
  DMA_MUX_TIM6_UP,
  DMA_MUX_TIM7_UP
} dma_mux_index_e;
/** @} */

#endif /* CH32_DMAMUX_H */

/** @} */
