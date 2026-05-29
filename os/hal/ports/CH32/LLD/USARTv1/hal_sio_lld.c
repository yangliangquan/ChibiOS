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
 * @file    USARTv1/hal_sio_lld.c
 * @brief   CH32 SIO subsystem low level driver source.
 *
 * @addtogroup SIO
 * @{
 */

#include "hal.h"

#if (HAL_USE_SIO == TRUE) || defined(__DOXYGEN__)

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
#define USART_CTLR2_CFG_FORBIDDEN           (USART_CTLR2_LBDIE)
#define USART_CTLR3_CFG_FORBIDDEN           (USART_CTLR3_CTSIE            |   \
                                             USART_CTLR3_EIE)

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   USART1 SIO driver identifier.
 */
#if (CH32_SIO_USE_USART1 == TRUE) || defined(__DOXYGEN__)
SIODriver SIOD1;
#endif

/**
 * @brief   USART2 SIO driver identifier.
 */
#if (CH32_SIO_USE_USART2 == TRUE) || defined(__DOXYGEN__)
SIODriver SIOD2;
#endif

/**
 * @brief   USART3 SIO driver identifier.
 */
#if (CH32_SIO_USE_USART3 == TRUE) || defined(__DOXYGEN__)
SIODriver SIOD3;
#endif

/**
 * @brief   USART4 SIO driver identifier.
 */
#if (CH32_SIO_USE_USART4 == TRUE) || defined(__DOXYGEN__)
SIODriver SIOD4;
#endif

/**
 * @brief   USART5 SIO driver identifier.
 */
#if (CH32_SIO_USE_USART5 == TRUE) || defined(__DOXYGEN__)
SIODriver SIOD5;
#endif

/**
 * @brief   USART6 SIO driver identifier.
 */
#if (CH32_SIO_USE_USART6 == TRUE) || defined(__DOXYGEN__)
SIODriver SIOD6;
#endif

/**
 * @brief   USART7 SIO driver identifier.
 */
#if (CH32_SIO_USE_USART7 == TRUE) || defined(__DOXYGEN__)
SIODriver SIOD7;
#endif

/**
 * @brief   USART8 SIO driver identifier.
 */
#if (CH32_SIO_USE_USART8 == TRUE) || defined(__DOXYGEN__)
SIODriver SIOD8;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver default configuration.
 * @note    In this implementation it is: 38400-8-N-1.
 */
static const SIOConfig default_config = {
  .baud  = SIO_DEFAULT_BITRATE,
  .cr1   = USART_CTLR1_DATA8,
  .cr2   = USART_CTLR2_STOP1_BITS,
  .cr3   = 0U
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Enables RX-related interrupts.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 */
static void usart_enable_rx_irq(SIODriver *siop) {
  uint32_t cr1;

  cr1 = siop->usart->CTLR1;
  if ((siop->enabled & SIO_EV_RXNOTEMPY) != 0U) {
    cr1 |= USART_CTLR1_RXNEIE;
  }
  if ((siop->enabled & SIO_EV_RXIDLE) != 0U) {
    cr1 |= USART_CTLR1_IDLEIE;
  }
  siop->usart->CTLR1 = cr1;
}

/**
 * @brief   Enables RX error interrupts.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 */
static void usart_enable_rx_errors_irq(SIODriver *siop) {

  siop->usart->CTLR1 |= __sio_reloc_field(siop->enabled, SIO_EV_PARITY_ERR, SIO_EV_PARITY_ERR_POS, USART_CTLR1_PEIE_Pos);
  siop->usart->CTLR2 |= __sio_reloc_field(siop->enabled, SIO_EV_RXBREAK,    SIO_EV_RXBREAK_POS,    USART_CTLR2_LBDIE_Pos);

  /* The following 3 are grouped under EIE.*/
  if ((siop->enabled & (SIO_EV_FRAMING_ERR |
                        SIO_EV_OVERRUN_ERR |
                        SIO_EV_NOISE_ERR)) != 0U) {
    siop->usart->CTLR3 |= USART_CTLR3_EIE;
  }
}

/**
 * @brief   Enables TX-related interrupts.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 */
static void usart_enable_tx_irq(SIODriver *siop) {

  if ((siop->enabled & SIO_EV_TXNOTFULL) != 0U) {
    siop->usart->CTLR1 |= USART_CTLR1_TXEIE;
  }
}

/**
 * @brief   Enables TX end interrupt.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 */
static void usart_enable_tx_end_irq(SIODriver *siop) {

  if ((siop->enabled & SIO_EV_TXDONE) != 0U) {
    siop->usart->CTLR1 |= USART_CTLR1_TCIE;
  }
}

/**
 * @brief   Clears the specified STATR flags.
 * @details CH32 STATR uses rc_w0 (write-0-to-clear) semantics, unlike STM32
 *          which has a separate ICR register. Writing 0 to a bit clears it,
 *          writing 1 keeps it unchanged.
 *
 * @param[in] u         pointer to the USART registers block
 * @param[in] flags     mask of flags to clear
 */
static void usart_clear_statr(USART_TypeDef *u, uint16_t flags) {

  u->STATR = (uint16_t)(~flags);
}

/**
 * @brief   USART initialization.
 * @details This function must be invoked with interrupts disabled.
 *
 * @param[in] siop      pointer to a @p SIODriver object
 */
static void usart_init(SIODriver *siop) {
  USART_TypeDef *u = siop->usart;
  uint32_t brr, clock;

  /* Prescaler calculation.*/
  clock = siop->clock;

  /* Baud rate setting.
     CH32 does not support OVER8 mode (no USART_CTLR1_OVER8 bit).*/
  brr = (uint32_t)((clock + siop->config->baud / 2)/ siop->config->baud);

  osalDbgAssert(brr < 0x10000, "invalid BRR value");

  /* Setting up USART.*/
  u->CTLR1  = siop->config->cr1 & ~USART_CTLR1_CFG_FORBIDDEN;
  u->CTLR2  = siop->config->cr2 & ~USART_CTLR2_CFG_FORBIDDEN;
  u->CTLR3  = siop->config->cr3 & ~USART_CTLR3_CFG_FORBIDDEN;
  u->BRR    = (uint16_t)brr;

  /* Clearing all pending status flags (rc_w0: write 0 to clear).*/
  u->STATR = 0x0000U;

  /* Starting operations.*/
  u->CTLR1 |= USART_CTLR1_UE | USART_CTLR1_TE | USART_CTLR1_RE;
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level SIO driver initialization.
 *
 * @notapi
 */
void sio_lld_init(void) {

  /* Driver instances initialization.*/
#if CH32_SIO_USE_USART1 == TRUE
  sioObjectInit(&SIOD1);
  SIOD1.usart       = USART1;
  SIOD1.clock       = hal_lld_get_clock_point(0U);
#endif
#if CH32_SIO_USE_USART2 == TRUE
  sioObjectInit(&SIOD2);
  SIOD2.usart       = USART2;
  SIOD2.clock       = hal_lld_get_clock_point(0U);
#endif
#if CH32_SIO_USE_USART3 == TRUE
  sioObjectInit(&SIOD3);
  SIOD3.usart       = USART3;
  SIOD3.clock       = hal_lld_get_clock_point(0U);
#endif
#if CH32_SIO_USE_USART4 == TRUE
  sioObjectInit(&SIOD4);
  SIOD4.usart       = USART4;
  SIOD4.clock       = hal_lld_get_clock_point(0U);
#endif
#if CH32_SIO_USE_USART5 == TRUE
  sioObjectInit(&SIOD5);
  SIOD5.usart       = USART5;
  SIOD5.clock       = hal_lld_get_clock_point(0U);
#endif
#if CH32_SIO_USE_USART6 == TRUE
  sioObjectInit(&SIOD6);
  SIOD6.usart       = USART6;
  SIOD6.clock       = hal_lld_get_clock_point(0U);
#endif
#if CH32_SIO_USE_USART7 == TRUE
  sioObjectInit(&SIOD7);
  SIOD7.usart       = USART7;
  SIOD7.clock       = hal_lld_get_clock_point(0U);
#endif
#if CH32_SIO_USE_USART8 == TRUE
  sioObjectInit(&SIOD8);
  SIOD8.usart       = USART8;
  SIOD8.clock       = hal_lld_get_clock_point(0U);
#endif
}

/**
 * @brief   Configures and activates the SIO peripheral.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @return              The operation status.
 *
 * @notapi
 */
msg_t sio_lld_start(SIODriver *siop) {

  /* Using the default configuration if the application passed a
     NULL pointer.*/
  if (siop->config == NULL) {
    siop->config = &default_config;
  }

  if (siop->state == SIO_STOP) {
    /* Enables the peripheral.*/
    if (false) {
    }

#if CH32_SIO_USE_USART1 == TRUE
    else if (&SIOD1 == siop) {
      resetHB2(RCC_USART1RST);
      enableHB2(RCC_USART1EN);
    }
#endif
#if CH32_SIO_USE_USART2 == TRUE
    else if (&SIOD2 == siop) {
      resetHB1(RCC_USART2RST);
      enableHB1(RCC_USART2EN);
    }
#endif
#if CH32_SIO_USE_USART3 == TRUE
    else if (&SIOD3 == siop) {
      resetHB1(RCC_USART3RST);
      enableHB1(RCC_USART3EN);
    }
#endif
#if CH32_SIO_USE_USART4 == TRUE
    else if (&SIOD4 == siop) {
      resetHB1(RCC_USART4RST);
      enableHB1(RCC_USART4EN);
    }
#endif
#if CH32_SIO_USE_USART5 == TRUE
    else if (&SIOD5 == siop) {
      resetHB1(RCC_USART5RST);
      enableHB1(RCC_USART5EN);
    }
#endif
#if CH32_SIO_USE_USART6 == TRUE
    else if (&SIOD6 == siop) {
      resetHB1(RCC_USART6RST);
      enableHB1(RCC_USART6EN);
    }
#endif
#if CH32_SIO_USE_USART7 == TRUE
    else if (&SIOD7 == siop) {
      resetHB1(RCC_USART7RST);
      enableHB1(RCC_USART7EN);
    }
#endif
#if CH32_SIO_USE_USART8 == TRUE
    else if (&SIOD8 == siop) {
      resetHB1(RCC_USART8RST);
      enableHB1(RCC_USART8EN);
    }
#endif

    else {
      osalDbgAssert(false, "invalid SIO instance");
      return HAL_RET_IS_INVALID;
    }
  }

  /* Configures the peripheral.*/
  usart_init(siop);

  return HAL_RET_SUCCESS;
}

/**
 * @brief   Deactivates the SIO peripheral.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 *
 * @notapi
 */
void sio_lld_stop(SIODriver *siop) {

  if (siop->state == SIO_READY) {
    /* Resets the peripheral.*/

    /* Disables the peripheral.*/
    if (false) {
    }
#if CH32_SIO_USE_USART1 == TRUE
    else if (&SIOD1 == siop) {
      resetHB2(RCC_USART1RST);
      disableHB2(RCC_USART1EN);
    }
#endif
#if CH32_SIO_USE_USART2 == TRUE
    else if (&SIOD2 == siop) {
      resetHB1(RCC_USART2RST);
      disableHB1(RCC_USART2EN);
    }
#endif
#if CH32_SIO_USE_USART3 == TRUE
    else if (&SIOD3 == siop) {
      resetHB1(RCC_USART3RST);
      disableHB1(RCC_USART3EN);
    }
#endif
#if CH32_SIO_USE_USART4 == TRUE
    else if (&SIOD4 == siop) {
      resetHB1(RCC_USART4RST);
      disableHB1(RCC_USART4EN);
    }
#endif
#if CH32_SIO_USE_USART5 == TRUE
    else if (&SIOD5 == siop) {
      resetHB1(RCC_USART5RST);
      disableHB1(RCC_USART5EN);
    }
#endif
#if CH32_SIO_USE_USART6 == TRUE
    else if (&SIOD6 == siop) {
      resetHB1(RCC_USART6RST);
      disableHB1(RCC_USART6EN);
    }
#endif
#if CH32_SIO_USE_USART7 == TRUE
    else if (&SIOD7 == siop) {
      resetHB1(RCC_USART7RST);
      disableHB1(RCC_USART7EN);
    }
#endif
#if CH32_SIO_USE_USART8 == TRUE
    else if (&SIOD8 == siop) {
      resetHB1(RCC_USART8RST);
      disableHB1(RCC_USART8EN);
    }
#endif
    else {
      osalDbgAssert(false, "invalid SIO instance");
    }
  }
}

/**
 * @brief   Enable flags change notification.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 */
void sio_lld_update_enable_flags(SIODriver *siop) {
  uint32_t cr1, cr2, cr3;

  cr1 = siop->usart->CTLR1 & ~(USART_CTLR1_TXEIE  | USART_CTLR1_RXNEIE |
                               USART_CTLR1_IDLEIE | USART_CTLR1_TCIE   |
                               USART_CTLR1_PEIE);
  cr2 = siop->usart->CTLR2 & ~(USART_CTLR2_LBDIE);
  cr3 = siop->usart->CTLR3 & ~(USART_CTLR3_EIE);

  cr1 |= __sio_reloc_field(siop->enabled, SIO_EV_RXNOTEMPY,  SIO_EV_RXNOTEMPY_POS,  USART_CTLR1_RXNEIE_Pos) |
         __sio_reloc_field(siop->enabled, SIO_EV_TXNOTFULL,  SIO_EV_TXNOTFULL_POS,  USART_CTLR1_TXEIE_Pos)  |
         __sio_reloc_field(siop->enabled, SIO_EV_RXIDLE,     SIO_EV_RXIDLE_POS,     USART_CTLR1_IDLEIE_Pos) |
         __sio_reloc_field(siop->enabled, SIO_EV_TXDONE,     SIO_EV_TXDONE_POS,     USART_CTLR1_TCIE_Pos)   |
         __sio_reloc_field(siop->enabled, SIO_EV_PARITY_ERR, SIO_EV_PARITY_ERR_POS, USART_CTLR1_PEIE_Pos);
  cr2 |= __sio_reloc_field(siop->enabled, SIO_EV_RXBREAK,    SIO_EV_RXBREAK_POS,    USART_CTLR2_LBDIE_Pos);

  /* The following 3 are grouped under EIE.*/
  if ((siop->enabled & (SIO_EV_FRAMING_ERR |
                        SIO_EV_OVERRUN_ERR |
                        SIO_EV_NOISE_ERR)) != 0U) {
    cr3 |= USART_CTLR3_EIE;
  }

  /* Setting up the operation.*/
  siop->usart->CTLR1 = cr1;
  siop->usart->CTLR2 = cr2;
  siop->usart->CTLR3 = cr3;
}

/**
 * @brief   Get and clears SIO error event flags.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @return              The pending error event flags.
 *
 * @notapi
 */
sioevents_t sio_lld_get_and_clear_errors(SIODriver *siop) {
  uint16_t statr;
  sioevents_t errors;

  /* Getting all error STATR flags (and only those).*/
  statr = siop->usart->STATR & (uint16_t)SIO_LLD_ISR_RX_ERRORS;

  /* Clearing captured events (rc_w0: write 0 to clear).*/
  usart_clear_statr(siop->usart, statr);

  /* Status flags cleared, now the error-related interrupts can be
     enabled again.*/
  usart_enable_rx_errors_irq(siop);

  /* Translating the status flags in SIO events.*/
  errors = __sio_reloc_field(statr, USART_STATR_ONFP_Msk,  USART_STATR_ONFP_Pos,  SIO_EV_ALL_ERRORS_POS) |
           __sio_reloc_field(statr, USART_STATR_LBD_Msk,   USART_STATR_LBD_Pos,   SIO_EV_RXBREAK_POS);

  return errors;
}

/**
 * @brief   Get and clears SIO event flags.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @return              The pending event flags.
 *
 * @notapi
 */
sioevents_t sio_lld_get_and_clear_events(SIODriver *siop) {
  uint16_t statr;
  sioevents_t events;

  /* Getting all STATR flags.*/
  statr = siop->usart->STATR & (uint16_t)(SIO_LLD_ISR_RX_ERRORS |
                                          USART_STATR_RXNE        |
                                          USART_STATR_IDLE        |
                                          USART_STATR_TXE         |
                                          USART_STATR_TC);

  /* Clearing captured events (rc_w0: write 0 to clear).*/
  usart_clear_statr(siop->usart, statr);

  /* Status flags cleared, now the RX-related interrupts can be
     enabled again.*/
  usart_enable_rx_irq(siop);
  usart_enable_rx_errors_irq(siop);

  /* Translating the status flags in SIO events.*/
  events = __sio_reloc_field(statr, USART_STATR_RXNE_Msk,   USART_STATR_RXNE_Pos,   SIO_EV_RXNOTEMPY_POS)  |
           __sio_reloc_field(statr, USART_STATR_TXE_Msk,    USART_STATR_TXE_Pos,    SIO_EV_TXNOTFULL_POS)  |
           __sio_reloc_field(statr, USART_STATR_TC_Msk,     USART_STATR_TC_Pos,     SIO_EV_TXDONE_POS)     |
           __sio_reloc_field(statr, USART_STATR_IONFP_Msk,  USART_STATR_IONFP_Pos,  SIO_EV_ALL_ERRORS_POS) |
           __sio_reloc_field(statr, USART_STATR_LBD_Msk,    USART_STATR_LBD_Pos,    SIO_EV_RXBREAK_POS);

  return events;
}

/**
 * @brief   Returns the pending SIO event flags.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @return              The pending event flags.
 *
 * @notapi
 */
sioevents_t sio_lld_get_events(SIODriver *siop) {
  uint16_t statr;
  sioevents_t events;

  /* Getting all STATR flags.*/
  statr = siop->usart->STATR & (uint16_t)(SIO_LLD_ISR_RX_ERRORS |
                                          USART_STATR_RXNE        |
                                          USART_STATR_IDLE        |
                                          USART_STATR_TXE         |
                                          USART_STATR_TC);

  /* Translating the status flags in SIO events.*/
  events = __sio_reloc_field(statr, USART_STATR_RXNE_Msk,   USART_STATR_RXNE_Pos,   SIO_EV_RXNOTEMPY_POS)  |
           __sio_reloc_field(statr, USART_STATR_TXE_Msk,    USART_STATR_TXE_Pos,    SIO_EV_TXNOTFULL_POS)  |
           __sio_reloc_field(statr, USART_STATR_TC_Msk,     USART_STATR_TC_Pos,     SIO_EV_TXDONE_POS)     |
           __sio_reloc_field(statr, USART_STATR_IONFP_Msk,  USART_STATR_IONFP_Pos,  SIO_EV_ALL_ERRORS_POS) |
           __sio_reloc_field(statr, USART_STATR_LBD_Msk,    USART_STATR_LBD_Pos,    SIO_EV_RXBREAK_POS);

  return events;
}

/**
 * @brief   Reads data from the RX FIFO.
 * @details The function is not blocking, it reads frames until there
 *          is no more data available without waiting.
 *
 * @param[in] siop          pointer to an @p SIODriver structure
 * @param[in] buffer        pointer to the buffer for read frames
 * @param[in] n             maximum number of frames to be read
 * @return                  The number of frames copied from the buffer.
 * @retval 0                if the RX FIFO is empty.
 */
size_t sio_lld_read(SIODriver *siop, uint8_t *buffer, size_t n) {
  size_t rd;

  rd = 0U;
  while (true) {

    /* If the RX FIFO has been emptied then the RX FIFO and IDLE interrupts
       are enabled again.*/
    if (sio_lld_is_rx_empty(siop)) {
      usart_enable_rx_irq(siop);
      break;
    }

    /* Buffer filled condition.*/
    if (rd >= n) {
      break;
    }

    *buffer++ = (uint8_t)siop->usart->DATAR;
    rd++;
  }

  return rd;
}

/**
 * @brief   Writes data into the TX FIFO.
 * @details The function is not blocking, it writes frames until there
 *          is space available without waiting.
 *
 * @param[in] siop          pointer to an @p SIODriver structure
 * @param[in] buffer        pointer to the buffer for transmit frames
 * @param[in] n             maximum number of frames to be written
 * @return                  The number of frames copied from the buffer.
 * @retval 0                if the TX FIFO is full.
 */
size_t sio_lld_write(SIODriver *siop, const uint8_t *buffer, size_t n) {
  size_t wr;

  wr = 0U;
  while (true) {

    /* If the TX FIFO has been filled then the interrupt is enabled again.*/
    if (sio_lld_is_tx_full(siop)) {
      usart_enable_tx_irq(siop);
      break;
    }

    /* Buffer emptied condition.*/
    if (wr >= n) {
      break;
    }

    siop->usart->DATAR = (uint16_t)*buffer++;
    wr++;
  }

  /* The transmit complete interrupt is always re-enabled on write.*/
  usart_enable_tx_end_irq(siop);

  return wr;
}

/**
 * @brief   Returns one frame from the RX FIFO.
 * @note    If the FIFO is empty then the returned value is unpredictable.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @return              The frame from RX FIFO.
 *
 * @notapi
 */
msg_t sio_lld_get(SIODriver *siop) {
  msg_t msg;

  msg = (msg_t)siop->usart->DATAR;

  /* If the RX FIFO has been emptied then the interrupt is enabled again.*/
  if (sio_lld_is_rx_empty(siop)) {
    usart_enable_rx_irq(siop);
  }

  return msg;
}

/**
 * @brief   Pushes one frame into the TX FIFO.
 * @note    If the FIFO is full then the behavior is unpredictable.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @param[in] data      frame to be written
 *
 * @notapi
 */
void sio_lld_put(SIODriver *siop, uint_fast16_t data) {

  siop->usart->DATAR = (uint16_t)data;

  /* If the TX FIFO has been filled then the interrupt is enabled again.*/
  if (sio_lld_is_tx_full(siop)) {
    usart_enable_tx_irq(siop);
  }

  /* The transmit complete interrupt is always re-enabled on write.*/
  usart_enable_tx_end_irq(siop);
}

/**
 * @brief   Control operation on a serial port.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @param[in] operation control operation code
 * @param[in,out] arg   operation argument
 *
 * @return              The control operation status.
 * @retval MSG_OK       in case of success.
 * @retval MSG_TIMEOUT  in case of operation timeout.
 * @retval MSG_RESET    in case of operation reset.
 *
 * @notapi
 */
msg_t sio_lld_control(SIODriver *siop, unsigned int operation, void *arg) {

  (void)siop;
  (void)operation;
  (void)arg;

  return MSG_OK;
}

/**
 * @brief   Serves an USART interrupt.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 *
 * @notapi
 */
void sio_lld_serve_interrupt(SIODriver *siop) {
  USART_TypeDef *u = siop->usart;
  uint32_t cr1, cr2, cr3, statr, isrmask;

  osalDbgAssert(siop->state == SIO_READY, "invalid state");

  /* Read on control registers.*/
  cr1 = u->CTLR1;
  cr2 = u->CTLR2;
  cr3 = u->CTLR3;

  /* Calculating the mask of status bits that should be processed according
     to the state of the various CTLRx registers.*/
  isrmask = __sio_reloc_field(cr1, USART_CTLR1_TXEIE,  USART_CTLR1_TXEIE_Pos,  USART_STATR_TXE_Pos)  |
            __sio_reloc_field(cr1, USART_CTLR1_RXNEIE, USART_CTLR1_RXNEIE_Pos, USART_STATR_RXNE_Pos) |
            /* NOTE: ORE interrupt also enabled by USART_CTLR1_RXNEIE, not just USART_CTLR3_EIE.*/
            __sio_reloc_field(cr1, USART_CTLR1_RXNEIE, USART_CTLR1_RXNEIE_Pos, USART_STATR_ORE_Pos)  |
            __sio_reloc_field(cr1, USART_CTLR1_IDLEIE, USART_CTLR1_IDLEIE_Pos, USART_STATR_IDLE_Pos) |
            __sio_reloc_field(cr1, USART_CTLR1_TCIE,   USART_CTLR1_TCIE_Pos,   USART_STATR_TC_Pos)   |
            __sio_reloc_field(cr1, USART_CTLR1_PEIE,   USART_CTLR1_PEIE_Pos,   USART_STATR_PE_Pos)   |
            __sio_reloc_field(cr2, USART_CTLR2_LBDIE,  USART_CTLR2_LBDIE_Pos,  USART_STATR_LBD_Pos);
  if ((cr3 & USART_CTLR3_EIE) != 0U) {
    isrmask |= USART_STATR_NE | USART_STATR_FE | USART_STATR_ORE;
  }

  /* Status flags to be processed.*/
  statr = u->STATR & isrmask;
  if (statr != 0U) {

    /* Error flags handled as a group.*/
    if ((statr & SIO_LLD_ISR_RX_ERRORS) != 0U) {
#if SIO_USE_SYNCHRONIZATION
      /* The idle flag is forcibly cleared when an RX error event is
         detected (rc_w0: write 0 to clear).*/
      u->STATR = (uint16_t)(~USART_STATR_IDLE);
#endif

      /* All RX-related interrupt sources disabled.*/
      cr1 &= ~(USART_CTLR1_PEIE | USART_CTLR1_RXNEIE | USART_CTLR1_IDLEIE);
      cr2 &= ~(USART_CTLR2_LBDIE);
      cr3 &= ~(USART_CTLR3_EIE);

      /* Waiting thread woken, if any.*/
      __sio_wakeup_errors(siop);
    }
    /* If there are no errors then we check for the other RX-related
       status flags.*/
    else {
      /* Idle RX flag. Note: At start the USART will produce an IDLE interrupt.*/
      if ((statr & USART_STATR_IDLE) != 0U) {

        /* Interrupt source disabled.*/
        cr1 &= ~USART_CTLR1_IDLEIE;

        /* Waiting thread woken, if any.*/
        __sio_wakeup_rxidle(siop);
      }

      /* RX FIFO is non-empty.*/
      if ((statr & USART_STATR_RXNE) != 0U) {

#if SIO_USE_SYNCHRONIZATION
        /* The idle flag is forcibly cleared when an RX data event is
           detected (rc_w0: write 0 to clear).*/
        u->STATR = (uint16_t)(~USART_STATR_IDLE);
#endif

        /* Interrupt source disabled.*/
        cr1 &= ~USART_CTLR1_RXNEIE;

        /* Waiting thread woken, if any.*/
        __sio_wakeup_rx(siop);
      }
    }

    /* TX FIFO is non-full.*/
    if ((statr & USART_STATR_TXE) != 0U) {

      /* Interrupt source disabled.*/
      cr1 &= ~USART_CTLR1_TXEIE;

      /* Waiting thread woken, if any.*/
      __sio_wakeup_tx(siop);
    }

    /* Physical transmission end.*/
    if ((statr & USART_STATR_TC) != 0U) {

      /* Interrupt source disabled.*/
      cr1 &= ~USART_CTLR1_TCIE;

      /* Waiting thread woken, if any.*/
      __sio_wakeup_txend(siop);
    }

    /* Updating control registers, some sources could have been disabled.*/
    u->CTLR1 = cr1;
    u->CTLR2 = cr2;
    u->CTLR3 = cr3;

    /* The callback is invoked.*/
    __sio_callback(siop);
  }
  else {
    /* Shared CH32 USART vectors can dispatch multiple instances, ignore the
       call if this peripheral has no pending enabled source.*/
  }
}

#endif /* HAL_USE_SIO == TRUE */

/** @} */
