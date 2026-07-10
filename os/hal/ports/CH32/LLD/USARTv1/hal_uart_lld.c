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
 * @file    USARTv1/hal_uart_lld.c
 * @brief   CH32 UART subsystem low level driver source.
 *
 * @addtogroup UART
 * @{
 */

#include "hal.h"

#if (HAL_USE_UART == TRUE) || defined(__DOXYGEN__)

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
#define USART_CTLR3_CFG_FORBIDDEN           (USART_CTLR3_DMAT             |   \
                                             USART_CTLR3_DMAR             |   \
                                             USART_CTLR3_EIE)

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief USART1 UART driver identifier.*/
#if (CH32_UART_USE_USART1 == TRUE) || defined(__DOXYGEN__)
UARTDriver UARTD1;
#endif

/** @brief USART2 UART driver identifier.*/
#if (CH32_UART_USE_USART2 == TRUE) || defined(__DOXYGEN__)
UARTDriver UARTD2;
#endif

/** @brief USART3 UART driver identifier.*/
#if (CH32_UART_USE_USART3 == TRUE) || defined(__DOXYGEN__)
UARTDriver UARTD3;
#endif

/** @brief USART4 UART driver identifier.*/
#if (CH32_UART_USE_USART4 == TRUE) || defined(__DOXYGEN__)
UARTDriver UARTD4;
#endif

/** @brief USART5 UART driver identifier.*/
#if (CH32_UART_USE_USART5 == TRUE) || defined(__DOXYGEN__)
UARTDriver UARTD5;
#endif

/** @brief USART6 UART driver identifier.*/
#if (CH32_UART_USE_USART6 == TRUE) || defined(__DOXYGEN__)
UARTDriver UARTD6;
#endif

/** @brief USART7 UART driver identifier.*/
#if (CH32_UART_USE_USART7 == TRUE) || defined(__DOXYGEN__)
UARTDriver UARTD7;
#endif

/** @brief USART8 UART driver identifier.*/
#if (CH32_UART_USE_USART8 == TRUE) || defined(__DOXYGEN__)
UARTDriver UARTD8;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Status bits translation.
 *
 * @param[in] statr     USART STATR register value
 *
 * @return  The error flags.
 */
static uartflags_t translate_errors(uint16_t statr) {
  uartflags_t sts = 0;

  if (statr & USART_STATR_ORE)
    sts |= UART_OVERRUN_ERROR;
  if (statr & USART_STATR_PE)
    sts |= UART_PARITY_ERROR;
  if (statr & USART_STATR_FE)
    sts |= UART_FRAMING_ERROR;
  if (statr & USART_STATR_NE)
    sts |= UART_NOISE_ERROR;
  if (statr & USART_STATR_LBD)
    sts |= UART_BREAK_DETECTED;
  return sts;
}

/**
 * @brief   Clears the specified STATR flags.
 * @details CH32 STATR uses rc_w0 (write-0-to-clear) semantics.
 *
 * @param[in] u         pointer to the USART registers block
 * @param[in] flags     mask of flags to clear
 */
static void usart_clear_statr(USART_TypeDef *u, uint16_t flags) {

  u->STATR = (uint16_t)(~flags);
}

/**
 * @brief   Puts the receiver in the UART_RX_IDLE state.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 */
static void uart_enter_rx_idle_loop(UARTDriver *uartp) {
  uint32_t mode;

  /* RX DMA channel preparation, if the char callback is defined then the
     TCIE interrupt is enabled too.*/
  if (uartp->config->rxchar_cb == NULL)
    mode = DMA_DIR_PeripheralSRC | DMA_Mode_Circular;
  else
    mode = DMA_DIR_PeripheralSRC | DMA_Mode_Circular | DMA_CFGR1_TCIE;

  dmaStreamSetMemory0(uartp->dmarx, &uartp->rxbuf);
  dmaStreamSetTransactionSize(uartp->dmarx, 1);
  dmaStreamSetMode(uartp->dmarx, uartp->dmarxmode | mode);
  dmaStreamEnable(uartp->dmarx);
}

/**
 * @brief   USART de-initialization.
 * @details This function must be invoked with interrupts disabled.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 */
static void usart_stop(UARTDriver *uartp) {

  /* Stops RX and TX DMA channels.*/
  dmaStreamDisable(uartp->dmarx);
  dmaStreamDisable(uartp->dmatx);

  /* Stops USART operations.*/
  uartp->usart->CTLR1 = 0;
  uartp->usart->CTLR2 = 0;
  uartp->usart->CTLR3 = 0;
}

/**
 * @brief   USART initialization.
 * @details This function must be invoked with interrupts disabled.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 */
static void usart_start(UARTDriver *uartp) {
  uint32_t clock;
  uint32_t brr;
  USART_TypeDef *u = uartp->usart;

  /* Defensive programming, starting from a clean state.*/
  usart_stop(uartp);

  /* Baud rate setting.
     CH32 does not support OVER8 mode (no USART_CTLR1_OVER8 bit).*/
  clock = uartp->clock;
  brr = (uint32_t)((clock + uartp->config->baud / 2)/ uartp->config->baud);

  osalDbgAssert(brr < 0x10000, "invalid BRR value");

  /* Setting up USART.*/
  u->CTLR1  = uartp->config->cr1 & ~USART_CTLR1_CFG_FORBIDDEN;
  u->CTLR2  = uartp->config->cr2 & ~USART_CTLR2_CFG_FORBIDDEN;
  u->CTLR3  = uartp->config->cr3 & ~USART_CTLR3_CFG_FORBIDDEN;
  u->BRR    = (uint16_t)brr;

  /* Note that some bits are enforced because required for correct driver
     operations.*/
  u->CTLR2 |= USART_CTLR2_LBDIE;
  u->CTLR3 |= USART_CTLR3_DMAT | USART_CTLR3_DMAR | USART_CTLR3_EIE;

  /* Clearing all pending status flags (rc_w0: write 0 to clear).*/
  u->STATR = 0x0000U;

  /* Mustn't ever set TCIE here - if done, it causes an immediate
     interrupt.*/
  u->CTLR1 |= USART_CTLR1_UE | USART_CTLR1_PEIE | USART_CTLR1_TE | USART_CTLR1_RE;

  /* Starting the receiver idle loop.*/
  uart_enter_rx_idle_loop(uartp);
}

/**
 * @brief   RX DMA common service routine.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 * @param[in] flags     pre-shifted content of the ISR register
 */
static void uart_lld_serve_rx_end_irq(UARTDriver *uartp, uint32_t flags) {

  /* DMA errors handling.*/
#if defined(CH32_UART_DMA_ERROR_HOOK)
  if ((flags & (DMA1_FLAG_TE1)) != 0) {
    CH32_UART_DMA_ERROR_HOOK(uartp);
  }
#else
  (void)flags;
#endif

  if(flags & (DMA1_FLAG_HT1)) {
    /* Half of the buffer has been transferred, a callback is generated, if
       enabled, for each half of the buffer and then the driver stays in the
       same state.*/
  }
  if(flags & (DMA1_FLAG_TC1)) {
    /* A completed transfer, a callback is generated, if enabled, and then
       the driver goes in idle state.*/
    if (uartp->rxstate == UART_RX_IDLE) {
      /* Receiver in idle state, a callback is generated, if enabled, for each
        received character and then the driver stays in the same state.*/
      _uart_rx_idle_code(uartp);
    }
    else {
      /* Receiver in active state, a callback is generated, if enabled, after
        a completed transfer.*/
      dmaStreamDisable(uartp->dmarx);
      _uart_rx_complete_isr_code(uartp);
    }
  }
}

/**
 * @brief   TX DMA common service routine.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 * @param[in] flags     pre-shifted content of the ISR register
 */
static void uart_lld_serve_tx_end_irq(UARTDriver *uartp, uint32_t flags) {

  /* DMA errors handling.*/
#if defined(CH32_UART_DMA_ERROR_HOOK)
  if ((flags & (DMA1_FLAG_TE1)) != 0) {
    CH32_UART_DMA_ERROR_HOOK(uartp);
  }
#else
  (void)flags;
#endif
  if(flags & (DMA1_FLAG_HT1)) {
    /* Half of the buffer has been transferred, a callback is generated, if
       enabled, for each half of the buffer and then the driver stays in the
       same state.*/
  }
  if(flags & (DMA1_FLAG_TC1)) {
    /* A completed transfer, a callback is generated, if enabled, and then
       the driver goes in idle state.*/
    dmaStreamDisable(uartp->dmatx);

    /* A callback is generated, if enabled, after a completed transfer.*/
    _uart_tx1_isr_code(uartp);
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if CH32_UART_USE_USART1 || defined(__DOXYGEN__)
#if !defined(CH32_USART1_SUPPRESS_ISR)
/**
 * @brief   USART1 IRQ handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(USART1_IRQHandler) {


  uart_lld_serve_interrupt(&UARTD1);

}
#endif
#endif /* CH32_UART_USE_USART1 */

#if CH32_UART_USE_USART2 || defined(__DOXYGEN__)
#if !defined(CH32_USART2_SUPPRESS_ISR)
/**
 * @brief   USART2 IRQ handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(USART2_IRQHandler) {


  uart_lld_serve_interrupt(&UARTD2);

}
#endif
#endif /* CH32_UART_USE_USART2 */

#if CH32_UART_USE_USART3 || defined(__DOXYGEN__)
#if !defined(CH32_USART3_SUPPRESS_ISR)
/**
 * @brief   USART3 IRQ handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(USART3_IRQHandler) {


  uart_lld_serve_interrupt(&UARTD3);

}
#endif
#endif /* CH32_UART_USE_USART3 */

#if CH32_UART_USE_USART4 || defined(__DOXYGEN__)
#if !defined(CH32_USART4_SUPPRESS_ISR)
/**
 * @brief   USART4 IRQ handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(USART4_IRQHandler) {


  uart_lld_serve_interrupt(&UARTD4);

}
#endif
#endif /* CH32_UART_USE_USART4 */

#if CH32_UART_USE_USART5 || defined(__DOXYGEN__)
#if !defined(CH32_USART5_SUPPRESS_ISR)
/**
 * @brief   USART5 IRQ handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(USART5_IRQHandler) {


  uart_lld_serve_interrupt(&UARTD5);

}
#endif
#endif /* CH32_UART_USE_USART5 */

#if CH32_UART_USE_USART6 || defined(__DOXYGEN__)
#if !defined(CH32_USART6_SUPPRESS_ISR)
/**
 * @brief   USART6 IRQ handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(USART6_IRQHandler) {


  uart_lld_serve_interrupt(&UARTD6);

}
#endif
#endif /* CH32_UART_USE_USART6 */

#if CH32_UART_USE_USART7 || defined(__DOXYGEN__)
#if !defined(CH32_USART7_SUPPRESS_ISR)
/**
 * @brief   USART7 IRQ handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(USART7_IRQHandler) {

  uart_lld_serve_interrupt(&UARTD7);

}
#endif
#endif /* CH32_UART_USE_USART7 */

#if CH32_UART_USE_USART8 || defined(__DOXYGEN__)
#if !defined(CH32_USART8_SUPPRESS_ISR)
/**
 * @brief   USART8 IRQ handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(USART8_IRQHandler) {


  uart_lld_serve_interrupt(&UARTD8);

}
#endif
#endif /* CH32_UART_USE_USART8 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level UART driver initialization.
 *
 * @notapi
 */
void uart_lld_init(void) {

#if CH32_UART_USE_USART1
  uartObjectInit(&UARTD1);
  UARTD1.usart     = USART1;
  UARTD1.clock     = hal_lld_get_clock_point(0U);
  UARTD1.dmarxmode = DMA_CFGR1_TEIE | DMA_CFGR1_HTIE;
  UARTD1.dmatxmode = DMA_CFGR1_TEIE | DMA_CFGR1_HTIE;
  UARTD1.dmarx     = NULL;
  UARTD1.dmatx     = NULL;
#endif

#if CH32_UART_USE_USART2
  uartObjectInit(&UARTD2);
  UARTD2.usart     = USART2;
  UARTD2.clock     = hal_lld_get_clock_point(0U);
  UARTD2.dmarxmode = DMA_CFGR1_TEIE | DMA_CFGR1_HTIE;
  UARTD2.dmatxmode = DMA_CFGR1_TEIE | DMA_CFGR1_HTIE;
  UARTD2.dmarx     = NULL;
  UARTD2.dmatx     = NULL;
#endif

#if CH32_UART_USE_USART3
  uartObjectInit(&UARTD3);
  UARTD3.usart     = USART3;
  UARTD3.clock     = hal_lld_get_clock_point(0U);
  UARTD3.dmarxmode = DMA_CFGR1_TEIE;
  UARTD3.dmatxmode = DMA_CFGR1_TEIE;
  UARTD3.dmarx     = NULL;
  UARTD3.dmatx     = NULL;
#endif

#if CH32_UART_USE_USART4
  uartObjectInit(&UARTD4);
  UARTD4.usart     = USART4;
  UARTD4.clock     = hal_lld_get_clock_point(0U);
  UARTD4.dmarxmode = DMA_CFGR1_TEIE | DMA_CFGR1_HTIE;
  UARTD4.dmatxmode = DMA_CFGR1_TEIE | DMA_CFGR1_HTIE;
  UARTD4.dmarx     = NULL;
  UARTD4.dmatx     = NULL;
#endif

#if CH32_UART_USE_USART5
  uartObjectInit(&UARTD5);
  UARTD5.usart     = USART5;
  UARTD5.clock     = hal_lld_get_clock_point(0U);
  UARTD5.dmarxmode = DMA_CFGR1_TEIE | DMA_CFGR1_HTIE;
  UARTD5.dmatxmode = DMA_CFGR1_TEIE | DMA_CFGR1_HTIE;
  UARTD5.dmarx     = NULL;
  UARTD5.dmatx     = NULL;
#endif

#if CH32_UART_USE_USART6
  uartObjectInit(&UARTD6);
  UARTD6.usart     = USART6;
  UARTD6.clock     = hal_lld_get_clock_point(0U);
  UARTD6.dmarxmode = DMA_CFGR1_TEIE | DMA_CFGR1_HTIE;
  UARTD6.dmatxmode = DMA_CFGR1_TEIE | DMA_CFGR1_HTIE;
  UARTD6.dmarx     = NULL;
  UARTD6.dmatx     = NULL;
#endif

#if CH32_UART_USE_USART7
  uartObjectInit(&UARTD7);
  UARTD7.usart     = USART7;
  UARTD7.clock     = hal_lld_get_clock_point(0U);
  UARTD7.dmarxmode = DMA_CFGR1_TEIE | DMA_CFGR1_HTIE;
  UARTD7.dmatxmode = DMA_CFGR1_TEIE | DMA_CFGR1_HTIE;
  UARTD7.dmarx     = NULL;
  UARTD7.dmatx     = NULL;
#endif

#if CH32_UART_USE_USART8
  uartObjectInit(&UARTD8);
  UARTD8.usart     = USART8;
  UARTD8.clock     = hal_lld_get_clock_point(0U);
  UARTD8.dmarxmode = DMA_CFGR1_TEIE | DMA_CFGR1_HTIE;
  UARTD8.dmatxmode = DMA_CFGR1_TEIE | DMA_CFGR1_HTIE;
  UARTD8.dmarx     = NULL;
  UARTD8.dmatx     = NULL;
#endif
}

/**
 * @brief   Configures and activates the UART peripheral.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 *
 * @notapi
 */
void uart_lld_start(UARTDriver *uartp) {

  if (uartp->state == UART_STOP) {
#if CH32_UART_USE_USART1
    if (&UARTD1 == uartp) {
      /* Allocating RX DMA channel.*/
      uartp->dmarx = dmaStreamAllocI(CH32_DMA_STREAM_ID_ANY,
                                     CH32_UART_USART1_DMA_PRIORITY,
                                     (ch32_dmaisr_t)uart_lld_serve_rx_end_irq,
                                     (void *)uartp);
      osalDbgAssert(uartp->dmarx != NULL, "unable to allocate stream");

      /* Allocating TX DMA channel.*/
      uartp->dmatx = dmaStreamAllocI(CH32_DMA_STREAM_ID_ANY,
                                     CH32_UART_USART1_DMA_PRIORITY,
                                     (ch32_dmaisr_t)uart_lld_serve_tx_end_irq,
                                     (void *)uartp);
      osalDbgAssert(uartp->dmatx != NULL, "unable to allocate stream");

      /* Enabling USART clock and IRQ.*/
      resetHB2(RCC_USART1RST);
      enableHB2(RCC_USART1EN);

      /* Setting DMAMUX request source.*/
      dmaSetRequestSource(uartp->dmarx, DMA_MUX_USART1_RX);
      dmaSetRequestSource(uartp->dmatx, DMA_MUX_USART1_TX);

      /* DMA configurations.*/
      uartp->dmarxmode |= DMA_Priority_Low;
      uartp->dmatxmode |= DMA_Priority_Low;

      /* Enabling IRQ.*/
      NVIC_SetPriority(USART1_IRQn, CH32_UART_USART1_IRQ_PRIORITY);
      NVIC_EnableIRQ(USART1_IRQn);
    }
#endif

#if CH32_UART_USE_USART2
    if (&UARTD2 == uartp) {
      /* Allocating RX DMA channel.*/
      uartp->dmarx = dmaStreamAllocI(CH32_DMA_STREAM_ID_ANY,
                                     CH32_UART_USART2_DMA_PRIORITY,
                                     (ch32_dmaisr_t)uart_lld_serve_rx_end_irq,
                                     (void *)uartp);
      osalDbgAssert(uartp->dmarx != NULL, "unable to allocate stream");

      /* Allocating TX DMA channel.*/
      uartp->dmatx = dmaStreamAllocI(CH32_DMA_STREAM_ID_ANY,
                                     CH32_UART_USART2_DMA_PRIORITY,
                                     (ch32_dmaisr_t)uart_lld_serve_tx_end_irq,
                                     (void *)uartp);
      osalDbgAssert(uartp->dmatx != NULL, "unable to allocate stream");

      /* Enabling USART clock and IRQ.*/
      resetHB1(RCC_USART2RST);
      enableHB1(RCC_USART2EN);

      /* Setting DMAMUX request source.*/
      dmaSetRequestSource(uartp->dmarx, DMA_MUX_USART2_RX);
      dmaSetRequestSource(uartp->dmatx, DMA_MUX_USART2_TX);

      /* DMA configurations.*/
      uartp->dmarxmode |= DMA_Priority_Low;
      uartp->dmatxmode |= DMA_Priority_Low;

      /* Enabling IRQ.*/
      NVIC_SetPriority(USART2_IRQn, CH32_UART_USART2_IRQ_PRIORITY);
      NVIC_EnableIRQ(USART2_IRQn);
    }
#endif

#if CH32_UART_USE_USART3
    if (&UARTD3 == uartp) {
      /* Allocating RX DMA channel.*/
      uartp->dmarx = dmaStreamAllocI(CH32_DMA_STREAM_ID_ANY,
                                     CH32_UART_USART3_DMA_PRIORITY,
                                     (ch32_dmaisr_t)uart_lld_serve_rx_end_irq,
                                     (void *)uartp);
      osalDbgAssert(uartp->dmarx != NULL, "unable to allocate stream");

      /* Allocating TX DMA channel.*/
      uartp->dmatx = dmaStreamAllocI(CH32_DMA_STREAM_ID_ANY,
                                     CH32_UART_USART3_DMA_PRIORITY,
                                     (ch32_dmaisr_t)uart_lld_serve_tx_end_irq,
                                     (void *)uartp);
      osalDbgAssert(uartp->dmatx != NULL, "unable to allocate stream");

      /* Enabling USART clock and IRQ.*/
      resetHB1(RCC_USART3RST);
      enableHB1(RCC_USART3EN);

      /* Setting DMAMUX request source.*/
      dmaSetRequestSource(uartp->dmarx, DMA_MUX_USART3_RX);
      dmaSetRequestSource(uartp->dmatx, DMA_MUX_USART3_TX);

      /* DMA configurations.*/
      uartp->dmarxmode |= DMA_Priority_Low;
      uartp->dmatxmode |= DMA_Priority_Low;

      /* Enabling IRQ.*/
      NVIC_SetPriority(USART3_IRQn, CH32_UART_USART3_IRQ_PRIORITY);
      NVIC_EnableIRQ(USART3_IRQn);
    }
#endif

#if CH32_UART_USE_USART4
    if (&UARTD4 == uartp) {
      /* Allocating RX DMA channel.*/
      uartp->dmarx = dmaStreamAllocI(CH32_DMA_STREAM_ID_ANY,
                                     CH32_UART_USART4_DMA_PRIORITY,
                                     (ch32_dmaisr_t)uart_lld_serve_rx_end_irq,
                                     (void *)uartp);
      osalDbgAssert(uartp->dmarx != NULL, "unable to allocate stream");

      /* Allocating TX DMA channel.*/
      uartp->dmatx = dmaStreamAllocI(CH32_DMA_STREAM_ID_ANY,
                                     CH32_UART_USART4_DMA_PRIORITY,
                                     (ch32_dmaisr_t)uart_lld_serve_tx_end_irq,
                                     (void *)uartp);
      osalDbgAssert(uartp->dmatx != NULL, "unable to allocate stream");

      /* Enabling USART clock and IRQ.*/
      resetHB1(RCC_USART4RST);
      enableHB1(RCC_USART4EN);

      /* Setting DMAMUX request source.*/
      dmaSetRequestSource(uartp->dmarx, DMA_MUX_USART4_RX);
      dmaSetRequestSource(uartp->dmatx, DMA_MUX_USART4_TX);

      /* DMA configurations.*/
      uartp->dmarxmode |= DMA_Priority_Low;
      uartp->dmatxmode |= DMA_Priority_Low;

      /* Enabling IRQ.*/
      NVIC_SetPriority(USART4_IRQn, CH32_UART_USART4_IRQ_PRIORITY);
      NVIC_EnableIRQ(USART4_IRQn);
    }
#endif

#if CH32_UART_USE_USART5
    if (&UARTD5 == uartp) {
      /* Allocating RX DMA channel.*/
      uartp->dmarx = dmaStreamAllocI(CH32_DMA_STREAM_ID_ANY,
                                     CH32_UART_USART5_DMA_PRIORITY,
                                     (ch32_dmaisr_t)uart_lld_serve_rx_end_irq,
                                     (void *)uartp);
      osalDbgAssert(uartp->dmarx != NULL, "unable to allocate stream");

      /* Allocating TX DMA channel.*/
      uartp->dmatx = dmaStreamAllocI(CH32_DMA_STREAM_ID_ANY,
                                     CH32_UART_USART5_DMA_PRIORITY,
                                     (ch32_dmaisr_t)uart_lld_serve_tx_end_irq,
                                     (void *)uartp);
      osalDbgAssert(uartp->dmatx != NULL, "unable to allocate stream");

      /* Enabling USART clock and IRQ.*/
      resetHB1(RCC_USART5RST);
      enableHB1(RCC_USART5EN);

      /* Setting DMAMUX request source.*/
      dmaSetRequestSource(uartp->dmarx, DMA_MUX_USART5_RX);
      dmaSetRequestSource(uartp->dmatx, DMA_MUX_USART5_TX);

      /* DMA configurations.*/
      uartp->dmarxmode |= DMA_Priority_Low;
      uartp->dmatxmode |= DMA_Priority_Low;

      /* Enabling IRQ.*/
      NVIC_SetPriority(USART5_IRQn, CH32_UART_USART5_IRQ_PRIORITY);
      NVIC_EnableIRQ(USART5_IRQn);
    }
#endif

#if CH32_UART_USE_USART6
    if (&UARTD6 == uartp) {
      /* Allocating RX DMA channel.*/
      uartp->dmarx = dmaStreamAllocI(CH32_DMA_STREAM_ID_ANY,
                                     CH32_UART_USART6_DMA_PRIORITY,
                                     (ch32_dmaisr_t)uart_lld_serve_rx_end_irq,
                                     (void *)uartp);
      osalDbgAssert(uartp->dmarx != NULL, "unable to allocate stream");

      /* Allocating TX DMA channel.*/
      uartp->dmatx = dmaStreamAllocI(CH32_DMA_STREAM_ID_ANY,
                                     CH32_UART_USART6_DMA_PRIORITY,
                                     (ch32_dmaisr_t)uart_lld_serve_tx_end_irq,
                                     (void *)uartp);
      osalDbgAssert(uartp->dmatx != NULL, "unable to allocate stream");

      /* Enabling USART clock and IRQ.*/
      resetHB1(RCC_USART6RST);
      enableHB1(RCC_USART6EN);

      /* Setting DMAMUX request source.*/
      dmaSetRequestSource(uartp->dmarx, DMA_MUX_USART6_RX);
      dmaSetRequestSource(uartp->dmatx, DMA_MUX_USART6_TX);

      /* DMA configurations.*/
      uartp->dmarxmode |= DMA_Priority_Low;
      uartp->dmatxmode |= DMA_Priority_Low;

      /* Enabling IRQ.*/
      NVIC_SetPriority(USART6_IRQn, CH32_UART_USART6_IRQ_PRIORITY);
      NVIC_EnableIRQ(USART6_IRQn);
    }
#endif

#if CH32_UART_USE_USART7
    if (&UARTD7 == uartp) {
      /* Allocating RX DMA channel.*/
      uartp->dmarx = dmaStreamAllocI(CH32_DMA_STREAM_ID_ANY,
                                     CH32_UART_USART7_DMA_PRIORITY,
                                     (ch32_dmaisr_t)uart_lld_serve_rx_end_irq,
                                     (void *)uartp);
      osalDbgAssert(uartp->dmarx != NULL, "unable to allocate stream");

      /* Allocating TX DMA channel.*/
      uartp->dmatx = dmaStreamAllocI(CH32_DMA_STREAM_ID_ANY,
                                     CH32_UART_USART7_DMA_PRIORITY,
                                     (ch32_dmaisr_t)uart_lld_serve_tx_end_irq,
                                     (void *)uartp);
      osalDbgAssert(uartp->dmatx != NULL, "unable to allocate stream");

      /* Enabling USART clock and IRQ.*/
      resetHB1(RCC_USART7RST);
      enableHB1(RCC_USART7EN);

      /* Setting DMAMUX request source.*/
      dmaSetRequestSource(uartp->dmarx, DMA_MUX_USART7_RX);
      dmaSetRequestSource(uartp->dmatx, DMA_MUX_USART7_TX);

      /* DMA configurations.*/
      uartp->dmarxmode |= DMA_Priority_Low;
      uartp->dmatxmode |= DMA_Priority_Low;

      /* Enabling IRQ.*/
      NVIC_SetPriority(USART7_IRQn, CH32_UART_USART7_IRQ_PRIORITY);
      NVIC_EnableIRQ(USART7_IRQn);
    }
#endif

#if CH32_UART_USE_USART8
    if (&UARTD8 == uartp) {
      /* Allocating RX DMA channel.*/
      uartp->dmarx = dmaStreamAllocI(CH32_DMA_STREAM_ID_ANY,
                                     CH32_UART_USART8_DMA_PRIORITY,
                                     (ch32_dmaisr_t)uart_lld_serve_rx_end_irq,
                                     (void *)uartp);
      osalDbgAssert(uartp->dmarx != NULL, "unable to allocate stream");

      /* Allocating TX DMA channel.*/
      uartp->dmatx = dmaStreamAllocI(CH32_DMA_STREAM_ID_ANY,
                                     CH32_UART_USART8_DMA_PRIORITY,
                                     (ch32_dmaisr_t)uart_lld_serve_tx_end_irq,
                                     (void *)uartp);
      osalDbgAssert(uartp->dmatx != NULL, "unable to allocate stream");

      /* Enabling USART clock and IRQ.*/
      resetHB1(RCC_USART8RST);
      enableHB1(RCC_USART8EN);

      /* Setting DMAMUX request source.*/
      dmaSetRequestSource(uartp->dmarx, DMA_MUX_USART8_RX);
      dmaSetRequestSource(uartp->dmatx, DMA_MUX_USART8_TX);

      /* DMA configurations.*/
      uartp->dmarxmode |= DMA_Priority_Low;
      uartp->dmatxmode |= DMA_Priority_Low;

      /* Enabling IRQ.*/
      NVIC_SetPriority(USART8_IRQn, CH32_UART_USART8_IRQ_PRIORITY);
      NVIC_EnableIRQ(USART8_IRQn);
    }
#endif

    /* Static DMA setup, the transfer size depends on the USART settings,
       it is 16 bits if M=1 and PCE=0 else it is 8 bits.*/
    if ((uartp->config->cr1 & (USART_CTLR1_M | USART_CTLR1_PCE)) == USART_CTLR1_M) {
      uartp->dmarxmode |= DMA_PeripheralDataSize_HalfWord | DMA_MemoryDataSize_HalfWord;
      uartp->dmatxmode |= DMA_PeripheralDataSize_HalfWord | DMA_MemoryDataSize_HalfWord;
    }

    dmaStreamSetPeripheral(uartp->dmarx, &uartp->usart->DATAR);
    dmaStreamSetPeripheral(uartp->dmatx, &uartp->usart->DATAR);
    uartp->rxbuf = 0;
  }

  uartp->rxstate = UART_RX_IDLE;
  uartp->txstate = UART_TX_IDLE;
  usart_start(uartp);
}

/**
 * @brief   Deactivates the UART peripheral.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 *
 * @notapi
 */
void uart_lld_stop(UARTDriver *uartp) {

  if (uartp->state == UART_READY) {
    usart_stop(uartp);
    dmaStreamFreeI(uartp->dmarx);
    dmaStreamFreeI(uartp->dmatx);
    uartp->dmarx = NULL;
    uartp->dmatx = NULL;

#if CH32_UART_USE_USART1
    if (&UARTD1 == uartp) {
      NVIC_DisableIRQ(USART1_IRQn);
      resetHB2(RCC_USART1RST);
      disableHB2(RCC_USART1EN);
      return;
    }
#endif

#if CH32_UART_USE_USART2
    if (&UARTD2 == uartp) {
      NVIC_DisableIRQ(USART2_IRQn);
      resetHB1(RCC_USART2RST);
      disableHB1(RCC_USART2EN);
      return;
    }
#endif

#if CH32_UART_USE_USART3
    if (&UARTD3 == uartp) {
      NVIC_DisableIRQ(USART3_IRQn);
      resetHB1(RCC_USART3RST);
      disableHB1(RCC_USART3EN);
      return;
    }
#endif

#if CH32_UART_USE_USART4
    if (&UARTD4 == uartp) {
      NVIC_DisableIRQ(USART4_IRQn);
      resetHB1(RCC_USART4RST);
      disableHB1(RCC_USART4EN);
      return;
    }
#endif

#if CH32_UART_USE_USART5
    if (&UARTD5 == uartp) {
      NVIC_DisableIRQ(USART5_IRQn);
      resetHB1(RCC_USART5RST);
      disableHB1(RCC_USART5EN);
      return;
    }
#endif

#if CH32_UART_USE_USART6
    if (&UARTD6 == uartp) {
      NVIC_DisableIRQ(USART6_IRQn);
      resetHB1(RCC_USART6RST);
      disableHB1(RCC_USART6EN);
      return;
    }
#endif

#if CH32_UART_USE_USART7
    if (&UARTD7 == uartp) {
      NVIC_DisableIRQ(USART7_IRQn);
      resetHB1(RCC_USART7RST);
      disableHB1(RCC_USART7EN);
      return;
    }
#endif

#if CH32_UART_USE_USART8
    if (&UARTD8 == uartp) {
      NVIC_DisableIRQ(USART8_IRQn);
      resetHB1(RCC_USART8RST);
      disableHB1(RCC_USART8EN);
      return;
    }
#endif
  }
}

/**
 * @brief   Starts a transmission on the UART peripheral.
 * @note    The buffers are organized as uint8_t arrays for data sizes below
 *          or equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 * @param[in] n         number of data frames to send
 * @param[in] txbuf     the pointer to the transmit buffer
 *
 * @notapi
 */
void uart_lld_start_send(UARTDriver *uartp, size_t n, const void *txbuf) {

  /* TX DMA channel preparation.*/
  dmaStreamSetMemory0(uartp->dmatx, txbuf);
  dmaStreamSetTransactionSize(uartp->dmatx, n);
  dmaStreamSetMode(uartp->dmatx, uartp->dmatxmode  | DMA_DIR_PeripheralDST |
                                 DMA_MemoryInc_Enable | DMA_CFGR1_TCIE);

  /* Only enable TC interrupt if there's a callback attached to it or
     if called from uartSendFullTimeout(). Also we need to clear TC flag
     which could be set before.*/
#if UART_USE_WAIT == TRUE
  if ((uartp->config->txend2_cb != NULL) || (uartp->early == false))
#else
  if (uartp->config->txend2_cb != NULL) 
#endif
  {
    uartp->usart->STATR = (uint16_t)(~USART_STATR_TC);
    uartp->usart->CTLR1 |= USART_CTLR1_TCIE;
  }

  /* Starting transfer.*/
  dmaStreamEnable(uartp->dmatx);
}

/**
 * @brief   Stops any ongoing transmission.
 * @note    Stopping a transmission also suppresses the transmission callbacks.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 *
 * @return              The number of data frames not transmitted by the
 *                      stopped transmit operation.
 *
 * @notapi
 */
size_t uart_lld_stop_send(UARTDriver *uartp) {

  dmaStreamDisable(uartp->dmatx);

  return dmaStreamGetTransactionSize(uartp->dmatx);
}

/**
 * @brief   Starts a receive operation on the UART peripheral.
 * @note    The buffers are organized as uint8_t arrays for data sizes below
 *          or equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 * @param[in] n         number of data frames to receive
 * @param[out] rxbuf    the pointer to the receive buffer
 *
 * @notapi
 */
void uart_lld_start_receive(UARTDriver *uartp, size_t n, void *rxbuf) {

  /* Stopping previous activity (idle state).*/
  dmaStreamDisable(uartp->dmarx);

  /* RX DMA channel preparation.*/
  dmaStreamSetMemory0(uartp->dmarx, rxbuf);
  dmaStreamSetTransactionSize(uartp->dmarx, n);
  dmaStreamSetMode(uartp->dmarx, uartp->dmarxmode  | DMA_DIR_PeripheralSRC |
                                 DMA_MemoryInc_Enable | DMA_CFGR1_TCIE);

  /* Starting transfer.*/
  dmaStreamEnable(uartp->dmarx);
}

/**
 * @brief   Stops any ongoing receive operation.
 * @note    Stopping a receive operation also suppresses the receive callbacks.
 *
 * @param[in] uartp      pointer to the @p UARTDriver object
 *
 * @return              The number of data frames not received by the
 *                      stopped receive operation.
 *
 * @notapi
 */
size_t uart_lld_stop_receive(UARTDriver *uartp) {
  size_t n;

  dmaStreamDisable(uartp->dmarx);
  n = dmaStreamGetTransactionSize(uartp->dmarx);
  uart_enter_rx_idle_loop(uartp);

  return n;
}

/**
 * @brief   USART common service routine.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 */
void uart_lld_serve_interrupt(UARTDriver *uartp) {
  uint16_t statr;
  USART_TypeDef *u = uartp->usart;
  uint32_t ctlr1 = u->CTLR1;

  osalDbgAssert(uartp->state == UART_READY, "invalid state");

  statr = u->STATR;

  /* Error handling.*/
  if (statr & (USART_STATR_LBD | USART_STATR_ORE | USART_STATR_NE |
            USART_STATR_FE  | USART_STATR_PE)) {

    /* Clearing pending flags (rc_w0: write 0 to clear).*/
    usart_clear_statr(u, (uint16_t)(statr & (USART_STATR_LBD | USART_STATR_ORE |
                                             USART_STATR_NE  | USART_STATR_FE  |
                                             USART_STATR_PE)));

    /* Dummy read to clear ORE.*/
    (void)u->DATAR;

    /* Invoking error callback.*/
    _uart_rx_error_isr_code(uartp, translate_errors(statr));
  }

  /* Transmission complete interrupt.*/
  if ((statr & USART_STATR_TC) && (ctlr1 & USART_CTLR1_TCIE)) {
    /* TC interrupt cleared and disabled.*/
    u->STATR = (uint16_t)(~USART_STATR_TC);
    u->CTLR1 = ctlr1 & ~USART_CTLR1_TCIE;

    /* End of transmission, a callback is generated.*/
    _uart_tx2_isr_code(uartp);
  }

  /* Timeout interrupt sources are only checked if enabled in CTLR1.*/
  if ((ctlr1 & USART_CTLR1_IDLEIE) && (statr & USART_STATR_IDLE)) {
    _uart_timeout_isr_code(uartp);
  }
}

#endif /* HAL_USE_UART == TRUE */

/** @} */
