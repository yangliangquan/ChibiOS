/*
    ChibiOS - Copyright (C) 2006-2026 Giovanni Di Sirio.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at:

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    hal_serial_lld.c
 * @brief   CH32 serial subsystem low level driver source.
 *
 * @addtogroup SERIAL
 * @{
 */

#include "hal.h"

#if (HAL_USE_SERIAL == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/* Bits that the driver manages, forbidden in user configuration.*/
#define USART_CTLR1_CFG_FORBIDDEN           (USART_CTLR1_PEIE             |   \
                                             USART_CTLR1_TXEIE            |   \
                                             USART_CTLR1_TCIE             |   \
                                             USART_CTLR1_RXNEIE           |   \
                                             USART_CTLR1_IDLEIE           |   \
                                             USART_CTLR1_TE               |   \
                                             USART_CTLR1_RE               |   \
                                             USART_CTLR1_UE)

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief USART1 serial driver identifier.*/
#if (CH32_SERIAL_USE_USART1 == TRUE) || defined(__DOXYGEN__)
SerialDriver SD1;
#endif

/** @brief USART2 serial driver identifier.*/
#if (CH32_SERIAL_USE_USART2 == TRUE) || defined(__DOXYGEN__)
SerialDriver SD2;
#endif

/** @brief USART3 serial driver identifier.*/
#if (CH32_SERIAL_USE_USART3 == TRUE) || defined(__DOXYGEN__)
SerialDriver SD3;
#endif

/** @brief USART4 serial driver identifier.*/
#if (CH32_SERIAL_USE_USART4 == TRUE) || defined(__DOXYGEN__)
SerialDriver SD4;
#endif

/** @brief USART5 serial driver identifier.*/
#if (CH32_SERIAL_USE_USART5 == TRUE) || defined(__DOXYGEN__)
SerialDriver SD5;
#endif

/** @brief USART6 serial driver identifier.*/
#if (CH32_SERIAL_USE_USART6 == TRUE) || defined(__DOXYGEN__)
SerialDriver SD6;
#endif

/** @brief USART7 serial driver identifier.*/
#if (CH32_SERIAL_USE_USART7 == TRUE) || defined(__DOXYGEN__)
SerialDriver SD7;
#endif

/** @brief USART8 serial driver identifier.*/
#if (CH32_SERIAL_USE_USART8 == TRUE) || defined(__DOXYGEN__)
SerialDriver SD8;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver default configuration.
 */
static const SerialConfig default_config = {
  SERIAL_DEFAULT_BITRATE,
  0,
  USART_CTLR2_STOP1_BITS,
  0
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   USART initialization.
 * @details This function must be invoked with interrupts disabled.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 * @param[in] config    the architecture-dependent serial driver configuration
 */
static void usart_init(SerialDriver *sdp, const SerialConfig *config) {
  uint32_t brr, clock;
  USART_TypeDef *u = sdp->usart;

  /* Prescaler calculation.*/
  clock = sdp->clock;

  /* Baud rate setting.
     CH32 does not support OVER8 mode (no USART_CTLR1_OVER8 bit).*/
  brr = (uint32_t)((clock + config->baud / 2) / config->baud);

  osalDbgAssert(brr < 0x10000, "invalid BRR value");
  
  u->BRR = (uint16_t)brr;

  /* Note that some bits are enforced.*/
  u->CTLR2  = config->cr2 | USART_CTLR2_LBDIE;
  u->CTLR3  = config->cr3 | USART_CTLR3_EIE;
  u->CTLR1  = config->cr1 | USART_CTLR1_UE | USART_CTLR1_PEIE |
                               USART_CTLR1_RXNEIE | USART_CTLR1_TE |
                               USART_CTLR1_RE;

  /* Clearing all pending status flags (rc_w0: write 0 to clear).*/
  u->STATR = 0x0000U;

  /* Dummy read to clear ORE.*/
  (void)u->DATAR;

  /* Deciding mask to be applied on the data register on receive, this is
     required in order to mask out the parity bit.*/
  if ((config->cr1 & (USART_CTLR1_M | USART_CTLR1_PCE)) == USART_CTLR1_PCE) {
    sdp->rxmask = 0x7F;
  }
  else {
    sdp->rxmask = 0xFF;
  }
}

/**
 * @brief   USART de-initialization.
 * @details This function must be invoked with interrupts disabled.
 *
 * @param[in] u         pointer to an USART I/O block
 */
static void usart_deinit(USART_TypeDef *u) {

  u->CTLR1 = 0;
  u->CTLR2 = 0;
  u->CTLR3 = 0;
}

/**
 * @brief   Error handling routine.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 * @param[in] statr     USART STATR register value
 */
static void set_error(SerialDriver *sdp, uint16_t statr) {
  eventflags_t sts = 0;

  if (statr & USART_STATR_ORE)
    sts |= SD_OVERRUN_ERROR;
  if (statr & USART_STATR_PE)
    sts |= SD_PARITY_ERROR;
  if (statr & USART_STATR_FE)
    sts |= SD_FRAMING_ERROR;
  if (statr & USART_STATR_NE)
    sts |= SD_NOISE_ERROR;
  chnAddFlagsI(sdp, sts);
}

#if CH32_SERIAL_USE_USART1 || defined(__DOXYGEN__)
static void notify1(io_queue_t *qp) {

  (void)qp;
  USART1->CTLR1 |= USART_CTLR1_TXEIE | USART_CTLR1_TCIE;
}
#endif

#if CH32_SERIAL_USE_USART2 || defined(__DOXYGEN__)
static void notify2(io_queue_t *qp) {

  (void)qp;
  USART2->CTLR1 |= USART_CTLR1_TXEIE | USART_CTLR1_TCIE;
}
#endif

#if CH32_SERIAL_USE_USART3 || defined(__DOXYGEN__)
static void notify3(io_queue_t *qp) {

  (void)qp;
  USART3->CTLR1 |= USART_CTLR1_TXEIE | USART_CTLR1_TCIE;
}
#endif

#if CH32_SERIAL_USE_USART4 || defined(__DOXYGEN__)
static void notify4(io_queue_t *qp) {

  (void)qp;
  USART4->CTLR1 |= USART_CTLR1_TXEIE | USART_CTLR1_TCIE;
}
#endif

#if CH32_SERIAL_USE_USART5 || defined(__DOXYGEN__)
static void notify5(io_queue_t *qp) {

  (void)qp;
  USART5->CTLR1 |= USART_CTLR1_TXEIE | USART_CTLR1_TCIE;
}
#endif

#if CH32_SERIAL_USE_USART6 || defined(__DOXYGEN__)
static void notify6(io_queue_t *qp) {

  (void)qp;
  USART6->CTLR1 |= USART_CTLR1_TXEIE | USART_CTLR1_TCIE;
}
#endif

#if CH32_SERIAL_USE_USART7 || defined(__DOXYGEN__)
static void notify7(io_queue_t *qp) {

  (void)qp;
  USART7->CTLR1 |= USART_CTLR1_TXEIE | USART_CTLR1_TCIE;
}
#endif

#if CH32_SERIAL_USE_USART8 || defined(__DOXYGEN__)
static void notify8(io_queue_t *qp) {

  (void)qp;
  USART8->CTLR1 |= USART_CTLR1_TXEIE | USART_CTLR1_TCIE;
}
#endif

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if CH32_SERIAL_USE_USART1 || defined(__DOXYGEN__)
#if !defined(CH32_USART1_SUPPRESS_ISR)
/**
 * @brief   USART1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(USART1_IRQHandler) {

  sd_lld_serve_interrupt(&SD1);

}
#endif
#endif

#if CH32_SERIAL_USE_USART2 || defined(__DOXYGEN__)
#if !defined(CH32_USART2_SUPPRESS_ISR)
/**
 * @brief   USART2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(USART2_IRQHandler) {

  sd_lld_serve_interrupt(&SD2);

}
#endif
#endif

#if CH32_SERIAL_USE_USART3 || defined(__DOXYGEN__)
#if !defined(CH32_USART3_SUPPRESS_ISR)
/**
 * @brief   USART3 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(USART3_IRQHandler) {

  sd_lld_serve_interrupt(&SD3);

}
#endif
#endif

#if CH32_SERIAL_USE_USART4 || defined(__DOXYGEN__)
#if !defined(CH32_USART4_SUPPRESS_ISR)
/**
 * @brief   USART4 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(USART4_IRQHandler) {

  sd_lld_serve_interrupt(&SD4);

}
#endif
#endif

#if CH32_SERIAL_USE_USART5 || defined(__DOXYGEN__)
#if !defined(CH32_USART5_SUPPRESS_ISR)
/**
 * @brief   USART5 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(USART5_IRQHandler) {

  sd_lld_serve_interrupt(&SD5);

}
#endif
#endif

#if CH32_SERIAL_USE_USART6 || defined(__DOXYGEN__)
#if !defined(CH32_USART6_SUPPRESS_ISR)
/**
 * @brief   USART6 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(USART6_IRQHandler) {

  sd_lld_serve_interrupt(&SD6);

}
#endif
#endif

#if CH32_SERIAL_USE_USART7 || defined(__DOXYGEN__)
#if !defined(CH32_USART7_SUPPRESS_ISR)
/**
 * @brief   USART7 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(USART7_IRQHandler) {

  sd_lld_serve_interrupt(&SD7);

}
#endif
#endif

#if CH32_SERIAL_USE_USART8 || defined(__DOXYGEN__)
#if !defined(CH32_USART8_SUPPRESS_ISR)
/**
 * @brief   USART8 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(USART8_IRQHandler) {

  sd_lld_serve_interrupt(&SD8);

}
#endif
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level serial driver initialization.
 *
 * @notapi
 */
void sd_lld_init(void) {

#if CH32_SERIAL_USE_USART1
  sdObjectInit(&SD1, NULL, notify1);
  SD1.usart = USART1;
  SD1.clock  = hal_lld_get_clock_point(0U);
#endif

#if CH32_SERIAL_USE_USART2
  sdObjectInit(&SD2, NULL, notify2);
  SD2.usart = USART2;
  SD2.clock  = hal_lld_get_clock_point(0U);
#endif

#if CH32_SERIAL_USE_USART3
  sdObjectInit(&SD3, NULL, notify3);
  SD3.usart = USART3;
  SD3.clock  = hal_lld_get_clock_point(0U);
#endif

#if CH32_SERIAL_USE_USART4
  sdObjectInit(&SD4, NULL, notify4);
  SD4.usart = USART4;
  SD4.clock  = hal_lld_get_clock_point(0U);
#endif

#if CH32_SERIAL_USE_USART5
  sdObjectInit(&SD5, NULL, notify5);
  SD5.usart = USART5;
  SD5.clock  = hal_lld_get_clock_point(0U);
#endif

#if CH32_SERIAL_USE_USART6
  sdObjectInit(&SD6, NULL, notify6);
  SD6.usart = USART6;
  SD6.clock  = hal_lld_get_clock_point(0U);
#endif

#if CH32_SERIAL_USE_USART7
  sdObjectInit(&SD7, NULL, notify7);
  SD7.usart = USART7;
  SD7.clock  = hal_lld_get_clock_point(0U);
#endif

#if CH32_SERIAL_USE_USART8
  sdObjectInit(&SD8, NULL, notify8);
  SD8.usart = USART8;
  SD8.clock  = hal_lld_get_clock_point(0U);
#endif
}

/**
 * @brief   Low level serial driver configuration and (re)start.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 * @param[in] config    the architecture-dependent serial driver configuration.
 *                      If this parameter is set to @p NULL then a default
 *                      configuration is used.
 *
 * @notapi
 */
void sd_lld_start(SerialDriver *sdp, const SerialConfig *config) {

  if (config == NULL) {
    config = &default_config;
  }

  if (sdp->state == SD_STOP) {
#if CH32_SERIAL_USE_USART1
    if (&SD1 == sdp) {
      resetHB2(RCC_USART1RST);
      enableHB2(RCC_USART1EN);
      NVIC_EnableIRQ(USART1_IRQn);
    }
#endif
#if CH32_SERIAL_USE_USART2
    if (&SD2 == sdp) {
      resetHB1(RCC_USART2RST);
      enableHB1(RCC_USART2EN);
      NVIC_EnableIRQ(USART2_IRQn);
    }
#endif
#if CH32_SERIAL_USE_USART3
    if (&SD3 == sdp) {
      resetHB1(RCC_USART3RST);
      enableHB1(RCC_USART3EN);
      NVIC_EnableIRQ(USART3_IRQn);
    }
#endif
#if CH32_SERIAL_USE_USART4
    if (&SD4 == sdp) {
      resetHB1(RCC_USART4RST);
      enableHB1(RCC_USART4EN);
      NVIC_EnableIRQ(USART4_IRQn);
    }
#endif
#if CH32_SERIAL_USE_USART5
    if (&SD5 == sdp) {
      resetHB1(RCC_USART5RST);
      enableHB1(RCC_USART5EN);
      NVIC_EnableIRQ(USART5_IRQn);
    }
#endif
#if CH32_SERIAL_USE_USART6
    if (&SD6 == sdp) {
      resetHB1(RCC_USART6RST);
      enableHB1(RCC_USART6EN);
      NVIC_EnableIRQ(USART6_IRQn);
    }
#endif
#if CH32_SERIAL_USE_USART7
    if (&SD7 == sdp) {
      resetHB1(RCC_USART7RST);
      enableHB1(RCC_USART7EN);
      NVIC_EnableIRQ(USART7_IRQn);
    }
#endif
#if CH32_SERIAL_USE_USART8
    if (&SD8 == sdp) {
      resetHB1(RCC_USART8RST);
      enableHB1(RCC_USART8EN);
      NVIC_EnableIRQ(USART8_IRQn);
    }
#endif
  }

  usart_init(sdp, config);
}

/**
 * @brief   Low level serial driver stop.
 * @details De-initializes the USART, stops the associated clock, resets the
 *          interrupt vector.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 *
 * @notapi
 */
void sd_lld_stop(SerialDriver *sdp) {

  if (sdp->state == SD_READY) {
#if CH32_SERIAL_USE_USART1
    if (&SD1 == sdp) {
      usart_deinit(sdp->usart);
      resetHB2(RCC_USART1RST);
      disableHB2(RCC_USART1EN);
      NVIC_DisableIRQ(USART1_IRQn);
      return;
    }
#endif
#if CH32_SERIAL_USE_USART2
    if (&SD2 == sdp) {
      usart_deinit(sdp->usart);
      resetHB1(RCC_USART2RST);
      disableHB1(RCC_USART2EN);
      NVIC_DisableIRQ(USART2_IRQn);
      return;
    }
#endif
#if CH32_SERIAL_USE_USART3
    if (&SD3 == sdp) {
      usart_deinit(sdp->usart);
      resetHB1(RCC_USART3RST);
      disableHB1(RCC_USART3EN);
      NVIC_DisableIRQ(USART3_IRQn);
      return;
    }
#endif
#if CH32_SERIAL_USE_USART4
    if (&SD4 == sdp) {
      usart_deinit(sdp->usart);
      resetHB1(RCC_USART4RST);
      disableHB1(RCC_USART4EN);
      NVIC_DisableIRQ(USART4_IRQn);
      return;
    }
#endif
#if CH32_SERIAL_USE_USART5
    if (&SD5 == sdp) {
      usart_deinit(sdp->usart);
      resetHB1(RCC_USART5RST);
      disableHB1(RCC_USART5EN);
      NVIC_DisableIRQ(USART5_IRQn);
      return;
    }
#endif
#if CH32_SERIAL_USE_USART6
    if (&SD6 == sdp) {
      usart_deinit(sdp->usart);
      resetHB1(RCC_USART6RST);
      disableHB1(RCC_USART6EN);
      NVIC_DisableIRQ(USART6_IRQn);
      return;
    }
#endif
#if CH32_SERIAL_USE_USART7
    if (&SD7 == sdp) {
      usart_deinit(sdp->usart);
      resetHB1(RCC_USART7RST);
      disableHB1(RCC_USART7EN);
      NVIC_DisableIRQ(USART7_IRQn);
      return;
    }
#endif
#if CH32_SERIAL_USE_USART8
    if (&SD8 == sdp) {
      usart_deinit(sdp->usart);
      resetHB1(RCC_USART8RST);
      disableHB1(RCC_USART8EN);
      NVIC_DisableIRQ(USART8_IRQn);
      return;
    }
#endif
  }
}

/**
 * @brief   Common IRQ handler.
 *
 * @param[in] sdp       communication channel associated to the USART
 */
void sd_lld_serve_interrupt(SerialDriver *sdp) {
  USART_TypeDef *u = sdp->usart;
  uint16_t cr1;
  uint16_t statr;

  /* Special case, LIN break detection.*/
  statr = u->STATR;
  if (statr & USART_STATR_LBD) {
    osalSysLockFromISR();
    chnAddFlagsI(sdp, SD_BREAK_DETECTED);
    /* Clear LBD flag (rc_w0: write 0 to clear).*/
    u->STATR = (uint16_t)(~USART_STATR_LBD);
    osalSysUnlockFromISR();
  }

  /* Data available.*/
  osalSysLockFromISR();
  statr = u->STATR;
  while (statr & (USART_STATR_RXNE | USART_STATR_ORE | USART_STATR_NE |
               USART_STATR_FE  | USART_STATR_PE)) {
    uint8_t b;

    /* Error condition detection.*/
    if (statr & (USART_STATR_ORE | USART_STATR_NE | USART_STATR_FE |
                 USART_STATR_PE)) {
      set_error(sdp, statr);
    }

    /* Note: For CH32, reading DATAR also clears the ORE flag.*/
    b = (uint8_t)u->DATAR & sdp->rxmask;
    if (statr & USART_STATR_RXNE) {
      sdIncomingDataI(sdp, b);
    }

    statr = u->STATR;
  }
  osalSysUnlockFromISR();

  /* Caching CTLR1.*/
  cr1 = u->CTLR1;

  /* Transmission buffer empty.*/
  statr = u->STATR;
  if ((cr1 & USART_CTLR1_TXEIE) && (statr & USART_STATR_TXE)) {
    msg_t b;
    osalSysLockFromISR();
    b = oqGetI(&sdp->oqueue);
    if (b < MSG_OK) {
      chnAddFlagsI(sdp, CHN_OUTPUT_EMPTY);
      cr1 &= ~USART_CTLR1_TXEIE;
    }
    else {
      u->DATAR = b;
    }
    osalSysUnlockFromISR();
  }

  /* Physical transmission end.*/
  statr = u->STATR;
  if ((cr1 & USART_CTLR1_TCIE) && (statr & USART_STATR_TC)) {
    osalSysLockFromISR();
    if (oqIsEmptyI(&sdp->oqueue)) {
      chnAddFlagsI(sdp, CHN_TRANSMISSION_END);
      cr1 &= ~USART_CTLR1_TCIE;
    }
    osalSysUnlockFromISR();
  }

  /* Writing CTLR1 once.*/
  u->CTLR1 = cr1;
}

#endif /* HAL_USE_SERIAL == TRUE */

/** @} */
