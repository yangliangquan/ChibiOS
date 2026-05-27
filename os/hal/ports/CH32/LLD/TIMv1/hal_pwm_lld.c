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
 * @file    hal_pwm_lld.c
 * @brief   CH32 PWM subsystem low level driver source.
 *
 * @addtogroup PWM
 * @{
 */

#include "hal.h"

#if (HAL_USE_PWM == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   PWMD1 driver identifier.
 * @note    The driver PWMD1 allocates the complex timer TIM1 when enabled.
 */
#if CH32_PWM_USE_TIM1 || defined(__DOXYGEN__)
PWMDriver PWMD1;
#endif

/**
 * @brief   PWMD2 driver identifier.
 * @note    The driver PWMD2 allocates the timer TIM2 when enabled.
 */
#if CH32_PWM_USE_TIM2 || defined(__DOXYGEN__)
PWMDriver PWMD2;
#endif

/**
 * @brief   PWMD3 driver identifier.
 * @note    The driver PWMD3 allocates the timer TIM3 when enabled.
 */
#if CH32_PWM_USE_TIM3 || defined(__DOXYGEN__)
PWMDriver PWMD3;
#endif

/**
 * @brief   PWMD4 driver identifier.
 * @note    The driver PWMD4 allocates the timer TIM4 when enabled.
 */
#if CH32_PWM_USE_TIM4 || defined(__DOXYGEN__)
PWMDriver PWMD4;
#endif

/**
 * @brief   PWMD5 driver identifier.
 * @note    The driver PWMD5 allocates the timer TIM5 when enabled.
 */
#if CH32_PWM_USE_TIM5 || defined(__DOXYGEN__)
PWMDriver PWMD5;
#endif

/**
 * @brief   PWMD8 driver identifier.
 * @note    The driver PWMD8 allocates the timer TIM8 when enabled.
 */
#if CH32_PWM_USE_TIM8 || defined(__DOXYGEN__)
PWMDriver PWMD8;
#endif

/**
 * @brief   PWMD9 driver identifier.
 * @note    The driver PWMD9 allocates the timer TIM9 when enabled.
 */
#if CH32_PWM_USE_TIM9 || defined(__DOXYGEN__)
PWMDriver PWMD9;
#endif

/**
 * @brief   PWMD10 driver identifier.
 * @note    The driver PWMD10 allocates the timer TIM10 when enabled.
 */
#if CH32_PWM_USE_TIM10 || defined(__DOXYGEN__)
PWMDriver PWMD10;
#endif

/**
 * @brief   PWMD11 driver identifier.
 * @note    The driver PWMD11 allocates the timer TIM11 when enabled.
 */
#if CH32_PWM_USE_TIM11 || defined(__DOXYGEN__)
PWMDriver PWMD11;
#endif

/**
 * @brief   PWMD12 driver identifier.
 * @note    The driver PWMD12 allocates the timer TIM12 when enabled.
 */
#if CH32_PWM_USE_TIM12 || defined(__DOXYGEN__)
PWMDriver PWMD12;
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
/**
 * @brief   Common TIM2...TIM5,TIM9 IRQ handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_serve_interrupt(PWMDriver *pwmp)
{
    uint32_t sr;

    sr = pwmp->tim->INTFR;
    sr &= (pwmp->tim->DMAINTENR &
           (TIM_UIE | TIM_CC1IE | TIM_CC2IE | TIM_CC3IE | TIM_CC4IE | TIM_COMIE | TIM_TIE | TIM_BIE));
    pwmp->tim->INTFR = ~sr;
    if (((sr & TIM_CC1IF) != 0) && (pwmp->config->channels[0].callback != NULL))
        pwmp->config->channels[0].callback(pwmp);
    if (((sr & TIM_CC2IF) != 0) && (pwmp->config->channels[1].callback != NULL))
        pwmp->config->channels[1].callback(pwmp);
    if (((sr & TIM_CC3IF) != 0) && (pwmp->config->channels[2].callback != NULL))
        pwmp->config->channels[2].callback(pwmp);
    if (((sr & TIM_CC4IF) != 0) && (pwmp->config->channels[3].callback != NULL))
        pwmp->config->channels[3].callback(pwmp);
    if (((sr & TIM_UIF) != 0) && (pwmp->config->callback != NULL))
        pwmp->config->callback(pwmp);
}
#if CH32_PWM_USE_TIM1 || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(TIM1_UP_IRQHandler) {

  pwm_lld_serve_interrupt(&PWMD1);

}

OSAL_IRQ_HANDLER(TIM1_CC_IRQHandler) {

  pwm_lld_serve_interrupt(&PWMD1);

}
#endif /* CH32_PWM_USE_TIM1 */

#if CH32_PWM_USE_TIM2 || defined(__DOXYGEN__)
/**
 * @brief   TIM2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM2_IRQHandler) {

  pwm_lld_serve_interrupt(&PWMD2);

}
#endif /* CH32_PWM_USE_TIM2 */

#if CH32_PWM_USE_TIM3 || defined(__DOXYGEN__)
/**
 * @brief   TIM3 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM3_IRQHandler) {

  pwm_lld_serve_interrupt(&PWMD3);

}
#endif /* CH32_PWM_USE_TIM3 */

#if CH32_PWM_USE_TIM4 || defined(__DOXYGEN__)
/**
 * @brief   TIM4 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM4_IRQHandler) {

  pwm_lld_serve_interrupt(&PWMD4);

}
#endif /* CH32_PWM_USE_TIM4 */

#if CH32_PWM_USE_TIM5 || defined(__DOXYGEN__)
/**
 * @brief   TIM5 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM5_IRQHandler) {

  pwm_lld_serve_interrupt(&PWMD5);

}
#endif /* CH32_PWM_USE_TIM5 */

#if CH32_PWM_USE_TIM8 || defined(__DOXYGEN__)
/**
 * @brief   TIM8 update interrupt handler.
 * @note    It is assumed that this interrupt is only activated if the callback
 *          pointer is not equal to @p NULL in order to not perform an extra
 *          check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM8_UP_IRQHandler) {

  pwm_lld_serve_interrupt(&PWMD8);

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

  pwm_lld_serve_interrupt(&PWMD8);

}
#endif /* CH32_PWM_USE_TIM8 */

#if CH32_PWM_USE_TIM9 || defined(__DOXYGEN__)
/**
 * @brief   TIM9 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM9_IRQHandler) {

  pwm_lld_serve_interrupt(&PWMD9);

}
#endif /* CH32_PWM_USE_TIM9 */

#if CH32_PWM_USE_TIM10 || defined(__DOXYGEN__)
/**
 * @brief   TIM10 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM10_IRQHandler) {

  pwm_lld_serve_interrupt(&PWMD10);

}
#endif /* CH32_PWM_USE_TIM10 */

#if CH32_PWM_USE_TIM11 || defined(__DOXYGEN__)
/**
 * @brief   TIM11 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM11_IRQHandler) {

  pwm_lld_serve_interrupt(&PWMD11);

}
#endif /* CH32_PWM_USE_TIM11 */

#if CH32_PWM_USE_TIM12 || defined(__DOXYGEN__)
/**
 * @brief   TIM12 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM12_IRQHandler) {

  pwm_lld_serve_interrupt(&PWMD12);

}
#endif /* CH32_PWM_USE_TIM12 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level PWM driver initialization.
 *
 * @notapi
 */
void pwm_lld_init(void) {

#if CH32_PWM_USE_TIM1
  /* Driver initialization.*/
  pwmObjectInit(&PWMD1);
  PWMD1.channels = CH32_TIM1_CHANNELS;
  PWMD1.tim = TIM1;
  PWMD1.has_bdtr = true;
#endif

#if CH32_PWM_USE_TIM2
  /* Driver initialization.*/
  pwmObjectInit(&PWMD2);
  PWMD2.channels = CH32_TIM2_CHANNELS;
  PWMD2.tim = TIM2;
  PWMD2.has_bdtr = false;
#endif

#if CH32_PWM_USE_TIM3
  /* Driver initialization.*/
  pwmObjectInit(&PWMD3);
  PWMD3.channels = CH32_TIM3_CHANNELS;
  PWMD3.tim = TIM3;
  PWMD3.has_bdtr = false;
#endif

#if CH32_PWM_USE_TIM4
  /* Driver initialization.*/
  pwmObjectInit(&PWMD4);
  PWMD4.channels = CH32_TIM4_CHANNELS;
  PWMD4.tim = TIM4;
  PWMD4.has_bdtr = false;
#endif

#if CH32_PWM_USE_TIM5
  /* Driver initialization.*/
  pwmObjectInit(&PWMD5);
  PWMD5.channels = CH32_TIM5_CHANNELS;
  PWMD5.tim = TIM5;
  PWMD5.has_bdtr = false;
#endif

#if CH32_PWM_USE_TIM8
  /* Driver initialization.*/
  pwmObjectInit(&PWMD8);
  PWMD8.channels = CH32_TIM8_CHANNELS;
  PWMD8.tim = TIM8;
  PWMD8.has_bdtr = true;
#endif

#if CH32_PWM_USE_TIM9
  /* Driver initialization.*/
  pwmObjectInit(&PWMD9);
  PWMD9.channels = CH32_TIM9_CHANNELS;
  PWMD9.tim = TIM9;
  PWMD9.has_bdtr = false;
#endif

#if CH32_PWM_USE_TIM10
  /* Driver initialization.*/
  pwmObjectInit(&PWMD10);
  PWMD10.channels = CH32_TIM10_CHANNELS;
  PWMD10.tim = TIM10;
  PWMD10.has_bdtr = false;
#endif

#if CH32_PWM_USE_TIM11
  /* Driver initialization.*/
  pwmObjectInit(&PWMD11);
  PWMD11.channels = CH32_TIM11_CHANNELS;
  PWMD11.tim = TIM11;
  PWMD11.has_bdtr = false;
#endif

#if CH32_PWM_USE_TIM12
  /* Driver initialization.*/
  pwmObjectInit(&PWMD12);
  PWMD12.channels = CH32_TIM12_CHANNELS;
  PWMD12.tim = TIM12;
  PWMD12.has_bdtr = false;
#endif
}

/**
 * @brief   Configures and activates the PWM peripheral.
 * @note    Starting a driver that is already in the @p PWM_READY state
 *          disables all the active channels.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_start(PWMDriver *pwmp) {
  uint32_t clock = hal_lld_get_clock_point((halclkpt_t)NULL);
  if (pwmp->state == PWM_STOP) {
    /* Clock activation and timer reset.*/
#if CH32_PWM_USE_TIM1 == TRUE
    if (&PWMD1 == pwmp) {
      enableHB2(RCC_TIM1EN);
      NVIC_EnableIRQ(TIM1_UP_IRQn);
      NVIC_EnableIRQ(TIM1_CC_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM2 == TRUE
    if (&PWMD2 == pwmp) {
      enableHB1(RCC_TIM2EN);
      NVIC_EnableIRQ(TIM2_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM3 == TRUE
    if (&PWMD3 == pwmp) {
      enableHB1(RCC_TIM3EN);
      NVIC_EnableIRQ(TIM3_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM4 == TRUE
    if (&PWMD4 == pwmp) {
      enableHB1(RCC_TIM4EN);
      NVIC_EnableIRQ(TIM4_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM5 == TRUE
    if (&PWMD5 == pwmp) {
      enableHB1(RCC_TIM5EN);
      NVIC_EnableIRQ(TIM5_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM6 == TRUE
    if (&PWMD6 == pwmp) {
      enableHB1(RCC_TIM6EN);
      NVIC_EnableIRQ(TIM6_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM7 == TRUE
    if (&PWMD7 == pwmp) {
      enableHB1(RCC_TIM7EN);
      NVIC_EnableIRQ(TIM7_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM8 == TRUE
    if (&PWMD8 == pwmp) {
      enableHB2(RCC_TIM8EN);
      NVIC_EnableIRQ(TIM8_UP_IRQn);
      NVIC_EnableIRQ(TIM8_CC_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM9 == TRUE
    if (&PWMD9 == pwmp) {
      enableHB2(RCC_TIM9EN);
      NVIC_EnableIRQ(TIM9_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM10 == TRUE
    if (&PWMD10 == pwmp) {
      enableHB2(RCC_TIM10EN);
      NVIC_EnableIRQ(TIM10_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM11 == TRUE
    if (&PWMD11 == pwmp) {
      enableHB2(RCC_TIM11EN);
      NVIC_EnableIRQ(TIM11_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM12 == TRUE
    if (&PWMD12 == pwmp) {
      enableHB2(RCC_TIM12EN);
      NVIC_EnableIRQ(TIM12_IRQn);
    }
#endif
    pwmp->tim->CHCTLR1 = (((6 << 4) & TIM_OC1M) | ((6 << 12) & TIM_OC2M))| TIM_OC1PE|TIM_OC2PE;  
    pwmp->tim->CHCTLR2 = (((6 << 4) & TIM_OC3M) | ((6 << 12) & TIM_OC4M))|TIM_OC3PE|TIM_OC4PE;
  }
  else
  {
    /* If the timer is already running then stop it and restart it to update
       all the registers.*/

    pwmp->tim->CTLR1 = 0;
    pwmp->tim->CH1CVR_32 = 0;
    pwmp->tim->CH2CVR_32 = 0;
    pwmp->tim->CH3CVR_32 = 0;
    pwmp->tim->CH4CVR_32 = 0;
    pwmp->tim->CNT_32 = 0;
  }
  uint32_t psc = (clock / pwmp->config->frequency) - 1;
  if(psc > 0xffff)
    osalSysHalt("Cannot configure GPT: psc too large");
  pwmp->tim->PSC = psc;
  pwmp->tim->ATRLR_32 = pwmp->config->period - 1;
  pwmp->tim->CTLR2 = 0;
  uint16_t ccer = 0;
  for (int i = 0; i < pwmp->channels; i++) { 
    switch (pwmp->config->channels[i].mode & PWM_OUTPUT_MASK) {
      case PWM_OUTPUT_ACTIVE_HIGH:
        ccer |= TIM_CC1P << (i * 4);
      // Fall through
      case PWM_OUTPUT_ACTIVE_LOW:
        ccer |= TIM_CC1E << (i * 4);
        break;
      default:
        break;
    }
  }

  if(pwmp->has_bdtr){
    for (int i = 0; i < pwmp->channels; i++) {
      switch (pwmp->config->channels[i].mode & PWM_COMPLEMENTARY_OUTPUT_MASK) {
        case PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH:
          ccer |= TIM_CC1NP << (i * 4);
        // Fall through
        case PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW:
          ccer |= TIM_CC1NE << (i * 4);
          break;
        default:
          break;
      }
    }
  }

  pwmp->tim->CCER = ccer;
  pwmp->tim->SWEVGR |= TIM_UG;
  pwmp->tim->INTFR = 0;
  pwmp->tim->DMAINTENR = (pwmp->config->dma_settings << 8) & ~(TIM_CC1IE | TIM_CC2IE | TIM_CC3IE |
                                               TIM_CC4IE | TIM_UIE);

  if(pwmp->has_bdtr){
    pwmp->tim->BDTR |= TIM_MOE;
  }
  pwmp->tim->CTLR1 = TIM_CEN|TIM_ARPE|TIM_URS;
}

/**
 * @brief   Deactivates the PWM peripheral.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_stop(PWMDriver *pwmp) {

  /* If in ready state then disables the PWM clock.*/
  pwmp->tim->CTLR1 = 0;
  pwmp->tim->DMAINTENR = 0;
  pwmp->tim->INTFR = 0;
  if(pwmp->has_bdtr){
    pwmp->tim->BDTR  = 0;
  }
  if (pwmp->state == PWM_READY) {
#if CH32_PWM_USE_TIM1 == TRUE
    if (&PWMD1 == pwmp) {
      disableHB2(RCC_TIM1EN);
      NVIC_DisableIRQ(TIM1_UP_IRQn);
      NVIC_DisableIRQ(TIM1_CC_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM2 == TRUE
    if (&PWMD2 == pwmp) {
      disableHB1(RCC_TIM2EN);
      NVIC_DisableIRQ(TIM2_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM3 == TRUE
    if (&PWMD3 == pwmp) {
      disableHB1(RCC_TIM3EN);
      NVIC_DisableIRQ(TIM3_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM4 == TRUE
    if (&PWMD4 == pwmp) {
      disableHB1(RCC_TIM4EN);
      NVIC_DisableIRQ(TIM4_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM5 == TRUE
    if (&PWMD5 == pwmp) {
      disableHB1(RCC_TIM5EN);
      NVIC_DisableIRQ(TIM5_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM6 == TRUE
    if (&PWMD6 == pwmp) {
      disableHB1(RCC_TIM6EN);
      NVIC_DisableIRQ(TIM6_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM7 == TRUE
    if (&PWMD7 == pwmp) {
      disableHB1(RCC_TIM7EN);
      NVIC_DisableIRQ(TIM7_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM8 == TRUE
    if (&PWMD8 == pwmp) {
      disableHB2(RCC_TIM8EN);
      NVIC_DisableIRQ(TIM8_UP_IRQn);
      NVIC_DisableIRQ(TIM8_CC_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM9 == TRUE
    if (&PWMD9 == pwmp) {
      disableHB2(RCC_TIM9EN);
      NVIC_DisableIRQ(TIM9_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM10 == TRUE
    if (&PWMD10 == pwmp) {
      disableHB2(RCC_TIM10EN);
      NVIC_DisableIRQ(TIM10_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM11 == TRUE
    if (&PWMD11 == pwmp) {
      disableHB2(RCC_TIM11EN);
      NVIC_DisableIRQ(TIM11_IRQn);
    }
#endif

#if CH32_PWM_USE_TIM12 == TRUE
    if (&PWMD12 == pwmp) {
      disableHB2(RCC_TIM12EN);
      NVIC_DisableIRQ(TIM12_IRQn);
    }
#endif

  }
}

/**
 * @brief   Enables a PWM channel.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @post    The channel is active using the specified configuration.
 * @note    The function has effect at the next cycle start.
 * @note    Channel notification is not enabled.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...channels-1)
 * @param[in] width     PWM pulse width as clock pulses number
 *
 * @notapi
 */
void pwm_lld_enable_channel(PWMDriver *pwmp,
                            pwmchannel_t channel,
                            pwmcnt_t width) {

  (void)pwmp;
  (void)channel;
  (void)width;
  if(channel == 0){
    pwmp->tim->CH1CVR_32 = width;
  } else if(channel == 1){
    pwmp->tim->CH2CVR_32 = width;
  } else if(channel == 2){
    pwmp->tim->CH3CVR_32 = width;
  } else if(channel == 3){
    pwmp->tim->CH4CVR_32 = width;
  }
}

/**
 * @brief   Disables a PWM channel and its notification.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @post    The channel is disabled and its output line returned to the
 *          idle state.
 * @note    The function has effect at the next cycle start.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...channels-1)
 *
 * @notapi
 */
void pwm_lld_disable_channel(PWMDriver *pwmp, pwmchannel_t channel) {

  (void)pwmp;
  (void)channel;
  pwmp->tim->DMAINTENR &= ~(TIM_CC1E<<channel);
  if(channel == 0){
    pwmp->tim->CH1CVR_32 = 0;
  } else if(channel == 1){
    pwmp->tim->CH2CVR_32 = 0;
  } else if(channel == 2){
    pwmp->tim->CH3CVR_32 = 0;
  } else if(channel == 3){
    pwmp->tim->CH4CVR_32 = 0;
  }
}

/**
 * @brief   Enables the periodic activation edge notification.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @note    If the notification is already enabled then the call has no effect.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_enable_periodic_notification(PWMDriver *pwmp) {

  (void)pwmp;
  if(pwmp->tim->DMAINTENR & TIM_UDE){
    return;
  }else{
    pwmp->tim->INTFR &= ~TIM_UIF;
    pwmp->tim->DMAINTENR |= TIM_UDE;
  }
}

/**
 * @brief   Disables the periodic activation edge notification.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @note    If the notification is already disabled then the call has no effect.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_disable_periodic_notification(PWMDriver *pwmp) {

  (void)pwmp;
  pwmp->tim->DMAINTENR &= ~TIM_UDE;
}

/**
 * @brief   Enables a channel de-activation edge notification.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @pre     The channel must have been activated using @p pwmEnableChannel().
 * @note    If the notification is already enabled then the call has no effect.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...channels-1)
 *
 * @notapi
 */
void pwm_lld_enable_channel_notification(PWMDriver *pwmp,
                                         pwmchannel_t channel) {

  (void)pwmp;
  (void)channel;
  if(pwmp->tim->DMAINTENR & (TIM_CC1E<<channel)){
    return;
  }else{
    pwmp->tim->INTFR &= ~(TIM_CC1IF<<channel);
    pwmp->tim->DMAINTENR |= (TIM_CC1E<<channel);
  }
}

/**
 * @brief   Disables a channel de-activation edge notification.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @pre     The channel must have been activated using @p pwmEnableChannel().
 * @note    If the notification is already disabled then the call has no effect.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...channels-1)
 *
 * @notapi
 */
void pwm_lld_disable_channel_notification(PWMDriver *pwmp,
                                          pwmchannel_t channel) {

  (void)pwmp;
  (void)channel;
  pwmp->tim->DMAINTENR &= ~(TIM_CC1E<<channel);
}

#endif /* HAL_USE_PWM == TRUE */

/** @} */
