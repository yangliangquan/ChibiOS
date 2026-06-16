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
/*
    Concepts and parts of this file have been contributed by Ilya Kharin.
*/

/**
 * @file    CH32H417/ch32_registry.h
 * @brief   CH32H417 capabilities registry.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef CH32_REGISTRY_H
#define CH32_REGISTRY_H

/*===========================================================================*/
/* Platform capabilities.                                                    */
/*===========================================================================*/

/**
 * @name    CH32H417 capabilities
 * @{
 */

/*===========================================================================*/
/* Common.                                                                   */
/*===========================================================================*/

/* RNG attributes.*/
#define CH32_HAS_RNG1 TRUE

/* RTC attributes.*/
#define CH32_HAS_RTC TRUE
#define CH32_RTC_HAS_SUBSECONDS TRUE
#define CH32_RTC_HAS_PERIODIC_WAKEUPS TRUE
#define CH32_RTC_NUM_ALARMS 2
#define CH32_RTC_STORAGE_SIZE 80
#define CH32_RTC_TAMP_STAMP_HANDLER RTC_IRQHandler
#define CH32_RTC_WKUP_HANDLER RTC_IRQHandler
#define CH32_RTC_ALARM_HANDLER RTCAlarm_IRQHandler
#define CH32_RTC_TAMP_STAMP_NUMBER 2
#define CH32_RTC_WKUP_NUMBER 3
#define CH32_RTC_ALARM_NUMBER 41
#define CH32_RTC_ALARM_EXTI 17
#define CH32_RTC_TAMP_STAMP_EXTI 18
#define CH32_RTC_WKUP_EXTI 19
#define CH32_RTC_IRQ_ENABLE()                                                                                         \
    do                                                                                                                 \
    {                                                                                                                  \
        NVIC_EnableIRQ(CH32_RTC_TAMP_STAMP_HANDLER);                                                                  \
        NVIC_EnableIRQ(CH32_RTC_ALARM_HANDLER);                                                                       \
    } while (false)

#define CH32_HAS_CRYP1 TRUE

/*===========================================================================*/
/* CH32H417.                                                              */
/*===========================================================================*/

#if defined(CH32H417) || defined(__DOXYGEN__)

/* ADC attributes.*/
#define CH32_HAS_ADC1 TRUE
#define CH32_HAS_ADC2 TRUE
#define CH32_ADC_SUPPORTS_PRESCALER        TRUE

/* CAN attributes.*/
#define CH32_HAS_CAN1 FALSE
#define CH32_HAS_CAN2 FALSE

/* DAC attributes.*/
#define CH32_HAS_DAC1_CH1 TRUE
#define CH32_HAS_DAC1_CH2 TRUE

/* DMA attributes.*/
#define CH32_HAS_DMA1 TRUE
#define CH32_HAS_DMA2 TRUE
#define CH32_ADVANCED_DMA TRUE
#define CH32_DMA_SUPPORTS_DMAMUX TRUE
#define CH32_DMA_SUPPORTS_CSELR FALSE
#define CH32_DMA1_NUM_CHANNELS 8
#define CH32_DMA2_NUM_CHANNELS 8


#define CH32_DMA1_CH1_HANDLER         DMA1_Channel1_IRQHandler
#define CH32_DMA1_CH2_HANDLER         DMA1_Channel2_IRQHandler
#define CH32_DMA1_CH3_HANDLER         DMA1_Channel3_IRQHandler
#define CH32_DMA1_CH4_HANDLER         DMA1_Channel4_IRQHandler
#define CH32_DMA1_CH5_HANDLER         DMA1_Channel5_IRQHandler
#define CH32_DMA1_CH6_HANDLER         DMA1_Channel6_IRQHandler
#define CH32_DMA1_CH7_HANDLER         DMA1_Channel7_IRQHandler
#define CH32_DMA1_CH8_HANDLER         DMA1_Channel8_IRQHandler
#define CH32_DMA2_CH1_HANDLER         DMA2_Channel1_IRQHandler
#define CH32_DMA2_CH2_HANDLER         DMA2_Channel2_IRQHandler
#define CH32_DMA2_CH3_HANDLER         DMA2_Channel3_IRQHandler
#define CH32_DMA2_CH4_HANDLER         DMA2_Channel4_IRQHandler
#define CH32_DMA2_CH5_HANDLER         DMA2_Channel5_IRQHandler
#define CH32_DMA2_CH6_HANDLER         DMA2_Channel6_IRQHandler
#define CH32_DMA2_CH7_HANDLER         DMA2_Channel7_IRQHandler
#define CH32_DMA2_CH8_HANDLER         DMA2_Channel8_IRQHandler

#define CH32_DMA1_CH1_NUMBER          DMA1_Channel1_IRQn
#define CH32_DMA1_CH2_NUMBER          DMA1_Channel2_IRQn
#define CH32_DMA1_CH3_NUMBER          DMA1_Channel3_IRQn
#define CH32_DMA1_CH4_NUMBER          DMA1_Channel4_IRQn
#define CH32_DMA1_CH5_NUMBER          DMA1_Channel5_IRQn
#define CH32_DMA1_CH6_NUMBER          DMA1_Channel6_IRQn
#define CH32_DMA1_CH7_NUMBER          DMA1_Channel7_IRQn
#define CH32_DMA1_CH8_NUMBER          DMA1_Channel8_IRQn
#define CH32_DMA2_CH1_NUMBER          DMA2_Channel1_IRQn
#define CH32_DMA2_CH2_NUMBER          DMA2_Channel2_IRQn
#define CH32_DMA2_CH3_NUMBER          DMA2_Channel3_IRQn
#define CH32_DMA2_CH4_NUMBER          DMA2_Channel4_IRQn
#define CH32_DMA2_CH5_NUMBER          DMA2_Channel5_IRQn
#define CH32_DMA2_CH6_NUMBER          DMA2_Channel6_IRQn
#define CH32_DMA2_CH7_NUMBER          DMA2_Channel7_IRQn
#define CH32_DMA2_CH8_NUMBER          DMA2_Channel8_IRQn

/* ETH attributes.*/
#define CH32_HAS_ETH TRUE

/* EXTI attributes.*/
#define CH32_EXTI_NUM_LINES 26
#define CH32_EXTI_MASK 0x7FFFFFFU

/* GPIO attributes.*/
#define CH32_HAS_GPIOA TRUE
#define CH32_HAS_GPIOB TRUE
#define CH32_HAS_GPIOC TRUE
#define CH32_HAS_GPIOD TRUE
#define CH32_HAS_GPIOE TRUE
#define CH32_HAS_GPIOF TRUE
#define CH32_HAS_GPIOG FALSE
#define CH32_HAS_GPIOH FALSE
#define CH32_HAS_GPIOI FALSE
#define CH32_HAS_GPIOJ FALSE
#define CH32_HAS_GPIOK FALSE
#define CH32_GPIO_EN_MASK (RCC_IOPAEN|RCC_IOPBEN|RCC_IOPCEN|RCC_IOPDEN|RCC_IOPEEN|RCC_IOPFEN)
enum {
#if CH32_HAS_GPIOA
  eGPIOA = 0,
#endif
#if CH32_HAS_GPIOB
  eGPIOB,
#endif
#if CH32_HAS_GPIOC
  eGPIOC,
#endif
#if CH32_HAS_GPIOD
  eGPIOD,
#endif
#if CH32_HAS_GPIOE
  eGPIOE,
#endif
#if CH32_HAS_GPIOF
  eGPIOF,
#endif
#if CH32_HAS_GPIOG
  eGPIOG,
#endif
  eGPIO_PORTS
};

/* I2C attributes.*/
#define CH32_HAS_I2C1 TRUE
#define CH32_HAS_I2C3 TRUE

#define CH32_HAS_I2C2 TRUE
#define CH32_HAS_I2C4 TRUE

/* QUADSPI attributes.*/
#define CH32_HAS_QUADSPI1 TRUE

/* SDMMC attributes.*/
#define CH32_HAS_SDMMC1 TRUE
#define CH32_HAS_SDMMC2 FALSE

/* SPI attributes.*/
#define CH32_HAS_SPI1 TRUE
#define CH32_SPI1_SUPPORTS_I2S TRUE
#define CH32_HAS_SPI2 TRUE
#define CH32_SPI2_SUPPORTS_I2S TRUE

#define CH32_HAS_SPI3 TRUE
#define CH32_HAS_SPI4 TRUE

/* TIM attributes.*/
#define CH32_TIM_MAX_CHANNELS 4

#define CH32_HAS_TIM1 TRUE
#define CH32_TIM1_IS_32BITS FALSE
#define CH32_TIM1_CHANNELS 4

#define CH32_HAS_TIM2 TRUE
#define CH32_TIM2_IS_32BITS FALSE
#define CH32_TIM2_CHANNELS 4

#define CH32_HAS_TIM3 TRUE
#define CH32_TIM3_IS_32BITS FALSE
#define CH32_TIM3_CHANNELS 4

#define CH32_HAS_TIM4 TRUE
#define CH32_TIM4_IS_32BITS FALSE
#define CH32_TIM4_CHANNELS 4

#define CH32_HAS_TIM5 TRUE
#define CH32_TIM5_IS_32BITS FALSE
#define CH32_TIM5_CHANNELS 4

#define CH32_HAS_TIM6 TRUE
#define CH32_TIM6_IS_32BITS FALSE
#define CH32_TIM6_CHANNELS 4

#define CH32_HAS_TIM7 TRUE
#define CH32_TIM7_IS_32BITS FALSE
#define CH32_TIM7_CHANNELS 4

#define CH32_HAS_TIM8 TRUE
#define CH32_TIM8_IS_32BITS FALSE
#define CH32_TIM8_CHANNELS 4

#define CH32_HAS_TIM9 TRUE
#define CH32_TIM9_IS_32BITS TRUE
#define CH32_TIM9_CHANNELS 4

#define CH32_HAS_TIM10 TRUE
#define CH32_TIM10_IS_32BITS TRUE
#define CH32_TIM10_CHANNELS 4

#define CH32_HAS_TIM11 TRUE
#define CH32_TIM11_IS_32BITS TRUE
#define CH32_TIM11_CHANNELS 4

#define CH32_HAS_TIM12 TRUE
#define CH32_TIM12_IS_32BITS TRUE
#define CH32_TIM12_CHANNELS 4

#define CH32_HAS_TIM13 FALSE
#define CH32_HAS_TIM14 FALSE
#define CH32_HAS_TIM15 FALSE
#define CH32_HAS_TIM16 FALSE
#define CH32_HAS_TIM17 FALSE
#define CH32_HAS_TIM18 FALSE
#define CH32_HAS_TIM19 FALSE
#define CH32_HAS_TIM20 FALSE
#define CH32_HAS_TIM21 FALSE
#define CH32_HAS_TIM22 FALSE

/* USART attributes.*/
#define CH32_HAS_USART1 TRUE
#define CH32_HAS_USART2 TRUE
#define CH32_HAS_USART3 TRUE
#define CH32_HAS_USART4 TRUE
#define CH32_HAS_USART5 TRUE
#define CH32_HAS_USART6 TRUE
#define CH32_HAS_USART7 TRUE
#define CH32_HAS_USART8 TRUE

/* USB attributes.*/
#define CH32_HAS_USB TRUE
#define CH32_USB_ACCESS_SCHEME_2x16 TRUE
#define CH32_USB_PMA_SIZE 1024
#define CH32_USB_HAS_BCDR TRUE

#define CH32_HAS_OTG1 TRUE

/* IWDG attributes.*/
#define CH32_HAS_IWDG TRUE
#define CH32_IWDG_IS_WINDOWED TRUE

/* LTDC attributes.*/
#define CH32_HAS_LTDC TRUE

/* DMA2D attributes.*/
#define CH32_HAS_DMA2D TRUE

/* FSMC attributes.*/
#define CH32_HAS_FSMC TRUE

/* CRC attributes.*/
#define CH32_HAS_CRC TRUE
#define CH32_CRC_PROGRAMMABLE TRUE
#endif /* defined(CH32H417) */


/** @} */

#endif /* CH32_REGISTRY_H */

/** @} */
