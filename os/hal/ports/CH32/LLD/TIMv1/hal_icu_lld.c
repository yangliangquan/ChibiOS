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
 * @file    hal_icu_lld.c
 * @brief   CH32 ICU subsystem low level driver source.
 *
 * @addtogroup ICU
 * @{
 */

#include "hal.h"

#if (HAL_USE_ICU == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   ICUD1 driver identifier.
 * @note    The driver ICUD1 allocates the complex timer TIM1 when enabled.
 */
#if CH32_ICU_USE_TIM1 || defined(__DOXYGEN__)
ICUDriver ICUD1;
#endif

/**
 * @brief   ICUD2 driver identifier.
 * @note    The driver ICUD2 allocates the timer TIM2 when enabled.
 */
#if CH32_ICU_USE_TIM2 || defined(__DOXYGEN__)
ICUDriver ICUD2;
#endif

/**
 * @brief   ICUD3 driver identifier.
 * @note    The driver ICUD3 allocates the timer TIM3 when enabled.
 */
#if CH32_ICU_USE_TIM3 || defined(__DOXYGEN__)
ICUDriver ICUD3;
#endif

/**
 * @brief   ICUD4 driver identifier.
 * @note    The driver ICUD4 allocates the timer TIM4 when enabled.
 */
#if CH32_ICU_USE_TIM4 || defined(__DOXYGEN__)
ICUDriver ICUD4;
#endif

/**
 * @brief   ICUD5 driver identifier.
 * @note    The driver ICUD5 allocates the timer TIM5 when enabled.
 */
#if CH32_ICU_USE_TIM5 || defined(__DOXYGEN__)
ICUDriver ICUD5;
#endif

/**
 * @brief   ICUD8 driver identifier.
 * @note    The driver ICUD8 allocates the timer TIM8 when enabled.
 */
#if CH32_ICU_USE_TIM8 || defined(__DOXYGEN__)
ICUDriver ICUD8;
#endif

/**
 * @brief   ICUD9 driver identifier.
 * @note    The driver ICUD9 allocates the timer TIM9 when enabled.
 */
#if CH32_ICU_USE_TIM9 || defined(__DOXYGEN__)
ICUDriver ICUD9;
#endif

/**
 * @brief   ICUD10 driver identifier.
 * @note    The driver ICUD10 allocates the timer TIM10 when enabled.
 */
#if CH32_ICU_USE_TIM10 || defined(__DOXYGEN__)
ICUDriver ICUD10;
#endif

/**
 * @brief   ICUD11 driver identifier.
 * @note    The driver ICUD11 allocates the timer TIM11 when enabled.
 */
#if CH32_ICU_USE_TIM11 || defined(__DOXYGEN__)
ICUDriver ICUD11;
#endif

/**
 * @brief   ICUD12 driver identifier.
 * @note    The driver ICUD12 allocates the timer TIM12 when enabled.
 */
#if CH32_ICU_USE_TIM12 || defined(__DOXYGEN__)
ICUDriver ICUD12;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/
void icu_lld_serve_interrupt(ICUDriver *icup) {
  uint32_t sr = icup->tim->INTFR;
  icup->tim->INTFR = ~sr;
  if(sr & TIM_UIF) {
    if (icup->config->overflow_cb != NULL) {
      icup->config->overflow_cb(icup);
    }
  }
  if(sr & (TIM_CC1IF)){
    _icu_isr_invoke_period_cb(icup);
  }
  if(sr & (TIM_CC2IF)){
    _icu_isr_invoke_width_cb(icup);
  }
}
#if CH32_ICU_USE_TIM1 || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(TIM1_UP_IRQHandler) {

  icu_lld_serve_interrupt(&ICUD1);

}

OSAL_IRQ_HANDLER(TIM1_CC_IRQHandler) {

  icu_lld_serve_interrupt(&ICUD1);

}
#endif /* CH32_ICU_USE_TIM1 */

#if CH32_ICU_USE_TIM2 || defined(__DOXYGEN__)
/**
 * @brief   TIM2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM2_IRQHandler) {

  icu_lld_serve_interrupt(&ICUD2);

}
#endif /* CH32_ICU_USE_TIM2 */

#if CH32_ICU_USE_TIM3 || defined(__DOXYGEN__)
/**
 * @brief   TIM3 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM3_IRQHandler) {

  icu_lld_serve_interrupt(&ICUD3);

}
#endif /* CH32_ICU_USE_TIM3 */

#if CH32_ICU_USE_TIM4 || defined(__DOXYGEN__)
/**
 * @brief   TIM4 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM4_IRQHandler) {

  icu_lld_serve_interrupt(&ICUD4);

}
#endif /* CH32_ICU_USE_TIM4 */

#if CH32_ICU_USE_TIM5 || defined(__DOXYGEN__)
/**
 * @brief   TIM5 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM5_IRQHandler) {

  icu_lld_serve_interrupt(&ICUD5);

}
#endif /* CH32_ICU_USE_TIM5 */

#if CH32_ICU_USE_TIM8 || defined(__DOXYGEN__)
/**
 * @brief   TIM8 update interrupt handler.
 * @note    It is assumed that this interrupt is only activated if the callback
 *          pointer is not equal to @p NULL in order to not perform an extra
 *          check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM8_UP_IRQHandler) {

  icu_lld_serve_interrupt(&ICUD8);

}

/**
 * @brief   TIM8 compare interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM8_CC_IRQHandler) {

  icu_lld_serve_interrupt(&ICUD8);

}
#endif /* CH32_ICU_USE_TIM8 */

#if CH32_ICU_USE_TIM9 || defined(__DOXYGEN__)
/**
 * @brief   TIM9 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM9_IRQHandler) {

  icu_lld_serve_interrupt(&ICUD9);

}
#endif /* CH32_ICU_USE_TIM9 */

#if CH32_ICU_USE_TIM10 || defined(__DOXYGEN__)
/**
 * @brief   TIM10 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM10_IRQHandler) {

  icu_lld_serve_interrupt(&ICUD10);

}
#endif /* CH32_ICU_USE_TIM10 */

#if CH32_ICU_USE_TIM11 || defined(__DOXYGEN__)
/**
 * @brief   TIM11 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM11_IRQHandler) {

  icu_lld_serve_interrupt(&ICUD11);

}
#endif /* CH32_ICU_USE_TIM11 */

#if CH32_ICU_USE_TIM12 || defined(__DOXYGEN__)
/**
 * @brief   TIM12 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM12_IRQHandler) {

  icu_lld_serve_interrupt(&ICUD12);

}
#endif /* CH32_ICU_USE_TIM12 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level ICU driver initialization.
 *
 * @notapi
 */
void icu_lld_init(void) {

#if CH32_ICU_USE_TIM1 == TRUE
  /* Driver initialization.*/
  icuObjectInit(&ICUD1);
  ICUD1.tim = TIM1;
#endif

#if CH32_ICU_USE_TIM2 == TRUE
  /* Driver initialization.*/
  icuObjectInit(&ICUD2);
  ICUD2.tim = TIM2;
#endif

#if CH32_ICU_USE_TIM3 == TRUE
  /* Driver initialization.*/
  icuObjectInit(&ICUD3);
  ICUD3.tim = TIM3;
#endif

#if CH32_ICU_USE_TIM4 == TRUE
  /* Driver initialization.*/
  icuObjectInit(&ICUD4);
  ICUD4.tim = TIM4;
#endif

#if CH32_ICU_USE_TIM5 == TRUE
  /* Driver initialization.*/
  icuObjectInit(&ICUD5);
  ICUD5.tim = TIM5;
#endif

#if CH32_ICU_USE_TIM6 == TRUE
  /* Driver initialization.*/
  icuObjectInit(&ICUD6);
  ICUD6.tim = TIM6;
#endif

#if CH32_ICU_USE_TIM7 == TRUE
  /* Driver initialization.*/
  icuObjectInit(&ICUD7);
  ICUD7.tim = TIM7;
#endif

#if CH32_ICU_USE_TIM8 == TRUE
  /* Driver initialization.*/
  icuObjectInit(&ICUD8);
  ICUD8.tim = TIM8;
#endif

#if CH32_ICU_USE_TIM9 == TRUE
  /* Driver initialization.*/
  icuObjectInit(&ICUD9);
  ICUD9.tim = TIM9;
#endif

#if CH32_ICU_USE_TIM10 == TRUE
  /* Driver initialization.*/
  icuObjectInit(&ICUD10);
  ICUD10.tim = TIM10;
#endif

#if CH32_ICU_USE_TIM11 == TRUE
  /* Driver initialization.*/
  icuObjectInit(&ICUD11);
  ICUD11.tim = TIM11;
#endif

#if CH32_ICU_USE_TIM12 == TRUE
  /* Driver initialization.*/
  icuObjectInit(&ICUD12);
  ICUD12.tim = TIM12;
#endif
}

/**
 * @brief   Configures and activates the ICU peripheral.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_start(ICUDriver *icup) {
  uint32_t clock = hal_lld_get_clock_point((halclkpt_t)NULL);
  if (icup->state == ICU_STOP) {
    /* Clock activation and timer reset.*/
#if CH32_ICU_USE_TIM1 == TRUE
    if (&ICUD1 == icup) {
      enableHB2(RCC_TIM1EN);
      NVIC_EnableIRQ(TIM1_UP_IRQn);
      NVIC_EnableIRQ(TIM1_CC_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM2 == TRUE
    if (&ICUD2 == icup) {
      enableHB1(RCC_TIM2EN);
      NVIC_EnableIRQ(TIM2_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM3 == TRUE
    if (&ICUD3 == icup) {
      enableHB1(RCC_TIM3EN);
      NVIC_EnableIRQ(TIM3_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM4 == TRUE
    if (&ICUD4 == icup) {
      enableHB1(RCC_TIM4EN);
      NVIC_EnableIRQ(TIM4_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM5 == TRUE
    if (&ICUD5 == icup) {
      enableHB1(RCC_TIM5EN);
      NVIC_EnableIRQ(TIM5_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM6 == TRUE
    if (&ICUD6 == icup) {
      enableHB1(RCC_TIM6EN);
      NVIC_EnableIRQ(TIM6_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM7 == TRUE
    if (&ICUD7 == icup) {
      enableHB1(RCC_TIM7EN);
      NVIC_EnableIRQ(TIM7_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM8 == TRUE
    if (&ICUD8 == icup) {
      enableHB2(RCC_TIM8EN);
      NVIC_EnableIRQ(TIM8_UP_IRQn);
      NVIC_EnableIRQ(TIM8_CC_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM9 == TRUE
    if (&ICUD9 == icup) {
      enableHB2(RCC_TIM9EN);
      NVIC_EnableIRQ(TIM9_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM10 == TRUE
    if (&ICUD10 == icup) {
      enableHB2(RCC_TIM10EN);
      NVIC_EnableIRQ(TIM10_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM11 == TRUE
    if (&ICUD11 == icup) {
      enableHB2(RCC_TIM11EN);
      NVIC_EnableIRQ(TIM11_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM12 == TRUE
    if (&ICUD12 == icup) {
      enableHB2(RCC_TIM12EN);
      NVIC_EnableIRQ(TIM12_IRQn);
    }
#endif
    icup->tim->CH1CVR_32 = 0;
    icup->tim->CH2CVR_32 = 0;
    icup->tim->CH3CVR_32 = 0;
    icup->tim->CH4CVR_32 = 0;
  }
  icup->tim->INTFR = 0;
  icup->tim->DMAINTENR = (icup->config->dma_settings << 8) & ~(TIM_CC1IE | TIM_CC2IE | TIM_CC3IE | TIM_CC4IE | TIM_UIE);
  uint32_t psc = (clock / icup->config->frequency) - 1;
  icup->tim->PSC = psc;

  if(icup->config->arr == 0){
    icup->tim->ATRLR_32 = 0xFFFFFFFF;
  }
  else{
    icup->tim->ATRLR_32 = icup->config->arr;
  }

  icup->tim->CHCTLR1 = (icup->tim->CHCTLR1 & ~TIM_CC1S) | 0x1;
  if(ICU_INPUT_ACTIVE_HIGH == icup->config->mode){
    icup->tim->CCER &= ~(TIM_CC1P);
  }else
  {
    icup->tim->CCER |= (TIM_CC1P);
  }
  
  icup->tim->CHCTLR1 = (icup->tim->CHCTLR1 & ~TIM_CC2S) | (0x2 << 8);
  if (ICU_INPUT_ACTIVE_HIGH == icup->config->mode)
  { 
    icup->tim->CCER |= (TIM_CC2P);
  }else
  {
    icup->tim->CCER &= ~(TIM_CC2P);
  }
  
  icup->tim->SMCFGR = ((icup->tim->SMCFGR & ~(TIM_SMS | TIM_TS)) | (0x5 << 4) | (0x4 << 0) | TIM_MSM);
  icup->pccrp = &icup->tim->CH1CVR_32;
  icup->wccrp = &icup->tim->CH2CVR_32;
  icup->tim->CCER |= TIM_CC1E | TIM_CC2E;
}

/**
 * @brief   Deactivates the ICU peripheral.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_stop(ICUDriver *icup) {

  if (icup->state == ICU_READY) {
    /* Clock deactivation.*/
#if CH32_ICU_USE_TIM1 == TRUE
    if (&ICUD1 == icup) {
      disableHB2(RCC_TIM1EN);
      NVIC_DisableIRQ(TIM1_UP_IRQn);
      NVIC_DisableIRQ(TIM1_CC_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM2 == TRUE
    if (&ICUD2 == icup) {
      disableHB1(RCC_TIM2EN);
      NVIC_DisableIRQ(TIM2_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM3 == TRUE
    if (&ICUD3 == icup) {
      disableHB1(RCC_TIM3EN);
      NVIC_DisableIRQ(TIM3_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM4 == TRUE
    if (&ICUD4 == icup) {
      disableHB1(RCC_TIM4EN);
      NVIC_DisableIRQ(TIM4_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM5 == TRUE
    if (&ICUD5 == icup) {
      disableHB1(RCC_TIM5EN);
      NVIC_DisableIRQ(TIM5_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM6 == TRUE
    if (&ICUD6 == icup) {
      disableHB1(RCC_TIM6EN);
      NVIC_DisableIRQ(TIM6_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM7 == TRUE
    if (&ICUD7 == icup) {
      disableHB1(RCC_TIM7EN);
      NVIC_DisableIRQ(TIM7_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM8 == TRUE
    if (&ICUD8 == icup) {
      disableHB2(RCC_TIM8EN);
      NVIC_DisableIRQ(TIM8_UP_IRQn);
      NVIC_DisableIRQ(TIM8_CC_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM9 == TRUE
    if (&ICUD9 == icup) {
      disableHB2(RCC_TIM9EN);
      NVIC_DisableIRQ(TIM9_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM10 == TRUE
    if (&ICUD10 == icup) {
      disableHB2(RCC_TIM10EN);
      NVIC_DisableIRQ(TIM10_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM11 == TRUE
    if (&ICUD11 == icup) {
      disableHB2(RCC_TIM11EN);
      NVIC_DisableIRQ(TIM11_IRQn);
    }
#endif

#if CH32_ICU_USE_TIM12 == TRUE
    if (&ICUD12 == icup) {
      disableHB2(RCC_TIM12EN);
      NVIC_DisableIRQ(TIM12_IRQn);
    }
#endif
  }
}

/**
 * @brief   Starts the input capture.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_start_capture(ICUDriver *icup) {
    icup->tim->DMAINTENR |= (TIM_UIE | TIM_CC1IE | TIM_CC2IE);
    icup->tim->SWEVGR = TIM_UG;
    icup->tim->INTFR = 0;

    icup->tim->CTLR1 = TIM_CEN;
    (void)icup;
}

/**
 * @brief   Waits for a completed capture.
 * @note    The operation is performed in polled mode.
 * @note    In order to use this function notifications must be disabled.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 * @return              The capture status.
 * @retval false        if the capture is successful.
 * @retval true         if a timer overflow occurred.
 *
 * @notapi
 */
bool icu_lld_wait_capture(ICUDriver *icup) {

  (void)icup;
  if (icup->tim->INTFR & TIM_CC1IF) {
    icup->tim->INTFR &= ~TIM_CC1IF;
    return true;
  }
  return false;
}

/**
 * @brief   Stops the input capture.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_stop_capture(ICUDriver *icup) {

  icup->tim->CTLR1 &= ~TIM_CEN;
  (void)icup;
}

/**
 * @brief   Enables notifications.
 * @pre     The ICU unit must have been activated using @p icuStart() and the
 *          capture started using @p icuStartCapture().
 * @note    If the notification is already enabled then the call has no effect.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @api
 */
void icu_lld_enable_notifications(ICUDriver *icup) {
    icup->tim->DMAINTENR |= (TIM_UIE | TIM_CC1IE | TIM_CC2IE);
    (void)icup;
}

/**
 * @brief   Disables notifications.
 * @pre     The ICU unit must have been activated using @p icuStart() and the
 *          capture started using @p icuStartCapture().
 * @note    If the notification is already disabled then the call has no effect.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @api
 */
void icu_lld_disable_notifications(ICUDriver *icup) {
    icup->tim->DMAINTENR &= ~(TIM_UIE | TIM_CC1IE | TIM_CC2IE);
    (void)icup;
}

#endif /* HAL_USE_ICU == TRUE */

/** @} */
