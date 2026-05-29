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
 * @file    hal_gpt_lld.c
 * @brief   CH32 GPT subsystem low level driver source.
 *
 * @addtogroup GPT
 * @{
 */

#include "hal.h"

#if (HAL_USE_GPT == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   GPTD1 driver identifier.
 * @note    The driver GPTD1 allocates the complex timer TIM1 when enabled.
 */
#if CH32_GPT_USE_TIM1 || defined(__DOXYGEN__)
GPTDriver GPTD1;
#endif

/**
 * @brief   GPTD2 driver identifier.
 * @note    The driver GPTD2 allocates the timer TIM2 when enabled.
 */
#if CH32_GPT_USE_TIM2 || defined(__DOXYGEN__)
GPTDriver GPTD2;
#endif

/**
 * @brief   GPTD3 driver identifier.
 * @note    The driver GPTD3 allocates the timer TIM3 when enabled.
 */
#if CH32_GPT_USE_TIM3 || defined(__DOXYGEN__)
GPTDriver GPTD3;
#endif

/**
 * @brief   GPTD4 driver identifier.
 * @note    The driver GPTD4 allocates the timer TIM4 when enabled.
 */
#if CH32_GPT_USE_TIM4 || defined(__DOXYGEN__)
GPTDriver GPTD4;
#endif

/**
 * @brief   GPTD5 driver identifier.
 * @note    The driver GPTD5 allocates the timer TIM5 when enabled.
 */
#if CH32_GPT_USE_TIM5 || defined(__DOXYGEN__)
GPTDriver GPTD5;
#endif

/**
 * @brief   GPTD6 driver identifier.
 * @note    The driver GPTD6 allocates the timer TIM6 when enabled.
 */
#if CH32_GPT_USE_TIM6 || defined(__DOXYGEN__)
GPTDriver GPTD6;
#endif

/**
 * @brief   GPTD7 driver identifier.
 * @note    The driver GPTD7 allocates the timer TIM7 when enabled.
 */
#if CH32_GPT_USE_TIM7 || defined(__DOXYGEN__)
GPTDriver GPTD7;
#endif

/**
 * @brief   GPTD8 driver identifier.
 * @note    The driver GPTD8 allocates the timer TIM8 when enabled.
 */
#if CH32_GPT_USE_TIM8 || defined(__DOXYGEN__)
GPTDriver GPTD8;
#endif

/**
 * @brief   GPTD9 driver identifier.
 * @note    The driver GPTD9 allocates the timer TIM9 when enabled.
 */
#if CH32_GPT_USE_TIM9 || defined(__DOXYGEN__)
GPTDriver GPTD9;
#endif

/**
 * @brief   GPTD10 driver identifier.
 * @note    The driver GPTD10 allocates the timer TIM10 when enabled.
 */
#if CH32_GPT_USE_TIM10 || defined(__DOXYGEN__)
GPTDriver GPTD10;
#endif

/**
 * @brief   GPTD11 driver identifier.
 * @note    The driver GPTD11 allocates the timer TIM11 when enabled.
 */
#if CH32_GPT_USE_TIM11 || defined(__DOXYGEN__)
GPTDriver GPTD11;
#endif

/**
 * @brief   GPTD12 driver identifier.
 * @note    The driver GPTD12 allocates the timer TIM12 when enabled.
 */
#if CH32_GPT_USE_TIM12 || defined(__DOXYGEN__)
GPTDriver GPTD12;
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
 * @brief   Shared IRQ handler.
 *
 * @param[in] gptp      pointer to a @p GPTDriver object
 *
 * @notapi
 */
void gpt_lld_serve_interrupt(GPTDriver *gptp)
{
    volatile uint32_t sr;

    sr = gptp->tim->INTFR;
    sr &= (gptp->tim->DMAINTENR &
           (TIM_UIE | TIM_CC1IE | TIM_CC2IE | TIM_CC3IE | TIM_CC4IE | TIM_COMIE | TIM_TIE | TIM_BIE));
    gptp->tim->INTFR = ~sr;
    if (sr & TIM_UIF)
    {
        _gpt_isr_invoke_cb(gptp);
    }
}
#if CH32_GPT_USE_TIM1 || defined(__DOXYGEN__)
/**
 * @brief   TIM1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM1_UP_IRQHandler) {

  gpt_lld_serve_interrupt(&GPTD1);

}
#endif /* CH32_GPT_USE_TIM1 */

#if CH32_GPT_USE_TIM2 || defined(__DOXYGEN__)
/**
 * @brief   TIM2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM2_IRQHandler) {

  gpt_lld_serve_interrupt(&GPTD2);

}
#endif /* CH32_GPT_USE_TIM2 */

#if CH32_GPT_USE_TIM3 || defined(__DOXYGEN__)
/**
 * @brief   TIM3 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM3_IRQHandler) {

  gpt_lld_serve_interrupt(&GPTD3);

}
#endif /* CH32_GPT_USE_TIM3 */

#if CH32_GPT_USE_TIM4 || defined(__DOXYGEN__)
/**
 * @brief   TIM4 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM4_IRQHandler) {

  gpt_lld_serve_interrupt(&GPTD4);

}
#endif /* CH32_GPT_USE_TIM4 */

#if CH32_GPT_USE_TIM5 || defined(__DOXYGEN__)
/**
 * @brief   TIM5 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM5_IRQHandler) {

  gpt_lld_serve_interrupt(&GPTD5);

}
#endif /* CH32_GPT_USE_TIM5 */

#if CH32_GPT_USE_TIM6 || defined(__DOXYGEN__)
/**
 * @brief   TIM6 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM6_IRQHandler) {

  gpt_lld_serve_interrupt(&GPTD6);

}
#endif /* CH32_GPT_USE_TIM6 */

#if CH32_GPT_USE_TIM7 || defined(__DOXYGEN__)
/**
 * @brief   TIM7 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM7_IRQHandler) {

  gpt_lld_serve_interrupt(&GPTD7);

}
#endif /* CH32_GPT_USE_TIM7 */

#if CH32_GPT_USE_TIM8 || defined(__DOXYGEN__)
/**
 * @brief   TIM8 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM8_UP_IRQHandler) {

  gpt_lld_serve_interrupt(&GPTD8);

}
#endif /* CH32_GPT_USE_TIM8 */

#if CH32_GPT_USE_TIM9 || defined(__DOXYGEN__)
/**
 * @brief   TIM9 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM9_IRQHandler) {

  gpt_lld_serve_interrupt(&GPTD9);

}
#endif /* CH32_GPT_USE_TIM9 */

#if CH32_GPT_USE_TIM10 || defined(__DOXYGEN__)
/**
 * @brief   TIM10 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM10_IRQHandler) {

  gpt_lld_serve_interrupt(&GPTD10);

}
#endif /* CH32_GPT_USE_TIM10 */

#if CH32_GPT_USE_TIM11 || defined(__DOXYGEN__)
/**
 * @brief   TIM11 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM11_IRQHandler) {

  gpt_lld_serve_interrupt(&GPTD11);

}
#endif /* CH32_GPT_USE_TIM11 */

#if CH32_GPT_USE_TIM12 || defined(__DOXYGEN__)
/**
 * @brief   TIM12 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM12_IRQHandler) {

  gpt_lld_serve_interrupt(&GPTD12);

}
#endif /* CH32_GPT_USE_TIM12 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level GPT driver initialization.
 *
 * @notapi
 */
void gpt_lld_init(void) {

#if CH32_GPT_USE_TIM1
  /* Driver initialization.*/
  GPTD1.tim = TIM1;
  gptObjectInit(&GPTD1);
#endif

#if CH32_GPT_USE_TIM2
  /* Driver initialization.*/
  GPTD2.tim = TIM2;
  gptObjectInit(&GPTD2);
#endif

#if CH32_GPT_USE_TIM3
  /* Driver initialization.*/
  GPTD3.tim = TIM3;
  gptObjectInit(&GPTD3);
#endif

#if CH32_GPT_USE_TIM4
  /* Driver initialization.*/
  GPTD4.tim = TIM4;
  gptObjectInit(&GPTD4);
#endif

#if CH32_GPT_USE_TIM5
  /* Driver initialization.*/
  GPTD5.tim = TIM5;
  gptObjectInit(&GPTD5);
#endif

#if CH32_GPT_USE_TIM6
  /* Driver initialization.*/
  GPTD6.tim = TIM6;
  gptObjectInit(&GPTD6);
#endif

#if CH32_GPT_USE_TIM7
  /* Driver initialization.*/
  GPTD7.tim = TIM7;
  gptObjectInit(&GPTD7);
#endif

#if CH32_GPT_USE_TIM8
  /* Driver initialization.*/
  GPTD8.tim = TIM8;
  gptObjectInit(&GPTD8);
#endif

#if CH32_GPT_USE_TIM9
  /* Driver initialization.*/
  GPTD9.tim = TIM9;
  gptObjectInit(&GPTD9);
#endif

#if CH32_GPT_USE_TIM10
  /* Driver initialization.*/
  GPTD10.tim = TIM10;
  gptObjectInit(&GPTD10);
#endif

#if CH32_GPT_USE_TIM11
  /* Driver initialization.*/
  GPTD11.tim = TIM11;
  gptObjectInit(&GPTD11);
#endif

#if CH32_GPT_USE_TIM12
  /* Driver initialization.*/
  GPTD12.tim = TIM12;
  gptObjectInit(&GPTD12);
#endif
}

/**
 * @brief   Configures and activates the GPT peripheral.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 *
 * @notapi
 */
void gpt_lld_start(GPTDriver *gptp) {
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq(&RCC_Clocks);
  uint32_t clock = RCC_Clocks.HCLK_Frequency;
  if (gptp->state == GPT_STOP) {
    /* Enables the peripheral.*/
#if CH32_GPT_USE_TIM1 == TRUE
    if (&GPTD1 == gptp) {
      enableHB2(RCC_TIM1EN);
      NVIC_EnableIRQ(TIM1_UP_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM2 == TRUE
    if (&GPTD2 == gptp) {
      enableHB1(RCC_TIM2EN);
      NVIC_EnableIRQ(TIM2_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM3 == TRUE
    if (&GPTD3 == gptp) {
      enableHB1(RCC_TIM3EN);
      NVIC_EnableIRQ(TIM3_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM4 == TRUE
    if (&GPTD4 == gptp) {
      enableHB1(RCC_TIM4EN);
      NVIC_EnableIRQ(TIM4_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM5 == TRUE
    if (&GPTD5 == gptp) {
      enableHB1(RCC_TIM5EN);
      NVIC_EnableIRQ(TIM5_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM6 == TRUE
    if (&GPTD6 == gptp) {
      enableHB1(RCC_TIM6EN);
      NVIC_EnableIRQ(TIM6_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM7 == TRUE
    if (&GPTD7 == gptp) {
      enableHB1(RCC_TIM7EN);
      NVIC_EnableIRQ(TIM7_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM8 == TRUE
    if (&GPTD8 == gptp) {
      enableHB2(RCC_TIM8EN);
      NVIC_EnableIRQ(TIM8_UP_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM9 == TRUE
    if (&GPTD9 == gptp) {
      enableHB2(RCC_TIM9EN);
      NVIC_EnableIRQ(TIM9_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM10 == TRUE
    if (&GPTD10 == gptp) {
      enableHB2(RCC_TIM10EN);
      NVIC_EnableIRQ(TIM10_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM11 == TRUE
    if (&GPTD11 == gptp) {
      enableHB2(RCC_TIM11EN);
      NVIC_EnableIRQ(TIM11_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM12 == TRUE
    if (&GPTD12 == gptp) {
      enableHB2(RCC_TIM12EN);
      NVIC_EnableIRQ(TIM12_IRQn);
    }
#endif
  }
  /* Configures the peripheral.*/
  uint32_t psc = (clock / gptp->config->frequency) - 1;
  if(psc > 0xffff)
    osalSysHalt("Cannot configure GPT: psc too large");
  gptp->tim->CTLR2 = 0;
  gptp->tim->CTLR1 = 0;
  gptp->tim->PSC = psc;
  gptp->tim->INTFR = 0;
  gptp->tim->DMAINTENR &= ~(TIM_UIE | TIM_CC1IE | TIM_CC2IE | TIM_CC3IE | TIM_CC4IE | TIM_COMIE | TIM_TIE | TIM_BIE);
}

/**
 * @brief   Deactivates the GPT peripheral.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 *
 * @notapi
 */
void gpt_lld_stop(GPTDriver *gptp) {

  if (gptp->state == GPT_READY) {
    /* Resets the peripheral.*/
    gptp->tim->CTLR1 = 0;
    gptp->tim->DMAINTENR = 0;
    gptp->tim->INTFR = 0;
    /* Disables the peripheral.*/
#if CH32_GPT_USE_TIM1 == TRUE
    if (&GPTD1 == gptp) {
      disableHB2(RCC_TIM1EN);
      NVIC_DisableIRQ(TIM1_UP_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM2 == TRUE
    if (&GPTD2 == gptp) {
      disableHB1(RCC_TIM2EN);
      NVIC_DisableIRQ(TIM2_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM3 == TRUE
    if (&GPTD3 == gptp) {
      disableHB1(RCC_TIM3EN);
      NVIC_DisableIRQ(TIM3_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM4 == TRUE
    if (&GPTD4 == gptp) {
      disableHB1(RCC_TIM4EN);
      NVIC_DisableIRQ(TIM4_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM5 == TRUE
    if (&GPTD5 == gptp) {
      disableHB1(RCC_TIM5EN);
      NVIC_DisableIRQ(TIM5_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM6 == TRUE
    if (&GPTD6 == gptp) {
      disableHB1(RCC_TIM6EN);
      NVIC_DisableIRQ(TIM6_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM7 == TRUE
    if (&GPTD7 == gptp) {
      disableHB1(RCC_TIM7EN);
      NVIC_DisableIRQ(TIM7_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM8 == TRUE
    if (&GPTD8 == gptp) {
      disableHB2(RCC_TIM8EN);
      NVIC_DisableIRQ(TIM8_UP_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM9 == TRUE
    if (&GPTD9 == gptp) {
      disableHB2(RCC_TIM9EN);
      NVIC_DisableIRQ(TIM9_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM10 == TRUE
    if (&GPTD10 == gptp) {
      disableHB2(RCC_TIM10EN);
      NVIC_DisableIRQ(TIM10_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM11 == TRUE
    if (&GPTD11 == gptp) {
      disableHB2(RCC_TIM11EN);
      NVIC_DisableIRQ(TIM11_IRQn);
    }
#endif

#if CH32_GPT_USE_TIM12 == TRUE
    if (&GPTD12 == gptp) {
      disableHB2(RCC_TIM12EN);
      NVIC_DisableIRQ(TIM12_IRQn);
    }
#endif
  }
}

/**
 * @brief   Starts the timer in continuous mode.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 * @param[in] interval  period in ticks
 *
 * @notapi
 */
void gpt_lld_start_timer(GPTDriver *gptp, gptcnt_t interval) {

  (void)gptp;
  (void)interval;
  gptp->tim->ATRLR_32 = interval - 1;
  gptp->tim->SWEVGR |= TIM_UG;
  (gptp)->tim->CNT_32 = 0;
  gptp->tim->INTFR = 0;
  if (NULL != gptp->config->callback)
    gptp->tim->DMAINTENR |= TIM_UIE;
  gptp->tim->CTLR1 = TIM_ARPE | TIM_URS | TIM_CEN;
}

/**
 * @brief   Stops the timer.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 *
 * @notapi
 */
void gpt_lld_stop_timer(GPTDriver *gptp) {

  (void)gptp;
  gptp->tim->CTLR1 = 0;
  gptp->tim->INTFR = 0;
  gptp->tim->DMAINTENR = ~(TIM_UIE | TIM_CC1IE | TIM_CC2IE | TIM_CC3IE | TIM_CC4IE | TIM_COMIE | TIM_TIE | TIM_BIE);
}

/**
 * @brief   Starts the timer in one shot mode and waits for completion.
 * @details This function specifically polls the timer waiting for completion
 *          in order to not have extra delays caused by interrupt servicing,
 *          this function is only recommended for short delays.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 * @param[in] interval  time interval in ticks
 *
 * @notapi
 */
void gpt_lld_polled_delay(GPTDriver *gptp, gptcnt_t interval) {

  (void)gptp;
  (void)interval;
  gptp->tim->CTLR1 = TIM_UDIS;
  gptp->tim->ATRLR_32 = interval - 1;
  gptp->tim->SWEVGR |= TIM_UG;
  gptp->tim->INTFR = 0;
  gptp->tim->CTLR1 = TIM_OPM | TIM_URS | TIM_CEN;
  while (!(gptp->tim->INTFR & TIM_UIF));
  gptp->tim->INTFR = 0;
}

#endif /* HAL_USE_GPT == TRUE */

/** @} */
