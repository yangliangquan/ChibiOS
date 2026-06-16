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
 * @file    hal_can_lld.c
 * @brief   CH32 CAN subsystem low level driver source.
 *
 * @addtogroup CAN
 * @{
 */

#include "hal.h"

#if (HAL_USE_CAN == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*
 * Addressing differences in the headers, they seem unable to agree on names.
 */
#if CH32_CAN_USE_CAN1
#if !defined(CAN1)
#define CAN1 CAN
#endif
#endif

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief CAN1 driver identifier.*/
#if CH32_CAN_USE_CAN1 || defined(__DOXYGEN__)
CANDriver CAND1;
#endif

/** @brief CAN2 driver identifier.*/
#if CH32_CAN_USE_CAN2 || defined(__DOXYGEN__)
CANDriver CAND2;
#endif

/** @brief CAN3 driver identifier.*/
#if CH32_CAN_USE_CAN3 || defined(__DOXYGEN__)
CANDriver CAND3;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Programs the filters of CAN 1.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 * @param[in] can2sb    number of the first filter assigned to CAN2 (unused)
 * @param[in] num       number of entries in the filters array, if zero then
 *                      a default filter is programmed
 * @param[in] cfp       pointer to the filters array, can be @p NULL if
 *                      (num == 0)
 *
 * @notapi
 */
static void can_lld_set_filters(CANDriver *canp,
                                uint32_t can2sb,
                                uint32_t num,
                                const CANFilter *cfp) {

  (void)can2sb;

  /* Temporarily enabling CAN clock.*/
#if CH32_CAN_USE_CAN1
  if (canp == &CAND1) {
    enableHB1(RCC_CAN1EN);
  }
#endif
#if CH32_CAN_USE_CAN3
  if (canp == &CAND3) {
    enableHB1(RCC_CAN3EN);
  }
#endif

  /* Filters initialization.*/
  canp->can->FCTLR |= CAN_FCTLR_TINIT;

  if (num > 0) {
    uint32_t i, fmask;

    /* All filters cleared.*/
#if CH32_CAN_USE_CAN1
    if (canp == &CAND1) {
      canp->can->FWR = 0;
      canp->can->FMCFGR = 0;
      canp->can->FSCFGR = 0;
      canp->can->FAFIFOR = 0;
    }
#endif
#if CH32_CAN_USE_CAN2
    if (canp == &CAND2) {
      canp->can->FWR = 0;
      canp->can->FMCFGR = 0;
      canp->can->FSCFGR = 0;
      canp->can->FAFIFOR = 0;
    }
#endif
#if CH32_CAN_USE_CAN3
    if (canp == &CAND3) {
      canp->can->FWR_CAN3 = 0;
      canp->can->FMCFGR_CAN3 = 0;
      canp->can->FSCFGR_CAN3 = 0;
      canp->can->FAFIFOR_CAN3 = 0;
    }
#endif

    for (i = 0; i < 42; i++) {
      canp->can->sFilterRegister[i].FR1 = 0;
      canp->can->sFilterRegister[i].FR2 = 0;
    }

    /* Scanning the filters array.*/
    for (i = 0; i < num; i++) {
      fmask = 1U << cfp->filter;
#if CH32_CAN_USE_CAN1
      if (canp == &CAND1) {
        if (cfp->mode)
          canp->can->FMCFGR |= fmask;
        if (cfp->scale)
          canp->can->FSCFGR |= fmask;
        if (cfp->assignment)
          canp->can->FAFIFOR |= fmask;
        canp->can->FWR |= fmask;
      }
#endif
#if CH32_CAN_USE_CAN2
      if (canp == &CAND2) {
        if (cfp->mode)
          canp->can->FMCFGR |= fmask;
        if (cfp->scale)
          canp->can->FSCFGR |= fmask;
        if (cfp->assignment)
          canp->can->FAFIFOR |= fmask;
        canp->can->FWR |= fmask;
      }
#endif
#if CH32_CAN_USE_CAN3
      if (canp == &CAND3) {
        if (cfp->mode)
          canp->can->FMCFGR_CAN3 |= fmask;
        if (cfp->scale)
          canp->can->FSCFGR_CAN3 |= fmask;
        if (cfp->assignment)
          canp->can->FAFIFOR_CAN3 |= fmask;
        canp->can->FWR_CAN3 |= fmask;
      }
#endif
      canp->can->sFilterRegister[cfp->filter].FR1 = cfp->register1;
      canp->can->sFilterRegister[cfp->filter].FR2 = cfp->register2;
      cfp++;
    }
  }
  else {
#if CH32_CAN_USE_CAN1
    if (canp == &CAND1) {
      /* Setting up a single default filter that enables everything.*/
      canp->can->sFilterRegister[0].FR1 = 0;
      canp->can->sFilterRegister[0].FR2 = 0;
      canp->can->FMCFGR = 0;
      canp->can->FAFIFOR = 0;
      canp->can->FSCFGR = 1;
      canp->can->FWR = 1;
    }
#endif
#if CH32_CAN_USE_CAN2
    if (canp == &CAND2) {
      /* Setting up a single default filter that enables everything.*/
      canp->can->sFilterRegister[0].FR1 = 0;
      canp->can->sFilterRegister[0].FR2 = 0;
      canp->can->FMCFGR = 0;
      canp->can->FAFIFOR = 0;
      canp->can->FSCFGR = 1;
      canp->can->FWR = 1;
    }
#endif
#if CH32_CAN_USE_CAN3
    if (canp == &CAND3) {
      canp->can->sFilterRegister[0].FR1 = 0;
      canp->can->sFilterRegister[0].FR2 = 0;
      canp->can->FMCFGR_CAN3 = 0;
      canp->can->FAFIFOR_CAN3 = 0;
      canp->can->FSCFGR_CAN3 = 1;
      canp->can->FWR_CAN3 = 1;
    }
#endif
  }

  canp->can->FCTLR &= ~CAN_FCTLR_TINIT;

  /* Clock disabled, it will be enabled again in can_lld_start().*/
#if CH32_CAN_USE_CAN1
  if (canp == &CAND1) {
    disableHB1(RCC_CAN1EN);
  }
#endif
#if CH32_CAN_USE_CAN3
  if (canp == &CAND3) {
    disableHB1(RCC_CAN3EN);
  }
#endif
}

/**
 * @brief   Common TX ISR handler.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
static void can_lld_tx_handler(CANDriver *canp) {
  uint32_t tsr;
  eventflags_t flags;

  /* Clearing IRQ sources.*/
  tsr = canp->can->TSTATR;
  canp->can->TSTATR = tsr;

  /* Flags to be signaled through the TX event source.*/
  flags = 0U;

  /* Checking mailbox 0.*/
  if ((tsr & CAN_TSTATR_RQCP0) != 0U) {
    if ((tsr & (CAN_TSTATR_ALST0 | CAN_TSTATR_TERR0)) != 0U) {
      flags |= CAN_MAILBOX_TO_MASK(1U) << 16U;
    }
    else {
      flags |= CAN_MAILBOX_TO_MASK(1U);
    }
  }

  /* Checking mailbox 1.*/
  if ((tsr & CAN_TSTATR_RQCP1) != 0U) {
    if ((tsr & (CAN_TSTATR_ALST1 | CAN_TSTATR_TERR1)) != 0U) {
      flags |= CAN_MAILBOX_TO_MASK(2U) << 16U;
    }
    else {
      flags |= CAN_MAILBOX_TO_MASK(2U);
    }
  }

  /* Checking mailbox 2.*/
  if ((tsr & CAN_TSTATR_RQCP2) != 0U) {
    if ((tsr & (CAN_TSTATR_ALST2 | CAN_TSTATR_TERR2)) != 0U) {
      flags |= CAN_MAILBOX_TO_MASK(3U) << 16U;
    }
    else {
      flags |= CAN_MAILBOX_TO_MASK(3U);
    }
  }

  /* Signaling flags and waking up threads waiting for a transmission slot.*/
  _can_tx_empty_isr(canp, flags);
}

/**
 * @brief   Common RX0 ISR handler.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
static void can_lld_rx0_handler(CANDriver *canp) {
  uint32_t rf0r;

  rf0r = canp->can->RFIFO0;
  if ((rf0r & CAN_RFIFO0_FMP0) > 0) {
    /* No more receive events until the queue 0 has been emptied.*/
    canp->can->INTENR &= ~CAN_INTENR_FMPIE0;
    _can_rx_full_isr(canp, CAN_MAILBOX_TO_MASK(1U));
  }
  if ((rf0r & CAN_RFIFO0_FOVR0) > 0) {
    /* Overflow events handling.*/
    canp->can->RFIFO0 = CAN_RFIFO0_FOVR0;
    _can_error_isr(canp, CAN_OVERFLOW_ERROR);
  }
}

/**
 * @brief   Common RX1 ISR handler.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
static void can_lld_rx1_handler(CANDriver *canp) {
  uint32_t rf1r;

  rf1r = canp->can->RFIFO1;
  if ((rf1r & CAN_RFIFO1_FMP1) > 0) {
    /* No more receive events until the queue 0 has been emptied.*/
    canp->can->INTENR &= ~CAN_INTENR_EMPIE1;
    _can_rx_full_isr(canp, CAN_MAILBOX_TO_MASK(2U));
  }
  if ((rf1r & CAN_RFIFO1_FOVR1) > 0) {
    /* Overflow events handling.*/
    canp->can->RFIFO1 = CAN_RFIFO1_FOVR1;
    _can_error_isr(canp, CAN_OVERFLOW_ERROR);
  }
}

/**
 * @brief   Common SCE ISR handler.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
static void can_lld_sce_handler(CANDriver *canp) {
  uint32_t statr;

  /* Clearing IRQ sources.*/
  statr = canp->can->STATR;
  canp->can->STATR = statr;

  /* Wakeup event.*/
#if CAN_USE_SLEEP_MODE
  if (statr & CAN_STATR_WKUI) {
    canp->state = CAN_READY;
    canp->can->CTLR &= ~CAN_CTLR_SLEEP;
    _can_wakeup_isr(canp);
  }
#endif /* CAN_USE_SLEEP_MODE */

  /* Error event.*/
  if (statr & CAN_STATR_ERRI) {
    eventflags_t flags;
    uint32_t errsr = canp->can->ERRSR;

#if CH32_CAN_REPORT_ALL_ERRORS
    flags = (eventflags_t)(errsr & 7);
    if ((errsr & CAN_ERRSR_LEC) > 0)
      flags |= CAN_FRAMING_ERROR;
#else
    flags = 0;
#endif

    /* The content of the ERRSR register is copied unchanged in the upper
       half word of the listener flags mask.*/
    _can_error_isr(canp, flags | (eventflags_t)(errsr << 16U));
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if CH32_CAN_USE_CAN1 || defined(__DOXYGEN__)
/**
 * @brief   CAN1 TX interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CAN1_TX_IRQHandler) {

  can_lld_tx_handler(&CAND1);
}

/**
 * @brief   CAN1 RX0 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CAN1_RX0_IRQHandler) {

  can_lld_rx0_handler(&CAND1);
}

/**
 * @brief   CAN1 RX1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CAN1_RX1_IRQHandler) {

  can_lld_rx1_handler(&CAND1);
}

/**
 * @brief   CAN1 SCE interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CAN1_SCE_IRQHandler) {

  can_lld_sce_handler(&CAND1);
}
#endif /* CH32_CAN_USE_CAN1 */

#if CH32_CAN_USE_CAN2 || defined(__DOXYGEN__)
/**
 * @brief   CAN2 TX interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CAN2_TX_IRQHandler) {

  can_lld_tx_handler(&CAND2);
}

/**
 * @brief   CAN2 RX0 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CAN2_RX0_IRQHandler) {

  can_lld_rx0_handler(&CAND2);
}

/**
 * @brief   CAN2 RX1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CAN2_RX1_IRQHandler) {

  can_lld_rx1_handler(&CAND2);
}

/**
 * @brief   CAN2 SCE interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CAN2_SCE_IRQHandler) {

  can_lld_sce_handler(&CAND2);
}
#endif /* CH32_CAN_USE_CAN2 */

#if CH32_CAN_USE_CAN3 || defined(__DOXYGEN__)
/**
 * @brief   CAN3 TX interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CAN3_TX_IRQHandler) {

  can_lld_tx_handler(&CAND3);
}

/**
 * @brief   CAN3 RX0 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CAN3_RX0_IRQHandler) {

  can_lld_rx0_handler(&CAND3);
}

/**
 * @brief   CAN3 RX1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CAN3_RX1_IRQHandler) {

  can_lld_rx1_handler(&CAND3);
}

/**
 * @brief   CAN3 SCE interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CAN3_SCE_IRQHandler) {

  can_lld_sce_handler(&CAND3);
}
#endif /* CH32_CAN_USE_CAN3 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level CAN driver initialization.
 *
 * @notapi
 */
void can_lld_init(void) {

#if CH32_CAN_USE_CAN1
  /* Driver initialization.*/
  canObjectInit(&CAND1);
  CAND1.can = CAN1;

  /* NVIC interrupt enabling for CAN1.*/
  NVIC->IPRIOR[CAN1_TX_IRQn]  = CH32_CAN_CAN1_IRQ_PRIORITY;
  NVIC->IPRIOR[CAN1_RX0_IRQn] = CH32_CAN_CAN1_IRQ_PRIORITY;
  NVIC->IPRIOR[CAN1_RX1_IRQn] = CH32_CAN_CAN1_IRQ_PRIORITY;
  NVIC->IPRIOR[CAN1_SCE_IRQn] = CH32_CAN_CAN1_IRQ_PRIORITY;
  NVIC->IENR[CAN1_TX_IRQn >> 5]  |= (1U << (CAN1_TX_IRQn & 0x1F));
  NVIC->IENR[CAN1_RX0_IRQn >> 5] |= (1U << (CAN1_RX0_IRQn & 0x1F));
  NVIC->IENR[CAN1_RX1_IRQn >> 5] |= (1U << (CAN1_RX1_IRQn & 0x1F));
  NVIC->IENR[CAN1_SCE_IRQn >> 5] |= (1U << (CAN1_SCE_IRQn & 0x1F));
#endif

#if CH32_CAN_USE_CAN2
  /* Driver initialization.*/
  canObjectInit(&CAND2);
  CAND2.can = CAN2;

  /* NVIC interrupt enabling for CAN2.*/
  NVIC->IPRIOR[CAN2_TX_IRQn]  = CH32_CAN_CAN2_IRQ_PRIORITY;
  NVIC->IPRIOR[CAN2_RX0_IRQn] = CH32_CAN_CAN2_IRQ_PRIORITY;
  NVIC->IPRIOR[CAN2_RX1_IRQn] = CH32_CAN_CAN2_IRQ_PRIORITY;
  NVIC->IPRIOR[CAN2_SCE_IRQn] = CH32_CAN_CAN2_IRQ_PRIORITY;
  NVIC->IENR[CAN2_TX_IRQn >> 5]  |= (1U << (CAN2_TX_IRQn & 0x1F));
  NVIC->IENR[CAN2_RX0_IRQn >> 5] |= (1U << (CAN2_RX0_IRQn & 0x1F));
  NVIC->IENR[CAN2_RX1_IRQn >> 5] |= (1U << (CAN2_RX1_IRQn & 0x1F));
  NVIC->IENR[CAN2_SCE_IRQn >> 5] |= (1U << (CAN2_SCE_IRQn & 0x1F));
#endif

#if CH32_CAN_USE_CAN3
  /* Driver initialization.*/
  canObjectInit(&CAND3);
  CAND3.can = CAN3;

  /* NVIC interrupt enabling for CAN3.*/
  NVIC->IPRIOR[CAN3_TX_IRQn]  = CH32_CAN_CAN3_IRQ_PRIORITY;
  NVIC->IPRIOR[CAN3_RX0_IRQn] = CH32_CAN_CAN3_IRQ_PRIORITY;
  NVIC->IPRIOR[CAN3_RX1_IRQn] = CH32_CAN_CAN3_IRQ_PRIORITY;
  NVIC->IPRIOR[CAN3_SCE_IRQn] = CH32_CAN_CAN3_IRQ_PRIORITY;
  NVIC->IENR[CAN3_TX_IRQn >> 5]  |= (1U << (CAN3_TX_IRQn & 0x1F));
  NVIC->IENR[CAN3_RX0_IRQn >> 5] |= (1U << (CAN3_RX0_IRQn & 0x1F));
  NVIC->IENR[CAN3_RX1_IRQn >> 5] |= (1U << (CAN3_RX1_IRQn & 0x1F));
  NVIC->IENR[CAN3_SCE_IRQn >> 5] |= (1U << (CAN3_SCE_IRQn & 0x1F));
#endif

  /* Filters initialization.*/
#if CH32_CAN_USE_CAN1
  can_lld_set_filters(&CAND1, 0, 0, NULL);
#endif
#if CH32_CAN_USE_CAN3
  can_lld_set_filters(&CAND3, 0, 0, NULL);
#endif
}

/**
 * @brief   Configures and activates the CAN peripheral.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
void can_lld_start(CANDriver *canp) {

  /* Clock activation.*/
#if CH32_CAN_USE_CAN1
  if (&CAND1 == canp) {
    enableHB1(RCC_CAN1EN);
  }
#endif
#if CH32_CAN_USE_CAN2
  if (&CAND2 == canp) {
    enableHB1(RCC_CAN1EN);    /* CAN2 requires CAN1, so enabling it first.*/
    enableHB1(RCC_CAN2EN);
  }
#endif
#if CH32_CAN_USE_CAN3
  if (&CAND3 == canp) {
    enableHB1(RCC_CAN3EN);
  }
#endif

  /* Configuring CAN. */
  canp->can->CTLR = CAN_CTLR_INRQ;
  while ((canp->can->STATR & CAN_STATR_INAK) == 0)
    osalThreadSleepS(1);
  canp->can->BTIMR = canp->config->btr;
  canp->can->CTLR = canp->config->mcr;

  /* Interrupt sources initialization.*/
#if CH32_CAN_REPORT_ALL_ERRORS
  canp->can->INTENR = CAN_INTENR_TMEIE  | CAN_INTENR_FMPIE0 |
                      CAN_INTENR_EMPIE1 | CAN_INTENR_WKUIE  |
                      CAN_INTENR_ERRIE  | CAN_INTENR_LECIE  |
                      CAN_INTENR_BOFIE  | CAN_INTENR_EPVIE  |
                      CAN_INTENR_EWGIE  | CAN_INTENR_FOVIE0 |
                      CAN_INTENR_FOVIE1;
#else
  canp->can->INTENR = CAN_INTENR_TMEIE  | CAN_INTENR_FMPIE0 |
                      CAN_INTENR_EMPIE1 | CAN_INTENR_WKUIE  |
                      CAN_INTENR_ERRIE  |
                      CAN_INTENR_BOFIE  | CAN_INTENR_EPVIE  |
                      CAN_INTENR_EWGIE  | CAN_INTENR_FOVIE0 |
                      CAN_INTENR_FOVIE1;
#endif
}

/**
 * @brief   Deactivates the CAN peripheral.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
void can_lld_stop(CANDriver *canp) {

  /* If in ready state then disables the CAN peripheral.*/
  if (canp->state == CAN_READY) {
#if CH32_CAN_USE_CAN1
    if (&CAND1 == canp) {
      CAN1->CTLR = 0x00010002;                     /* Register reset value. */
      CAN1->INTENR = 0x00000000;                   /* All sources disabled. */
#if CH32_CAN_USE_CAN2
      /* If CAND2 is stopped then CAN1 clock is stopped here.*/
      if (CAND2.state == CAN_STOP)
#endif
      {
        disableHB1(RCC_CAN1EN);
      }
    }
#endif
#if CH32_CAN_USE_CAN2
    if (&CAND2 == canp) {
      CAN2->CTLR = 0x00010002;                     /* Register reset value. */
      CAN2->INTENR = 0x00000000;                   /* All sources disabled. */
#if CH32_CAN_USE_CAN1
      /* If CAND1 is stopped then CAN1 clock is stopped here.*/
      if (CAND1.state == CAN_STOP)
#endif
      {
        disableHB1(RCC_CAN1EN);
      }
      disableHB1(RCC_CAN2EN);
    }
#endif
#if CH32_CAN_USE_CAN3
    if (&CAND3 == canp) {
      CAN3->CTLR = 0x00010002;                     /* Register reset value. */
      CAN3->INTENR = 0x00000000;                   /* All sources disabled. */
      disableHB1(RCC_CAN3EN);
    }
#endif
  }
}

/**
 * @brief   Determines whether a frame can be transmitted.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 * @param[in] mailbox   mailbox number, @p CAN_ANY_MAILBOX for any mailbox
 *
 * @return              The queue space availability.
 * @retval false        no space in the transmit queue.
 * @retval true         transmit slot available.
 *
 * @notapi
 */
bool can_lld_is_tx_empty(CANDriver *canp, canmbx_t mailbox) {

  switch (mailbox) {
  case CAN_ANY_MAILBOX:
    return (canp->can->TSTATR & CAN_TSTATR_TME) != 0;
  case 1:
    return (canp->can->TSTATR & CAN_TSTATR_TME0) != 0;
  case 2:
    return (canp->can->TSTATR & CAN_TSTATR_TME1) != 0;
  case 3:
    return (canp->can->TSTATR & CAN_TSTATR_TME2) != 0;
  default:
    return false;
  }
}

/**
 * @brief   Inserts a frame into the transmit queue.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 * @param[in] ctfp      pointer to the CAN frame to be transmitted
 * @param[in] mailbox   mailbox number,  @p CAN_ANY_MAILBOX for any mailbox
 *
 * @notapi
 */
void can_lld_transmit(CANDriver *canp,
                      canmbx_t mailbox,
                      const CANTxFrame *ctfp) {
  uint32_t tir;
  CAN_TxMailBox_TypeDef *tmbp;

  /* Pointer to a free transmission mailbox.*/
  switch (mailbox) {
  case CAN_ANY_MAILBOX:
    tmbp = &canp->can->sTxMailBox[(canp->can->TSTATR & CAN_TSTATR_CODE) >> 24];
    break;
  case 1:
    tmbp = &canp->can->sTxMailBox[0];
    break;
  case 2:
    tmbp = &canp->can->sTxMailBox[1];
    break;
  case 3:
    tmbp = &canp->can->sTxMailBox[2];
    break;
  default:
    return;
  }

  /* Preparing the message.*/
  if (ctfp->IDE)
    tir = ((uint32_t)ctfp->EID << 3) | ((uint32_t)ctfp->RTR << 1) |
          CAN_TXMI0R_IDE;
  else
    tir = ((uint32_t)ctfp->SID << 21) | ((uint32_t)ctfp->RTR << 1);
  tmbp->TXMDTR = ctfp->DLC;
  tmbp->TXMDLR = ctfp->data32[0];
  tmbp->TXMDHR = ctfp->data32[1];
  tmbp->TXMIR  = tir | CAN_TXMI0R_TXRQ;
}

/**
 * @brief   Determines whether a frame has been received.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 * @param[in] mailbox   mailbox number, @p CAN_ANY_MAILBOX for any mailbox
 *
 * @return              The queue space availability.
 * @retval false        no space in the transmit queue.
 * @retval true         transmit slot available.
 *
 * @notapi
 */
bool can_lld_is_rx_nonempty(CANDriver *canp, canmbx_t mailbox) {

  switch (mailbox) {
  case CAN_ANY_MAILBOX:
    return ((canp->can->RFIFO0 & CAN_RFIFO0_FMP0) != 0 ||
            (canp->can->RFIFO1 & CAN_RFIFO1_FMP1) != 0);
  case 1:
    return (canp->can->RFIFO0 & CAN_RFIFO0_FMP0) != 0;
  case 2:
    return (canp->can->RFIFO1 & CAN_RFIFO1_FMP1) != 0;
  default:
    return false;
  }
}

/**
 * @brief   Receives a frame from the input queue.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 * @param[in] mailbox   mailbox number, @p CAN_ANY_MAILBOX for any mailbox
 * @param[out] crfp     pointer to the buffer where the CAN frame is copied
 *
 * @notapi
 */
void can_lld_receive(CANDriver *canp,
                     canmbx_t mailbox,
                     CANRxFrame *crfp) {
  uint32_t rir, rdtr;

  if (mailbox == CAN_ANY_MAILBOX) {
    if ((canp->can->RFIFO0 & CAN_RFIFO0_FMP0) != 0)
      mailbox = 1;
    else if ((canp->can->RFIFO1 & CAN_RFIFO1_FMP1) != 0)
      mailbox = 2;
    else {
      /* Should not happen, do nothing.*/
      return;
    }
  }
  switch (mailbox) {
  case 1:
    /* Fetches the message.*/
    rir  = canp->can->sFIFOMailBox[0].RXMIR;
    rdtr = canp->can->sFIFOMailBox[0].RXMDTR;
    crfp->data32[0] = canp->can->sFIFOMailBox[0].RXMDLR;
    crfp->data32[1] = canp->can->sFIFOMailBox[0].RXMDHR;

    /* Releases the mailbox.*/
    canp->can->RFIFO0 = CAN_RFIFO0_RFOM0;

    /* If the queue is empty re-enables the interrupt in order to generate
       events again.*/
    if ((canp->can->RFIFO0 & CAN_RFIFO0_FMP0) == 0)
      canp->can->INTENR |= CAN_INTENR_FMPIE0;
    break;
  case 2:
    /* Fetches the message.*/
    rir  = canp->can->sFIFOMailBox[1].RXMIR;
    rdtr = canp->can->sFIFOMailBox[1].RXMDTR;
    crfp->data32[0] = canp->can->sFIFOMailBox[1].RXMDLR;
    crfp->data32[1] = canp->can->sFIFOMailBox[1].RXMDHR;

    /* Releases the mailbox.*/
    canp->can->RFIFO1 = CAN_RFIFO1_RFOM1;

    /* If the queue is empty re-enables the interrupt in order to generate
       events again.*/
    if ((canp->can->RFIFO1 & CAN_RFIFO1_FMP1) == 0)
      canp->can->INTENR |= CAN_INTENR_EMPIE1;
    break;
  default:
    /* Should not happen, do nothing.*/
    return;
  }

  /* Decodes the various fields in the RX frame.*/
  crfp->RTR = (rir & CAN_RXMI0R_RTR) >> 1;
  crfp->IDE = (rir & CAN_RXMI0R_IDE) >> 2;
  if (crfp->IDE)
    crfp->EID = rir >> 3;
  else
    crfp->SID = rir >> 21;
  crfp->DLC = rdtr & CAN_RXMDT0R_DLC;
  crfp->FMI = (uint8_t)(rdtr >> 8);
  crfp->TIME = (uint16_t)(rdtr >> 16);
}

/**
 * @brief   Tries to abort an ongoing transmission.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 * @param[in] mailbox   mailbox number
 *
 * @notapi
 */
void can_lld_abort(CANDriver *canp,
                   canmbx_t mailbox) {

  canp->can->TSTATR = 128U << ((mailbox - 1U) * 8U);
}

#if CAN_USE_SLEEP_MODE || defined(__DOXYGEN__)
/**
 * @brief   Enters the sleep mode.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
void can_lld_sleep(CANDriver *canp) {

  canp->can->CTLR |= CAN_CTLR_SLEEP;
}

/**
 * @brief   Enforces leaving the sleep mode.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
void can_lld_wakeup(CANDriver *canp) {

  canp->can->CTLR &= ~CAN_CTLR_SLEEP;
}
#endif /* CAN_USE_SLEEP_MODE */

/**
 * @brief   Programs the filters.
 * @note    This is a CH32-specific API.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 * @param[in] can2sb    number of the first filter assigned to CAN2
 * @param[in] num       number of entries in the filters array, if zero then
 *                      a default filter is programmed
 * @param[in] cfp       pointer to the filters array, can be @p NULL if
 *                      (num == 0)
 *
 * @api
 */
void canSTM32SetFilters(CANDriver *canp, uint32_t can2sb,
                        uint32_t num, const CANFilter *cfp) {

#if CH32_CAN_USE_CAN2
  osalDbgCheck((can2sb <= 28) &&
               (num <= 28));
#endif

#if CH32_CAN_USE_CAN1
  osalDbgAssert(CAND1.state == CAN_STOP, "invalid state");
#endif
#if CH32_CAN_USE_CAN2
  osalDbgAssert(CAND2.state == CAN_STOP, "invalid state");
#endif
#if CH32_CAN_USE_CAN3
  osalDbgAssert(CAND3.state == CAN_STOP, "invalid state");
#endif

#if CH32_CAN_USE_CAN1
  if (canp == &CAND1) {
    can_lld_set_filters(canp, can2sb, num, cfp);
  }
#endif
#if CH32_CAN_USE_CAN3
  if (canp == &CAND3) {
    can_lld_set_filters(canp, can2sb, num, cfp);
  }
#endif
}

#endif /* HAL_USE_CAN */

/** @} */
