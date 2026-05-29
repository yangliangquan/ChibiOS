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
 * @file    STv5f/hal_st_lld.h
 * @brief   CH32H417 V5F core ST subsystem low level driver header.
 * @details This header is designed to be include-able without having to
 *          include other files from the HAL.
 *
 * @warning This is a SKELETON file for the V5F core.
 *          Implement the V5F-specific system timer driver here.
 *
 * @addtogroup ST
 * @{
 */

#ifndef HAL_ST_LLD_H
#define HAL_ST_LLD_H

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/
/**
 * @name    Configuration options
 * @{
 */

/**
 * @brief   ST timer IRQ priority for V5F core.
 */
#if !defined(CH32_ST_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_ST_IRQ_PRIORITY               8
#endif

/**
 * @brief   System tick timer unit for V5F core.
 * @note    Select the timer to be used by the V5F core for free running mode.
 */
#if !defined(CH32_ST_USE_TIMER) || defined(__DOXYGEN__)
#define CH32_ST_USE_TIMER                  9
#endif

/**
 * @brief   Overrides the number of supported alarms.
 * @note    The default number of alarms is equal to the number of
 *          comparators in the timer, overriding it to one makes
 *          the driver a little faster and smaller. The kernel itself
 *          only needs one alarm, additional features could need more.
 * @note    Zero means do not override.
 */
#if !defined(CH32_ST_OVERRIDE_ALARMS) || defined(__DOXYGEN__)
#define CH32_ST_OVERRIDE_ALARMS            1
#endif
/** @} */

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void st_lld_init(void);
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Driver inline functions.                                                  */
/*===========================================================================*/

/**
 * @brief   Returns the time counter value.
 *
 * @return              The counter value.
 *
 * @notapi
 */
static inline systime_t st_lld_get_counter(void) {

  /* TODO: Implement V5F core counter read */
  return (systime_t)0U;
}

/**
 * @brief   Starts the alarm.
 *
 * @param[in] abstime   the time to be set for the first alarm
 *
 * @notapi
 */
static inline void st_lld_start_alarm(systime_t abstime) {

  /* TODO: Implement V5F core alarm start */
  (void)abstime;
}

/**
 * @brief   Stops the alarm interrupt.
 *
 * @notapi
 */
static inline void st_lld_stop_alarm(void) {

  /* TODO: Implement V5F core alarm stop */
}

/**
 * @brief   Sets the alarm time.
 *
 * @param[in] abstime   the time to be set for the next alarm
 *
 * @notapi
 */
static inline void st_lld_set_alarm(systime_t abstime) {

  /* TODO: Implement V5F core alarm set */
  (void)abstime;
}

/**
 * @brief   Returns the current alarm time.
 *
 * @return              The currently set alarm time.
 *
 * @notapi
 */
static inline systime_t st_lld_get_alarm(void) {

  /* TODO: Implement V5F core alarm get */
  return (systime_t)0U;
}

/**
 * @brief   Determines if the alarm is active.
 *
 * @return              The alarm status.
 * @retval false        if the alarm is not active.
 * @retval true         if the alarm is active.
 *
 * @notapi
 */
static inline bool st_lld_is_alarm_active(void) {

  /* TODO: Implement V5F core alarm active check */
  return false;
}

#endif /* HAL_ST_LLD_H */

/** @} */
