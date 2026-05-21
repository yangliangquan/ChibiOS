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
 * @brief   Requires use of SPIv2 driver model.
 */
// #define HAL_LLD_SELECT_SPI_V2           FALSE

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

#if defined(HAL_LLD_USE_CLOCK_MANAGEMENT) || defined(__DOXYGEN__)
/**
 * @brief   Type of a clock configuration structure.
 */
typedef struct {
  uint32_t          FPRE;
  uint32_t          HPRE;
  uint32_t          SW;
  uint32_t          SYSPLL_SEL;
  uint32_t          PLLMUL;
  uint32_t          PLLDIV;
  uint32_t          PLLSRC;
} halclkcfg_t;
#endif /* defined(HAL_LLD_USE_CLOCK_MANAGEMENT) */

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/
#define resetHB1(x) do{RCC->HB1PRSTR |= x; RCC->HB1PRSTR &= ~x;(void)RCC->HB1PRSTR;}while(0)
#define resetHB2(x) do{RCC->HB2PRSTR |= x; RCC->HB2PRSTR &= ~x;(void)RCC->HB2PRSTR;}while(0)
#define resetHB(x) do{RCC->HBRSTR |= x; RCC->HBRSTR &= ~x;(void)RCC->HBRSTR;}while(0)

#define enableHB1(x) do{RCC->HB1PCENR |= x;}while(0)
#define enableHB2(x) do{RCC->HB2PCENR |= x;}while(0)
#define enableHB(x) do{RCC->HBPCENR |= x;}while(0)

#define disableHB1(x) do{RCC->HB1PCENR &= ~x;}while(0)
#define disableHB2(x) do{RCC->HB2PCENR &= ~x;}while(0)
#define disableHB(x) do{RCC->HBPCENR &= ~x;}while(0)
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
#if defined(HAL_LLD_USE_CLOCK_MANAGEMENT) || defined(__DOXYGEN__)
  bool hal_lld_clock_switch_mode(const halclkcfg_t *ccp);
  halfreq_t hal_lld_get_clock_point(halclkpt_t clkpt);
#endif /* defined(HAL_LLD_USE_CLOCK_MANAGEMENT) */
#ifdef __cplusplus
}
#endif

#endif /* HAL_LLD_H */

/** @} */
