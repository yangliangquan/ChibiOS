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
 * @file    hal_icu_lld.h
 * @brief   CH32 ICU subsystem low level driver header.
 *
 * @addtogroup ICU
 * @{
 */

#ifndef HAL_ICU_LLD_H
#define HAL_ICU_LLD_H

#if (HAL_USE_ICU == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/
#define ICU_DRIVER_EXT_FIELDS                                                                                          \
    /* Extension fields.*/                                                                                             \
    TIM_TypeDef *tim;                                                                                                  \
    volatile uint32_t *wccrp;                                                                                          \
    volatile uint32_t *pccrp;
/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    CH32 configuration options
 * @{
 */
/**
 * @brief   ICUD1 driver enable switch.
 * @details If set to @p TRUE the support for ICUD1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_ICU_USE_TIM1) || defined(__DOXYGEN__)
#define CH32_ICU_USE_TIM1                  FALSE
#endif

/**
 * @brief   ICUD2 driver enable switch.
 * @details If set to @p TRUE the support for ICUD2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_ICU_USE_TIM2) || defined(__DOXYGEN__)
#define CH32_ICU_USE_TIM2                  FALSE
#endif

/**
 * @brief   ICUD3 driver enable switch.
 * @details If set to @p TRUE the support for ICUD3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_ICU_USE_TIM3) || defined(__DOXYGEN__)
#define CH32_ICU_USE_TIM3                  FALSE
#endif

/**
 * @brief   ICUD4 driver enable switch.
 * @details If set to @p TRUE the support for ICUD4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_ICU_USE_TIM4) || defined(__DOXYGEN__)
#define CH32_ICU_USE_TIM4                  FALSE
#endif

/**
 * @brief   ICUD5 driver enable switch.
 * @details If set to @p TRUE the support for ICUD5 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_ICU_USE_TIM5) || defined(__DOXYGEN__)
#define CH32_ICU_USE_TIM5                  FALSE
#endif

/**
 * @brief   ICUD6 driver enable switch.
 * @details If set to @p TRUE the support for ICUD6 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_ICU_USE_TIM6) || defined(__DOXYGEN__)
#define CH32_ICU_USE_TIM6                  FALSE
#endif

/**
 * @brief   ICUD7 driver enable switch.
 * @details If set to @p TRUE the support for ICUD7 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_ICU_USE_TIM7) || defined(__DOXYGEN__)
#define CH32_ICU_USE_TIM7                  FALSE
#endif

/**
 * @brief   ICUD8 driver enable switch.
 * @details If set to @p TRUE the support for ICUD8 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_ICU_USE_TIM8) || defined(__DOXYGEN__)
#define CH32_ICU_USE_TIM8                  FALSE
#endif

/**
 * @brief   ICUD9 driver enable switch.
 * @details If set to @p TRUE the support for ICUD9 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_ICU_USE_TIM9) || defined(__DOXYGEN__)
#define CH32_ICU_USE_TIM9                  FALSE
#endif

/**
 * @brief   ICUD10 driver enable switch.
 * @details If set to @p TRUE the support for ICUD10 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_ICU_USE_TIM10) || defined(__DOXYGEN__)
#define CH32_ICU_USE_TIM10                 FALSE
#endif

/**
 * @brief   ICUD11 driver enable switch.
 * @details If set to @p TRUE the support for ICUD11 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_ICU_USE_TIM11) || defined(__DOXYGEN__)
#define CH32_ICU_USE_TIM11                 FALSE
#endif

/**
 * @brief   ICUD12 driver enable switch.
 * @details If set to @p TRUE the support for ICUD12 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_ICU_USE_TIM12) || defined(__DOXYGEN__)
#define CH32_ICU_USE_TIM12                 FALSE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/
/* Checks on allocation of TIMx units.*/
#if CH32_ICU_USE_TIM1
#if defined(CH32_TIM1_IS_USED)
#error "ICUD1 requires TIM1 but the timer is already used"
#else
#define CH32_TIM1_IS_USED
#endif
#endif

#if CH32_ICU_USE_TIM2
#if defined(CH32_TIM2_IS_USED)
#error "ICUD2 requires TIM2 but the timer is already used"
#else
#define CH32_TIM2_IS_USED
#endif
#endif

#if CH32_ICU_USE_TIM3
#if defined(CH32_TIM3_IS_USED)
#error "ICUD3 requires TIM3 but the timer is already used"
#else
#define CH32_TIM3_IS_USED
#endif
#endif

#if CH32_ICU_USE_TIM4
#if defined(CH32_TIM4_IS_USED)
#error "ICUD4 requires TIM4 but the timer is already used"
#else
#define CH32_TIM4_IS_USED
#endif
#endif

#if CH32_ICU_USE_TIM5
#if defined(CH32_TIM5_IS_USED)
#error "ICUD5 requires TIM5 but the timer is already used"
#else
#define CH32_TIM5_IS_USED
#endif
#endif

#if CH32_ICU_USE_TIM8
#if defined(CH32_TIM8_IS_USED)
#error "ICUD8 requires TIM8 but the timer is already used"
#else
#define CH32_TIM8_IS_USED
#endif
#endif

#if CH32_ICU_USE_TIM9
#if defined(CH32_TIM9_IS_USED)
#error "ICUD9 requires TIM9 but the timer is already used"
#else
#define CH32_TIM9_IS_USED
#endif
#endif

#if CH32_ICU_USE_TIM10
#if defined(CH32_TIM10_IS_USED)
#error "ICUD10 requires TIM10 but the timer is already used"
#else
#define CH32_TIM10_IS_USED
#endif
#endif

#if CH32_ICU_USE_TIM11
#if defined(CH32_TIM11_IS_USED)
#error "ICUD11 requires TIM11 but the timer is already used"
#else
#define CH32_TIM11_IS_USED
#endif
#endif

#if CH32_ICU_USE_TIM12
#if defined(CH32_TIM12_IS_USED)
#error "ICUD12 requires TIM12 but the timer is already used"
#else
#define CH32_TIM12_IS_USED
#endif
#endif
/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   ICU driver mode.
 */
typedef enum {
  ICU_INPUT_ACTIVE_HIGH = 0,        /**< Trigger on rising edge.            */
  ICU_INPUT_ACTIVE_LOW = 1          /**< Trigger on falling edge.           */
} icumode_t;

/**
 * @brief   ICU frequency type.
 */
typedef uint32_t icufreq_t;

/**
 * @brief   ICU counter type.
 */
typedef uint32_t icucnt_t;

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
  /**
   * @brief   Driver mode.
   */
  icumode_t                 mode;
  /**
   * @brief   Timer clock in Hz.
   * @note    The low level can use assertions in order to catch invalid
   *          frequency specifications.
   */
  icufreq_t                 frequency;
  /**
   * @brief   Callback for pulse width measurement.
   */
  icucallback_t             width_cb;
  /**
   * @brief   Callback for cycle period measurement.
   */
  icucallback_t             period_cb;
  /**
   * @brief   Callback for timer overflow.
   */
  icucallback_t             overflow_cb;
  /* End of the mandatory fields.*/
  /* DMA settings: bit 0: update, bit 1: capture 0, bit 2: capture 1, bit 3: capture 2, bit 4: capture 3, bit 5: trigger*/
  uint16_t                  dma_settings; 

  uint32_t                  arr; /* Auto-reload value, zero means maximum value. */
} ICUConfig;

/**
 * @brief   Structure representing an ICU driver.
 */
struct ICUDriver {
  /**
   * @brief Driver state.
   */
  icustate_t                state;
  /**
   * @brief Current configuration data.
   */
  const ICUConfig           *config;
#if defined(ICU_DRIVER_EXT_FIELDS)
  ICU_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Returns the width of the latest pulse.
 * @details The pulse width is defined as number of ticks between the start
 *          edge and the stop edge.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 * @return              The number of ticks.
 *
 * @notapi
 */
#define icu_lld_get_width(icup) (*((icup)->wccrp) + 1)

/**
 * @brief   Returns the width of the latest cycle.
 * @details The cycle width is defined as number of ticks between a start
 *          edge and the next start edge.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 * @return              The number of ticks.
 *
 * @notapi
 */
#define icu_lld_get_period(icup) (*((icup)->pccrp) + 1)

/**
 * @brief   Check on notifications status.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 * @return              The notifications status.
 * @retval false        if notifications are not enabled.
 * @retval true         if notifications are enabled.
 *
 * @notapi
 */
#define icu_lld_are_notifications_enabled(icup)                                                                        \
    (bool)(((icup)->tim->DMAINTENR &                                                                                   \
            (TIM_UIE | TIM_CC1IE | TIM_CC2IE | TIM_CC3IE | TIM_CC4IE | TIM_COMIE | TIM_TIE | TIM_BIE)) != 0)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if CH32_ICU_USE_TIM1 && !defined(__DOXYGEN__)
extern ICUDriver ICUD1;
#endif

#if CH32_ICU_USE_TIM2 && !defined(__DOXYGEN__)
extern ICUDriver ICUD2;
#endif

#if CH32_ICU_USE_TIM3 && !defined(__DOXYGEN__)
extern ICUDriver ICUD3;
#endif

#if CH32_ICU_USE_TIM4 && !defined(__DOXYGEN__)
extern ICUDriver ICUD4;
#endif

#if CH32_ICU_USE_TIM5 && !defined(__DOXYGEN__)
extern ICUDriver ICUD5;
#endif

#if CH32_ICU_USE_TIM8 && !defined(__DOXYGEN__)
extern ICUDriver ICUD8;
#endif

#if CH32_ICU_USE_TIM9 && !defined(__DOXYGEN__)
extern ICUDriver ICUD9;
#endif

#if CH32_ICU_USE_TIM10 && !defined(__DOXYGEN__)
extern ICUDriver ICUD10;
#endif

#if CH32_ICU_USE_TIM11 && !defined(__DOXYGEN__)
extern ICUDriver ICUD11;
#endif

#if CH32_ICU_USE_TIM12 && !defined(__DOXYGEN__)
extern ICUDriver ICUD12;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void icu_lld_init(void);
  void icu_lld_start(ICUDriver *icup);
  void icu_lld_stop(ICUDriver *icup);
  void icu_lld_start_capture(ICUDriver *icup);
  bool icu_lld_wait_capture(ICUDriver *icup);
  void icu_lld_stop_capture(ICUDriver *icup);
  void icu_lld_enable_notifications(ICUDriver *icup);
  void icu_lld_disable_notifications(ICUDriver *icup);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_ICU == TRUE */

#endif /* HAL_ICU_LLD_H */

/** @} */
