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
 * @file    hal_lld.h
 * @brief   CH32H417 HAL subsystem low level driver header.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef HAL_LLD_H
#define HAL_LLD_H

/*
 * Registry definitions.
 */
#include "ch32_registry.h"
#include "ch32_dma.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Specifies implementation of dynamic clock management.
 */
#define HAL_LLD_USE_CLOCK_MANAGEMENT

/**
 * @name    CH32H417 identification macros
 * @{
 */
#define CH32H417_NAME           "CH32H417"
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    CH32H417 configuration options
 * @{
 */
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*
 * Configuration-related checks.
 */
#if !defined(CH32H417_MCUCONF)
#error "Using a wrong mcuconf.h file, CH32H417_MCUCONF not defined"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   RCC clock frequencies structure.
 */
typedef struct {
  uint32_t SYSCLK_Frequency;  /**< SYSCLK clock frequency expressed in Hz */
  uint32_t HCLK_Frequency;    /**< HCLK/Core0 clock frequency expressed in Hz */
  uint32_t Core_Frequency;    /**< Core clock frequency expressed in Hz */
  uint32_t ADCCLK_Frequency;  /**< ADCCLK clock frequency expressed in Hz */
} RCC_ClocksTypeDef;

#if defined(HAL_LLD_USE_CLOCK_MANAGEMENT) || defined(__DOXYGEN__)
/**
 * @brief   Type of a clock configuration structure.
 */
typedef struct {
  uint32_t          FPRE;       /**< Fast peripheral clock prescaler */
  uint32_t          HPRE;       /**< AHB prescaler */
  uint32_t          SW;         /**< System clock switch */
  uint32_t          SYSPLL_SEL; /**< System PLL clock source selection */
  uint32_t          PLLMUL;     /**< PLL multiplication factor */
  uint32_t          PLLDIV;     /**< PLL input division factor */
  uint32_t          PLLSRC;     /**< PLL clock source */
} halclkcfg_t;
#endif /* defined(HAL_LLD_USE_CLOCK_MANAGEMENT) */

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @name    AHB reset macros
 * @{
 */
/**
 * @brief   Reset AHB1 peripherals.
 * @param[in] x         peripherals mask
 */
#define resetHB1(x) do {                                                   \
  RCC->HB1PRSTR |= (x);                                                    \
  RCC->HB1PRSTR &= ~(x);                                                   \
  (void)RCC->HB1PRSTR;                                                     \
} while (0)

/**
 * @brief   Reset AHB2 peripherals.
 * @param[in] x         peripherals mask
 */
#define resetHB2(x) do {                                                   \
  RCC->HB2PRSTR |= (x);                                                    \
  RCC->HB2PRSTR &= ~(x);                                                   \
  (void)RCC->HB2PRSTR;                                                     \
} while (0)

/**
 * @brief   Reset AHB peripherals.
 * @param[in] x         peripherals mask
 */
#define resetHB(x) do {                                                     \
  RCC->HBRSTR |= (x);                                                      \
  RCC->HBRSTR &= ~(x);                                                     \
  (void)RCC->HBRSTR;                                                       \
} while (0)
/** @} */

/**
 * @name    AHB enable macros
 * @{
 */
/**
 * @brief   Enable AHB1 peripherals clock.
 * @param[in] x         peripherals mask
 */
#define enableHB1(x) do {                                                  \
  RCC->HB1PCENR |= (x);                                                    \
} while (0)

/**
 * @brief   Enable AHB2 peripherals clock.
 * @param[in] x         peripherals mask
 */
#define enableHB2(x) do {                                                  \
  RCC->HB2PCENR |= (x);                                                    \
} while (0)

/**
 * @brief   Enable AHB peripherals clock.
 * @param[in] x         peripherals mask
 */
#define enableHB(x) do {                                                    \
  RCC->HBPCENR |= (x);                                                     \
} while (0)
/** @} */

/**
 * @name    AHB disable macros
 * @{
 */
/**
 * @brief   Disable AHB1 peripherals clock.
 * @param[in] x         peripherals mask
 */
#define disableHB1(x) do {                                                 \
  RCC->HB1PCENR &= ~(x);                                                   \
} while (0)

/**
 * @brief   Disable AHB2 peripherals clock.
 * @param[in] x         peripherals mask
 */
#define disableHB2(x) do {                                                 \
  RCC->HB2PCENR &= ~(x);                                                   \
} while (0)

/**
 * @brief   Disable AHB peripherals clock.
 * @param[in] x         peripherals mask
 */
#define disableHB(x) do {                                                   \
  RCC->HBPCENR &= ~(x);                                                    \
} while (0)
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if defined(HAL_LLD_USE_CLOCK_MANAGEMENT) && !defined(__DOXYGEN__)
extern const halclkcfg_t hal_clkcfg_reset;
extern const halclkcfg_t hal_clkcfg_default;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void hal_lld_init(void);
  void RCC_GetClocksFreq(RCC_ClocksTypeDef *RCC_Clocks);
#if defined(HAL_LLD_USE_CLOCK_MANAGEMENT) || defined(__DOXYGEN__)
  bool hal_lld_clock_switch_mode(const halclkcfg_t *ccp);
  halfreq_t hal_lld_get_clock_point(halclkpt_t clkpt);
#endif /* defined(HAL_LLD_USE_CLOCK_MANAGEMENT) */
#ifdef __cplusplus
}
#endif

#endif /* HAL_LLD_H */

/** @} */
