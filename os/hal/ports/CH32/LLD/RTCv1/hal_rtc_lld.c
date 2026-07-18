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
   Concepts and parts of this file have been contributed by Uladzimir Pylinsky
   aka barthess.
 */

/**
 * @file    hal_rtc_lld.c
 * @brief   CH32 RTC subsystem low level driver source.
 *
 * @addtogroup RTC
 * @{
 */

#include "hal.h"

#if (HAL_USE_RTC == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief RTC driver identifier.
 */
#if (CH32_RTC_USE_RTC1 == TRUE) && !defined(__DOXYGEN__)
RTCDriver RTCD1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Waits until the RTC registers are synchronized with the APB clock.
 * @note    Must be called before any read operation after an APB reset
 *          or an APB clock stop.
 *
 * @notapi
 */
static void rtc_wait_for_synchro(void) {

  RTC->CTLRL &= (uint16_t)~RTC_CTLRL_RSF;
  while ((RTC->CTLRL & RTC_CTLRL_RSF) == (uint16_t)RESET) {
  }
}

/**
 * @brief   Waits until the last write operation on RTC registers has finished.
 * @note    Must be called before any write to RTC registers.
 *
 * @notapi
 */
static void rtc_wait_for_last_task(void) {

  while ((RTC->CTLRL & RTC_CTLRL_RTOFF) == (uint16_t)RESET) {
  }
}

/**
 * @brief   Acquires write access to the backup domain registers.
 * @details Waits for write completion, then enters configuration mode.
 *
 * @notapi
 */
static void rtc_acquire_access(void) {

  rtc_wait_for_last_task();
  RTC->CTLRL |= RTC_CTLRL_CNF;
}

/**
 * @brief   Releases write access to the backup domain registers.
 * @details Exits configuration mode and waits for write completion.
 *
 * @notapi
 */
static void rtc_release_access(void) {

  RTC->CTLRL &= (uint16_t)~RTC_CTLRL_CNF;
  rtc_wait_for_last_task();
}

/**
 * @brief   Converts @p RTCDateTime to seconds since Unix epoch.
 *
 * @param[in] dtsp      pointer to a @p RTCDateTime structure
 * @return              seconds since 1970-01-01 00:00:00 UTC
 *
 * @notapi
 */
static uint32_t rtc_encode(const RTCDateTime *dtsp) {
  struct tm timp;
  uint32_t tv_msec;

  rtcConvertDateTimeToStructTm(dtsp, &timp, &tv_msec);
  return (uint32_t)mktime(&timp);
}

/**
 * @brief   Converts seconds since Unix epoch and milliseconds to
 *          @p RTCDateTime.
 *
 * @param[in] tv_sec    seconds since 1970-01-01 00:00:00 UTC
 * @param[in] tv_msec   milliseconds fraction (0..999)
 * @param[out] dtsp     pointer to a @p RTCDateTime structure
 *
 * @notapi
 */
static void rtc_decode(uint32_t tv_sec, uint32_t tv_msec,
                        RTCDateTime *dtsp) {
  struct tm timp;

  localtime_r((const time_t *)&tv_sec, &timp);
  rtcConvertStructTmToDateTime(&timp, tv_msec, dtsp);
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/**
 * @brief   RTC IRQ handler.
 * @details Handles second and alarm interrupt events.
 */
OSAL_IRQ_HANDLER(CH32_RTC_TAMP_STAMP_HANDLER) {
  OSAL_IRQ_PROLOGUE();

#if (CH32_RTC_USE_RTC1 == TRUE)
  if ((RTC->CTLRH & RTC_CTLRH_SECIE) != 0 &&
      (RTC->CTLRL & RTC_CTLRL_SECF) != 0) {
    RTC->CTLRL &= (uint16_t)~RTC_CTLRL_SECF;
    if (RTCD1.callback != NULL) {
      RTCD1.callback(&RTCD1, RTC_EVENT_SECOND);
    }
  }
  if ((RTC->CTLRH & RTC_CTLRH_OWIE) != 0 &&
      (RTC->CTLRL & RTC_CTLRL_OWF) != 0) {
    RTC->CTLRL &= (uint16_t)~RTC_CTLRL_OWF;
  }
#endif

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   RTC Alarm IRQ handler.
 * @details Handles alarm match interrupt events.
 */
OSAL_IRQ_HANDLER(CH32_RTC_ALARM_HANDLER) {
  OSAL_IRQ_PROLOGUE();

#if (CH32_RTC_USE_RTC1 == TRUE)
  if ((RTC->CTLRL & RTC_CTLRL_ALRF) != 0) {
    RTC->CTLRL &= (uint16_t)~RTC_CTLRL_ALRF;
    if (RTCD1.callback != NULL) {
      RTCD1.callback(&RTCD1, RTC_EVENT_ALARM);
    }
  }
#endif

  OSAL_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Initialize RTC subsystem.
 */
void rtc_lld_init(void) {

#if CH32_RTC_USE_RTC1 == TRUE
  /* RTC object initialization.*/
  rtcObjectInit(&RTCD1);

  RTCD1.callback = NULL;

  /* Enable PWR and BKP clocks.*/
  enableHB1(RCC_PWREN);
  enableHB1(RCC_BKPEN);

  /* Enable backup domain access.*/
  PWR->CTLR |= PWR_CTLR_DBP;

  /* Clear pending RTC interrupts.*/
  RTC->CTLRL &= (uint16_t)~(RTC_CTLRL_SECF | RTC_CTLRL_ALRF | RTC_CTLRL_OWF);

  /* Enable LSI as RTC clock source.*/
  RCC->RSTSCKR |= RCC_LSION;
  while ((RCC->RSTSCKR & RCC_LSIRDY) == 0) {
  }

  /* Select LSI as RTC clock and enable RTC.*/
  RCC->BDCTLR = (RCC->BDCTLR & (uint32_t)~(RCC_RTCSEL | RCC_RTCEN)) |
                RCC_RTCSEL_LSI | RCC_RTCEN;

  /* Wait for RTC registers synchronization.*/
  rtc_wait_for_synchro();

  /* Disable all RTC interrupts.*/
  RTC->CTLRH = 0;
  rtc_wait_for_last_task();

  /* Enable RTCAlarm IRQ vector.*/
  NVIC_EnableIRQ(RTCAlarm_IRQn);
  /* Enable RTC IRQ vector.*/
  NVIC_EnableIRQ(RTC_IRQn);
#endif /* CH32_RTC_USE_RTC1 == TRUE */
}

/**
 * @brief   Set current time.
 * @note    Fractional part will be silently ignored. There is no possibility
 *          to set it on the CH32 platform.
 * @note    The function can be called from any context.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @param[in] timespec  pointer to a @p RTCDateTime structure
 *
 * @notapi
 */
void rtc_lld_set_time(RTCDriver *rtcp, const RTCDateTime *timespec) {
  syssts_t sts;
  uint32_t sec;

  (void)rtcp;

  sts = osalSysGetStatusAndLockX();
  sec = rtc_encode(timespec);
  rtc_acquire_access();
  RTC->CNTH = (uint16_t)(sec >> 16);
  RTC->CNTL = (uint16_t)(sec & 0xFFFF);
  rtc_release_access();
  osalSysRestoreStatusX(sts);
}

/**
 * @brief   Get current time.
 * @note    The function can be called from any context.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @param[out] timespec pointer to a @p RTCDateTime structure
 *
 * @notapi
 */
void rtc_lld_get_time(RTCDriver *rtcp, RTCDateTime *timespec) {

  (void)rtcp;

  uint32_t sec, msec;

  rtc_lld_get_counter(&sec, &msec);
  rtc_decode(sec, msec, timespec);
}

#if (RTC_ALARMS > 0) || defined(__DOXYGEN__)
/**
 * @brief   Set alarm time.
 * @note    Default value after BKP domain reset for both comparators is 0.
 * @note    Function does not perform any checks of alarm time validity.
 * @note    The function can be called from any context.
 *
 * @param[in] rtcp      pointer to RTC driver structure.
 * @param[in] alarm     alarm identifier. Can be 0.
 * @param[in] alarmspec pointer to a @p RTCAlarm structure.
 *
 * @notapi
 */
void rtc_lld_set_alarm(RTCDriver *rtcp,
                       rtcalarm_t alarm,
                       const RTCAlarm *alarmspec) {
  syssts_t sts;

  (void)rtcp;
  (void)alarm;

  sts = osalSysGetStatusAndLockX();

  if (alarmspec != NULL) {
    rtc_acquire_access();
    RTC->ALRMH = (uint16_t)(alarmspec->tv_sec >> 16);
    RTC->ALRML = (uint16_t)(alarmspec->tv_sec & 0xFFFF);
    rtc_release_access();

    /* Enable alarm interrupt.*/
    RTC->CTLRH |= RTC_CTLRH_ALRIE;
    rtc_wait_for_last_task();
  }
  else {
    /* Disable alarm interrupt.*/
    RTC->CTLRH &= (uint16_t)~RTC_CTLRH_ALRIE;
    rtc_wait_for_last_task();

    rtc_acquire_access();
    RTC->ALRMH = 0;
    RTC->ALRML = 0;
    rtc_release_access();
  }

  osalSysRestoreStatusX(sts);
}

/**
 * @brief   Get alarm time.
 * @note    The function can be called from any context.
 *
 * @param[in] rtcp       pointer to RTC driver structure
 * @param[in] alarm      alarm identifier
 * @param[out] alarmspec pointer to a @p RTCAlarm structure
 *
 * @notapi
 */
void rtc_lld_get_alarm(RTCDriver *rtcp,
                       rtcalarm_t alarm,
                       RTCAlarm *alarmspec) {
  syssts_t sts;

  (void)rtcp;
  (void)alarm;

  sts = osalSysGetStatusAndLockX();

  rtc_wait_for_synchro();
  alarmspec->tv_sec = ((uint32_t)RTC->ALRMH << 16) |
                      ((uint32_t)RTC->ALRML);

  osalSysRestoreStatusX(sts);
}
#endif /* RTC_ALARMS > 0 */

#if (RTC_SUPPORTS_CALLBACKS == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   Set callback function.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @param[in] callback  pointer to callback function, or NULL to disable
 *
 * @notapi
 */
void rtc_lld_set_callback(RTCDriver *rtcp, rtccb_t callback) {
  syssts_t sts;

  (void)rtcp;

  sts = osalSysGetStatusAndLockX();
  RTCD1.callback = callback;
  osalSysRestoreStatusX(sts);
}
#endif /* RTC_SUPPORTS_CALLBACKS == TRUE */

/**
 * @brief   Set the RTC prescaler reload value.
 * @details The prescaler determines the RTC clock divider to achieve 1 Hz.
 *          This function should be called before setting the time.
 *
 * @param[in] prescaler prescaler reload value (clock_freq - 1)
 *
 * @notapi
 */
void rtc_lld_set_prescaler(uint32_t prescaler) {

  rtc_acquire_access();
  RTC->PSCRH = (uint16_t)((prescaler >> 16) & 0x0F);
  RTC->PSCRL = (uint16_t)(prescaler & 0xFFFF);
  rtc_release_access();
}

/**
 * @brief   Get current RTC counter value.
 * @details Reads the 32-bit seconds counter and the fractional divider
 *          to compute seconds and milliseconds since midnight.
 *
 * @param[out] sec      pointer to seconds value (since epoch)
 * @param[out] msec     pointer to milliseconds value (0..999)
 *
 * @notapi
 */
void rtc_lld_get_counter(uint32_t *sec, uint32_t *msec) {
  syssts_t sts;
  uint16_t high1, high2, low;
  uint32_t counter;
  uint32_t divider;
  uint32_t prescaler;

  sts = osalSysGetStatusAndLockX();

  /* Read counter with double-read safety for 16-bit register rollover.*/
  high1 = RTC->CNTH;
  low   = RTC->CNTL;
  high2 = RTC->CNTH;

  if (high1 != high2) {
    /* High word rolled over, re-read low word.*/
    counter = ((uint32_t)high2 << 16) | (uint32_t)RTC->CNTL;
  }
  else {
    counter = ((uint32_t)high1 << 16) | (uint32_t)low;
  }

  /* Read fractional divider for sub-second precision.
   * The divider counts down from the reload value to 0, then the counter
   * increments. Milliseconds = (prescaler - divider) * 1000 / prescaler. */
  divider = ((uint32_t)(RTC->DIVH & 0x0F) << 16) |
            (uint32_t)RTC->DIVL;

  if (sec != NULL) {
    *sec = counter;
  }
  if (msec != NULL) {
    /* Compute milliseconds from the fractional counter.
     * divider counts from (prescaler-1) down to 0.
     * frac = (prescaler - 1 - divider) / prescaler * 1000 */
    prescaler = CH32_RTC_CLOCK_FREQ;
    if (divider < prescaler) {
      *msec = ((prescaler - 1 - divider) * 1000U) / prescaler;
    }
    else {
      *msec = 0;
    }
  }

  osalSysRestoreStatusX(sts);
}

/**
 * @brief   Set the RTC counter value directly.
 *
 * @param[in] sec       seconds since Unix epoch to write
 *
 * @notapi
 */
void rtc_lld_set_counter(uint32_t sec) {
  syssts_t sts;

  sts = osalSysGetStatusAndLockX();
  rtc_acquire_access();
  RTC->CNTH = (uint16_t)(sec >> 16);
  RTC->CNTL = (uint16_t)(sec & 0xFFFF);
  rtc_release_access();
  osalSysRestoreStatusX(sts);
}

#endif /* HAL_USE_RTC */

/** @} */
