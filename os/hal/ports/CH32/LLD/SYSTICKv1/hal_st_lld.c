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
 * @file    hal_st_lld.c
 * @brief   CH32 ST subsystem low level driver source.
 *
 * @addtogroup ST
 * @{
 */

#include "hal.h"

#if (OSAL_ST_MODE != OSAL_ST_MODE_NONE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/
#if OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING

#define CH32_TIMCLK hal_lld_get_clock_point((halclkpt_t)NULL)


#if (OSAL_ST_RESOLUTION != 16) && (OSAL_ST_RESOLUTION != 32)
#error "unsupported ST resolution"
#endif

#if (OSAL_ST_RESOLUTION == 32)
#define ST_ARR_INIT                         0xFFFFFFFFU
#else
#define ST_ARR_INIT                         0x0000FFFFU
#endif

#if CH32_ST_USE_TIMER == 1

#if !CH32_HAS_TIM1
#error "TIM1 not present in the selected device"
#endif

#if (OSAL_ST_RESOLUTION == 32) && !CH32_TIM1_IS_32BITS
#error "TIM1 is not a 32bits timer"
#endif

#define ST_HANDLER                          TIM1_CC_IRQHandler
#define ST_NUMBER                           TIM1_CC_IRQn
#define ST_CLOCK_SRC                        CH32_TIMCLK
#define ST_ENABLE_CLOCK()                   enableHB2(RCC_TIM1EN)

#elif CH32_ST_USE_TIMER == 2

#if !CH32_HAS_TIM2
#error "TIM2 not present in the selected device"
#endif

#if (OSAL_ST_RESOLUTION == 32) && !CH32_TIM2_IS_32BITS
#error "TIM2 is not a 32bits timer"
#endif

#define ST_HANDLER                          TIM2_IRQHandler
#define ST_NUMBER                           TIM2_IRQn
#define ST_CLOCK_SRC                        CH32_TIMCLK
#define ST_ENABLE_CLOCK()                   enableHB1(RCC_TIM2EN)

#elif CH32_ST_USE_TIMER == 3

#if !CH32_HAS_TIM3
#error "TIM3 not present in the selected device"
#endif

#if (OSAL_ST_RESOLUTION == 32) && !CH32_TIM3_IS_32BITS
#error "TIM3 is not a 32bits timer"
#endif

#define ST_HANDLER                          TIM3_IRQHandler
#define ST_NUMBER                           TIM3_IRQn
#define ST_CLOCK_SRC                        CH32_TIMCLK
#define ST_ENABLE_CLOCK()                   enableHB1(RCC_TIM3EN)

#elif CH32_ST_USE_TIMER == 4

#if !CH32_HAS_TIM4
#error "TIM4 not present in the selected device"
#endif

#if (OSAL_ST_RESOLUTION == 32) && !CH32_TIM4_IS_32BITS
#error "TIM4 is not a 32bits timer"
#endif

#define ST_HANDLER                          TIM4_IRQHandler
#define ST_NUMBER                           TIM4_IRQn
#define ST_CLOCK_SRC                        CH32_TIMCLK
#define ST_ENABLE_CLOCK()                   enableHB1(RCC_TIM4EN)

#elif CH32_ST_USE_TIMER == 5

#if !CH32_HAS_TIM5
#error "TIM5 not present in the selected device"
#endif

#if (OSAL_ST_RESOLUTION == 32) && !CH32_TIM5_IS_32BITS
#error "TIM5 is not a 32bits timer"
#endif

#define ST_HANDLER                          TIM5_IRQHandler
#define ST_NUMBER                           TIM5_IRQn
#define ST_CLOCK_SRC                        CH32_TIMCLK
#define ST_ENABLE_CLOCK()                   enableHB1(RCC_TIM5EN)

#elif CH32_ST_USE_TIMER == 8

#if !CH32_HAS_TIM8
#error "TIM8 not present in the selected device"
#endif

#if (OSAL_ST_RESOLUTION == 32) && !CH32_TIM2_IS_32BITS
#error "TIM8 is not a 32bits timer"
#endif

#define ST_HANDLER                          TIM8_CC_IRQHandler
#define ST_NUMBER                           TIM8_CC_IRQn
#define ST_CLOCK_SRC                        CH32_TIMCLK
#define ST_ENABLE_CLOCK()                   enableHB2(RCC_TIM8EN)

#elif CH32_ST_USE_TIMER == 9

#if !CH32_HAS_TIM9
#error "TIM9 not present in the selected device"
#endif

#if (OSAL_ST_RESOLUTION == 32) && !CH32_TIM9_IS_32BITS
#error "TIM9 is not a 32bits timer"
#endif

#define ST_HANDLER                          TIM9_IRQHandler
#define ST_NUMBER                           TIM9_IRQn
#define ST_CLOCK_SRC                        CH32_TIMCLK
#define ST_ENABLE_CLOCK()                   enableHB2(RCC_TIM9EN)
#define ST_DBG_STOP()                       __set_DEBUG_CR(DBGMCU_TIM9_STOP)

#elif CH32_ST_USE_TIMER == 10

#if !CH32_HAS_TIM10
#error "TIM10 not present in the selected device"
#endif

#if (OSAL_ST_RESOLUTION == 32) && !CH32_TIM10_IS_32BITS
#error "TIM10 is not a 32bits timer"
#endif

#define ST_HANDLER                          TIM10_IRQHandler
#define ST_NUMBER                           TIM10_IRQn
#define ST_CLOCK_SRC                        CH32_TIMCLK
#define ST_ENABLE_CLOCK()                   enableHB2(RCC_TIM10EN)

#elif CH32_ST_USE_TIMER == 11

#if !CH32_HAS_TIM11
#error "TIM11 not present in the selected device"
#endif

#if (OSAL_ST_RESOLUTION == 32) && !CH32_TIM11_IS_32BITS
#error "TIM11 is not a 32bits timer"
#endif

#define ST_HANDLER                          TIM11_IRQHandler
#define ST_NUMBER                           TIM11_IRQn
#define ST_CLOCK_SRC                        CH32_TIMCLK
#define ST_ENABLE_CLOCK()                   enableHB2(RCC_TIM11EN)

#elif CH32_ST_USE_TIMER == 12

#if !CH32_HAS_TIM12
#error "TIM12 not present in the selected device"
#endif

#if (OSAL_ST_RESOLUTION == 32) && !CH32_TIM12_IS_32BITS
#error "TIM12 is not a 32bits timer"
#endif

#define ST_HANDLER                          TIM12_IRQHandler
#define ST_NUMBER                           TIM12_IRQn
#define ST_CLOCK_SRC                        CH32_TIMCLK
#define ST_ENABLE_CLOCK()                   enableHB2(RCC_TIM12EN)

#else
#error "CH32_ST_USE_TIMER specifies an unsupported timer"
#endif

#define DBGMCU_IWDG_STOP             ((uint32_t)0x00000100)
#define DBGMCU_WWDG_STOP             ((uint32_t)0x00000200)
#define DBGMCU_I2C1_SMBUS_TIMEOUT    ((uint32_t)0x00000400)
#define DBGMCU_I2C2_SMBUS_TIMEOUT    ((uint32_t)0x00000800)
#define DBGMCU_TIM1_STOP             ((uint32_t)0x00001000)
#define DBGMCU_TIM2_STOP             ((uint32_t)0x00002000)
#define DBGMCU_TIM3_STOP             ((uint32_t)0x00004000)
#define DBGMCU_TIM4_STOP             ((uint32_t)0x00008000)
#define DBGMCU_TIM5_STOP             ((uint32_t)0x00010000)
#define DBGMCU_TIM6_STOP             ((uint32_t)0x00020000)
#define DBGMCU_TIM7_STOP             ((uint32_t)0x00040000)
#define DBGMCU_TIM8_STOP             ((uint32_t)0x00080000)
#define DBGMCU_TIM9_STOP             ((uint32_t)0x00100000)
#define DBGMCU_TIM10_STOP            ((uint32_t)0x00200000)
#define DBGMCU_TIM11_STOP            ((uint32_t)0x00400000)
#define DBGMCU_TIM12_STOP            ((uint32_t)0x00800000)
#define DBGMCU_LPTIM1_STOP           ((uint32_t)0x01000000)
#define DBGMCU_LPTIM2_STOP           ((uint32_t)0x02000000)
#define DBGMCU_I2C3_SMBUS_TIMEOUT    ((uint32_t)0x04000000)
#define DBGMCU_I2C4_SMBUS_TIMEOUT    ((uint32_t)0x08000000)
#define DBGMCU_CAN1_STOP             ((uint32_t)0x10000000)
#define DBGMCU_CAN2_STOP             ((uint32_t)0x20000000)
#define DBGMCU_CAN3_STOP             ((uint32_t)0x40000000)

#endif

#if OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC

#define ST_HANDLER                          SysTick0_Handler
#define ST_NUMBER                           SysTick0_IRQn

#endif /* OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC */
/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
void __set_DEBUG_CR(uint32_t value)
{
    __asm volatile("csrw 0x7C0, %0" : : "r"(value));
}
/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/
void st_lld_serve_interrupt(void);
OSAL_IRQ_HANDLER(ST_HANDLER) {
  st_lld_serve_interrupt();
}
/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level ST driver initialization.
 *
 * @notapi
 */
void st_lld_init(void) {
#if OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING
    ST_DBG_STOP();
    ST_ENABLE_CLOCK();
    CH32_ST_TIM->PSC = ST_CLOCK_SRC / OSAL_ST_FREQUENCY - 1;
    CH32_ST_TIM->ATRLR_32    = ST_ARR_INIT;
    CH32_ST_TIM->CHCTLR1     = 0;
    CH32_ST_TIM->CH1CVR_32  = 0;
#if ST_LLD_NUM_ALARMS > 1
    CH32_ST_TIM->CH2CVR_32 = 0;
#endif
#if ST_LLD_NUM_ALARMS > 2
    CH32_ST_TIM->CH3CVR_32 = 0;
#endif
#if ST_LLD_NUM_ALARMS > 3
    CH32_ST_TIM->CH4CVR_32 = 0;
#endif
    CH32_ST_TIM->DMAINTENR = 0;
    CH32_ST_TIM->CTLR2 = 0;
    CH32_ST_TIM->SWEVGR = TIM_UG;
    CH32_ST_TIM->CTLR1 = TIM_CEN;
    NVIC_EnableIRQ( ST_NUMBER );
#elif OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC
    SysTick0->ISR &= ~(1 << 0);

    SysTick0->CNT = 0;
    SysTick0->CMP = CH32_TIMCLK / OSAL_ST_FREQUENCY - 1;
    SysTick0->CTLR = (1 << 2) | (1 << 1) | (1 << 3);
    SysTick0->CTLR |= (1 << 0);
    NVIC_EnableIRQ( ST_NUMBER );
#endif
}

void st_lld_serve_interrupt(void) {
#if OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING
    uint32_t sr;

    sr = CH32_ST_TIM->INTFR;
    sr &= CH32_ST_TIM->DMAINTENR &
          (TIM_UIE | TIM_CC1IE | TIM_CC2IE | TIM_CC3IE | TIM_CC4IE | TIM_COMIE | TIM_TIE | TIM_BIE | TIM_UDE);
    CH32_ST_TIM->INTFR = ~sr;

    if ((sr & TIM_CC1IF) != 0U)
#endif
  {
    osalSysLockFromISR();
    osalOsTimerHandlerI();
    osalSysUnlockFromISR();
  }
#if OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING
#if ST_LLD_NUM_ALARMS > 1
  if ((sr & TIM_CC2IF) != 0U) {
    if (st_callbacks[1] != NULL) {
      st_callbacks[1](1U);
    }
  }
#endif
#if ST_LLD_NUM_ALARMS > 2
  if ((sr & TIM_CC3IF) != 0U) {
    if (st_callbacks[2] != NULL) {
      st_callbacks[2](2U);
    }
  }
#endif
#if ST_LLD_NUM_ALARMS > 3
  if ((sr & TIM_CC4IF) != 0U) {
    if (st_callbacks[3] != NULL) {
      st_callbacks[3](3U);
    }
  }
#endif
#endif
}

#endif /* OSAL_ST_MODE != OSAL_ST_MODE_NONE */

/** @} */
