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
 * @file    QULDSPIv1/hal_wspi_lld.c
 * @brief   CH32 WSPI subsystem low level driver source.
 *
 * @addtogroup WSPI
 * @{
 */

#include "hal.h"

#if (HAL_USE_WSPI == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/
#define  QSPI_CR_SIOXEN                              ((uint32_t)0x00002000)
/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief WSPID1 driver identifier.*/
#if (CH32_WSPI_USE_WSPI1 == TRUE) || defined(__DOXYGEN__)
WSPIDriver WSPID1;
#endif

/** @brief WSPID2 driver identifier.*/
#if (CH32_WSPI_USE_WSPI2 == TRUE) || defined(__DOXYGEN__)
WSPIDriver WSPID2;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Builds the CCR register value from a wspi_command_t descriptor.
 *
 * @param[in] cmdp      pointer to the command descriptor
 * @return              the CCR register value (without FMODE)
 */
static inline uint32_t wspi_build_ccr(const wspi_command_t *cmdp) {

  return cmdp->cmd | cmdp->cfg |
         QULDSPI_CCR_DUMMY_CYCLES(cmdp->dummy);
}

/**
 * @brief   Waits for completion of previous operation.
 */
static inline void wspi_lld_sync(WSPIDriver *wspip) {

  while ((wspip->qspi->SR & QSPI_SR_BUSY) != 0U) {
  }
}

/**
 * @brief   Returns @p true when the QSPI is idle (not busy executing a
 *          command).
 */
static inline bool wspi_lld_idle_p(WSPIDriver *wspip) {

  return (wspip->qspi->SR & QSPI_SR_IDLEF) != 0U;
}

/**
 * @brief   Shared DMA interrupt service routine.
 *
 * @param[in] wspip     pointer to the @p WSPIDriver object
 * @param[in] flags     pre-shifted content of the ISR register
 */
static void wspi_lld_serve_dma_interrupt(WSPIDriver *wspip, uint32_t flags) {

  /* DMA errors handling.*/
#if defined(CH32_WSPI_DMA_ERROR_HOOK)
  if ((flags & DMA_CFGR1_TEIE) != 0) {
    CH32_WSPI_DMA_ERROR_HOOK(wspip);
  }
#else
  (void)flags;
#endif
  (void)wspip;
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if (CH32_WSPI_USE_WSPI1 == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   QSPI1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(QSPI1_IRQHandler) {

  WSPID1.qspi->FCR = QSPI_FCR_CTEF | QSPI_FCR_CTCF |
                      QSPI_FCR_CSMF | QSPI_FCR_CTOF;

  /* Portable WSPI ISR code defined in the high level driver, note, it is
     a macro.*/
  _wspi_isr_code(&WSPID1);
}
#endif /* CH32_WSPI_USE_WSPI1 */

#if (CH32_WSPI_USE_WSPI2 == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   QSPI2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(QSPI2_IRQHandler) {

  WSPID2.qspi->FCR = QSPI_FCR_CTEF | QSPI_FCR_CTCF |
                      QSPI_FCR_CSMF | QSPI_FCR_CTOF;

  _wspi_isr_code(&WSPID2);
}
#endif /* CH32_WSPI_USE_WSPI2 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level WSPI driver initialization.
 *
 * @notapi
 */
void wspi_lld_init(void) {

#if CH32_WSPI_USE_WSPI1
  wspiObjectInit(&WSPID1);
  WSPID1.qspi     = QSPI1;
  WSPID1.dma      = NULL;
  WSPID1.dmamode  = DMA_CFGR1_TEIE;
#endif

#if CH32_WSPI_USE_WSPI2
  wspiObjectInit(&WSPID2);
  WSPID2.qspi     = QSPI2;
  WSPID2.dma      = NULL;
  WSPID2.dmamode  = DMA_CFGR1_TEIE;
#endif
}

/**
 * @brief   Configures and activates the WSPI peripheral.
 *
 * @param[in] wspip     pointer to the @p WSPIDriver object
 *
 * @notapi
 */
void wspi_lld_start(WSPIDriver *wspip) {

  /* If in stopped state then full initialization.*/
  if (wspip->state == WSPI_STOP) {
#if CH32_WSPI_USE_WSPI1
    if (&WSPID1 == wspip) {
      resetHB1(RCC_QSPI1RST);
      enableHB1(RCC_QSPI1EN);

      wspip->dma = dmaStreamAllocI(CH32_WSPI_QSPI1_DMA_STREAM,
                                   CH32_WSPI_QSPI1_DMA_IRQ_PRIORITY,
                                   (ch32_dmaisr_t)wspi_lld_serve_dma_interrupt,
                                   (void *)wspip);
      osalDbgAssert(wspip->dma != NULL, "unable to allocate stream");
      dmaSetRequestSource(wspip->dma, DMA_MUX_QSPI1_DMA);

      /* Enable QSPI1 interrupt in NVIC.*/
      NVIC_SetPriority(QSPI1_IRQn, CH32_WSPI_QSPI1_IRQ_PRIORITY);
      NVIC_EnableIRQ(QSPI1_IRQn);
    }
#endif

#if CH32_WSPI_USE_WSPI2
    if (&WSPID2 == wspip) {
      resetHB1(RCC_QSPI2RST);
      enableHB1(RCC_QSPI2EN);

      wspip->dma = dmaStreamAllocI(CH32_WSPI_QSPI2_DMA_STREAM,
                                   CH32_WSPI_QSPI2_DMA_IRQ_PRIORITY,
                                   (ch32_dmaisr_t)wspi_lld_serve_dma_interrupt,
                                   (void *)wspip);
      osalDbgAssert(wspip->dma != NULL, "unable to allocate stream");
      dmaSetRequestSource(wspip->dma, DMA_MUX_QSPI2_DMA);

      /* Enable QSPI2 interrupt in NVIC.*/
      NVIC_SetPriority(QSPI2_IRQn, CH32_WSPI_QSPI2_IRQ_PRIORITY);
      NVIC_EnableIRQ(QSPI2_IRQn);
    }
#endif

    /* Common initializations.*/
    dmaStreamSetPeripheral(wspip->dma, &wspip->qspi->DR);
  }

  /* WSPI setup and enable.*/
  wspip->qspi->DCR = QULDSPI_DCR_FSIZE(wspip->config->fsize) |
                      QULDSPI_DCR_CSHT(wspip->config->cshtime) |
                      (wspip->config->ckmode & QULDSPI_DCR_CKMODE);
  wspip->qspi->CR  = ((wspip->config->prescaler & 0xFFU) << 24U) |
                      QSPI_CR_TCIE |
                      QSPI_CR_EN |
                      QULDSPI_CR_SIOXEN;

  /* Flash select: 0=NCS0 (primary), 1=NCS1 (secondary).*/
  if (wspip->config->fselect != 0U) {
    wspip->qspi->CR |= QSPI_CR_FSEL;
  }

  /* Dual-flash mode.*/
  if (wspip->config->dfm != 0U) {
    wspip->qspi->CR |= QSPI_CR_DFM;
  }

  /* Set FIFO threshold level.*/
  wspip->qspi->CR |= (wspip->config->fifo_threshold & 0x1FU) << 8U;

  /* Clear all pending flags.*/
  wspip->qspi->FCR = QSPI_FCR_CTEF | QSPI_FCR_CTCF |
                     QSPI_FCR_CSMF | QSPI_FCR_CTOF;
}

/**
 * @brief   Deactivates the WSPI peripheral.
 *
 * @param[in] wspip     pointer to the @p WSPIDriver object
 *
 * @notapi
 */
void wspi_lld_stop(WSPIDriver *wspip) {

  /* If in ready state then disables the QSPI clock.*/
  if (wspip->state == WSPI_READY) {

    /* WSPI disable.*/
    wspip->qspi->CR = 0U;

    /* Disable QSPI interrupt in NVIC.*/
#if CH32_WSPI_USE_WSPI1
    if (&WSPID1 == wspip) {
      NVIC_DisableIRQ(QSPI1_IRQn);
    }
#endif
#if CH32_WSPI_USE_WSPI2
    if (&WSPID2 == wspip) {
      NVIC_DisableIRQ(QSPI2_IRQn);
    }
#endif

    /* Releasing the DMA.*/
    dmaStreamFreeI(wspip->dma);
    wspip->dma = NULL;

    /* Stopping involved clocks.*/
#if CH32_WSPI_USE_WSPI1
    if (&WSPID1 == wspip) {
      disableHB1(RCC_QSPI1EN);
    }
#endif

#if CH32_WSPI_USE_WSPI2
    if (&WSPID2 == wspip) {
      disableHB1(RCC_QSPI2EN);
    }
#endif
  }
}

/**
 * @brief   Enables or disables the dual-flash mode.
 * @details  The QSPI peripheral is temporarily disabled to allow the
 *           modification of the DFM bit, then re-enabled.
 *           Note: this resets the peripheral state, so it must be
 *           reconfigured before use if needed.
 *
 * @param[in] wspip     pointer to the @p WSPIDriver object
 * @param[in] enable    @p true to enable dual-flash mode, @p false to disable
 *
 * @notapi
 */
void wspi_lld_set_dual_flash(WSPIDriver *wspip, bool enable) {

  osalDbgCheck(wspip != NULL);

  /* Abort any ongoing transfer and wait for idle.*/
  wspip->qspi->CR |= QSPI_CR_ABORT;
  while ((wspip->qspi->CR & QSPI_CR_ABORT) != 0U) {
  }
  while ((wspip->qspi->SR & QSPI_SR_IDLEF) == 0U) {
  }

  /* Clear all flags.*/
  wspip->qspi->FCR = QSPI_FCR_CTEF | QSPI_FCR_CTCF |
                     QSPI_FCR_CSMF | QSPI_FCR_CTOF;

  if (enable) {
    wspip->qspi->CR |= QSPI_CR_DFM;
  }
  else {
    wspip->qspi->CR &= ~QSPI_CR_DFM;
  }

  /* Re-enable QSPI.*/
  wspip->qspi->CR |= QSPI_CR_EN;
}

/**
 * @brief   Selects the flash device to be accessed.
 * @details  The QSPI peripheral is temporarily disabled to allow the
 *           modification of the FSEL bit, then re-enabled.
 *           Note: this resets the peripheral state, so it must be
 *           reconfigured before use if needed.
 *
 * @param[in] wspip     pointer to the @p WSPIDriver object
 * @param[in] fselect   flash select value, 0=NCS0 (primary),
 *                      1=NCS1 (secondary)
 *
 * @notapi
 */
void wspi_lld_select_flash(WSPIDriver *wspip, uint8_t fselect) {

  osalDbgCheck(wspip != NULL);

  /* Abort any ongoing transfer and wait for idle.*/
  wspip->qspi->CR |= QSPI_CR_ABORT;
  while ((wspip->qspi->CR & QSPI_CR_ABORT) != 0U) {
  }
  while ((wspip->qspi->SR & QSPI_SR_IDLEF) == 0U) {
  }

  /* Clear all flags.*/
  wspip->qspi->FCR = QSPI_FCR_CTEF | QSPI_FCR_CTCF |
                     QSPI_FCR_CSMF | QSPI_FCR_CTOF;

  if (fselect != 0U) {
    wspip->qspi->CR |= QSPI_CR_FSEL;
  }
  else {
    wspip->qspi->CR &= ~QSPI_CR_FSEL;
  }

  /* Re-enable QSPI.*/
  wspip->qspi->CR |= QSPI_CR_EN;
}

/**
 * @brief   Sends a command without data phase.
 * @post    At the end of the operation the configured callback is invoked.
 *
 * @param[in] wspip     pointer to the @p WSPIDriver object
 * @param[in] cmdp      pointer to the command descriptor
 *
 * @notapi
 */
void wspi_lld_command(WSPIDriver *wspip, const wspi_command_t *cmdp) {

  /* Wait for previous transfer to complete and QSPI to be idle.*/
  
  while (!wspi_lld_idle_p(wspip)) {
  }

  wspip->qspi->CR |= QSPI_CR_ABORT;
  while((wspip->qspi->CR & QSPI_CR_ABORT) != 0U) {
  }

  wspi_lld_sync(wspip);


  wspip->qspi->CR &= ~QSPI_CR_DMAEN;

  wspip->qspi->FCR = QSPI_FCR_CTEF | QSPI_FCR_CTCF |
                     QSPI_FCR_CSMF | QSPI_FCR_CTOF;

  wspip->qspi->CCR = wspi_build_ccr(cmdp);
  wspip->qspi->ABR = cmdp->alt;
  wspip->qspi->DLR = 0U;
  if ((cmdp->cfg & WSPI_CFG_ADDR_MODE_MASK) != WSPI_CFG_ADDR_MODE_NONE) {
    wspip->qspi->AR  = cmdp->addr;
  }

  /* Kick off the transfer sequence.*/
  wspip->qspi->CR |= QSPI_CR_START;
}

/**
 * @brief   Sends a command with data over the WSPI bus.
 * @post    At the end of the operation the configured callback is invoked.
 *
 * @param[in] wspip     pointer to the @p WSPIDriver object
 * @param[in] cmdp      pointer to the command descriptor
 * @param[in] n         number of bytes to send
 * @param[in] txbuf     the pointer to the transmit buffer
 *
 * @notapi
 */
void wspi_lld_send(WSPIDriver *wspip, const wspi_command_t *cmdp,
                   size_t n, const uint8_t *txbuf) {

  /* Wait for previous transfer to complete and QSPI to be idle.*/
  while (!wspi_lld_idle_p(wspip)) {
  }
  wspip->qspi->CR |= QSPI_CR_ABORT;
  while((wspip->qspi->CR & QSPI_CR_ABORT) != 0U) {
  }
  wspi_lld_sync(wspip);
  dmaStreamDisable(wspip->dma);
  wspip->qspi->CR &= ~QSPI_CR_DMAEN;

  wspip->qspi->FCR = QSPI_FCR_CTEF | QSPI_FCR_CTCF |
                     QSPI_FCR_CSMF | QSPI_FCR_CTOF;

  wspip->qspi->CCR = wspi_build_ccr(cmdp)
                     | QULDSPI_CCR_FMODE_INDWR;
  wspip->qspi->ABR = cmdp->alt;
  wspip->qspi->DLR = n - 1;
  if ((cmdp->cfg & WSPI_CFG_ADDR_MODE_MASK) != WSPI_CFG_ADDR_MODE_NONE) {
    wspip->qspi->AR  = cmdp->addr;
  }

  /* Configure DMA for memory-to-peripheral transfer.
     QSPI DR is 32-bit, so transfer count is in words.*/

  /* Must disable the stream before modifying CFGR1, otherwise
     write-when-EN=1 is ignored on CH32 DMA.*/

  dmaStreamSetMemory0(wspip->dma, txbuf);
  dmaStreamSetTransactionSize(wspip->dma, n);
  dmaStreamSetMode(wspip->dma, wspip->dmamode |
                               DMA_CFGR1_MINC |
                               DMA_CFGR1_DIR);
  wspip->qspi->CR |= QSPI_CR_DMAEN;
  dmaStreamEnable(wspip->dma);

  wspip->qspi->CR |= QSPI_CR_START;
}

/**
 * @brief   Sends a command then receives data over the WSPI bus.
 * @post    At the end of the operation the configured callback is invoked.
 *
 * @param[in] wspip     pointer to the @p WSPIDriver object
 * @param[in] cmdp      pointer to the command descriptor
 * @param[in] n         number of bytes to receive
 * @param[out] rxbuf    the pointer to the receive buffer
 *
 * @notapi
 */
void wspi_lld_receive(WSPIDriver *wspip, const wspi_command_t *cmdp,
                      size_t n, uint8_t *rxbuf) {

  /* Wait for previous transfer to complete and QSPI to be idle.*/
  while (!wspi_lld_idle_p(wspip)) {
  }
  wspip->qspi->CR |= QSPI_CR_ABORT;
  while((wspip->qspi->CR & QSPI_CR_ABORT) != 0U) {
  }
  wspi_lld_sync(wspip);
  dmaStreamDisable(wspip->dma);
  wspip->qspi->CR &= ~QSPI_CR_DMAEN;

  wspip->qspi->FCR = QSPI_FCR_CTEF | QSPI_FCR_CTCF |
                     QSPI_FCR_CSMF | QSPI_FCR_CTOF;

  wspip->qspi->CCR = wspi_build_ccr(cmdp) |
                     QULDSPI_CCR_FMODE_INDREAD;
  wspip->qspi->ABR = cmdp->alt;
  wspip->qspi->DLR = n - 1;
  if ((cmdp->cfg & WSPI_CFG_ADDR_MODE_MASK) != WSPI_CFG_ADDR_MODE_NONE) {
    wspip->qspi->AR  = cmdp->addr;
  }

  /* Configure DMA for peripheral-to-memory transfer.
     QSPI DR is 32-bit, so transfer count is in words.*/

  /* Must disable the stream before modifying CFGR1, otherwise
     write-when-EN=1 is ignored on CH32 DMA.*/

  dmaStreamSetMemory0(wspip->dma, rxbuf);
  dmaStreamSetTransactionSize(wspip->dma, n);
  dmaStreamSetMode(wspip->dma, wspip->dmamode |
                               DMA_CFGR1_MINC);
  wspip->qspi->CR |= QSPI_CR_DMAEN;
  dmaStreamEnable(wspip->dma);

  wspip->qspi->CR |= QSPI_CR_START;
}

#if (WSPI_SUPPORTS_MEMMAP == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   Maps in memory space a WSPI flash device.
 * @pre     The memory flash device must be initialized appropriately
 *          before mapping it in memory space.
 *
 * @param[in] wspip     pointer to the @p WSPIDriver object
 * @param[in] cmdp      pointer to the command descriptor
 * @param[out] addrp    pointer to the memory start address of the mapped
 *                      flash or @p NULL
 *
 * @notapi
 */
void wspi_lld_map_flash(WSPIDriver *wspip,
                        const wspi_command_t *cmdp,
                        uint8_t **addrp) {

  /* Wait for previous transfer to complete and QSPI to be idle.*/
  wspi_lld_sync(wspip);
  while (!wspi_lld_idle_p(wspip)) {
  }
  dmaStreamDisable(wspip->dma);
  /* Disable the DMA request while in memory mapped mode.*/
  wspip->qspi->CR &= ~QSPI_CR_DMAEN;

  /* Starting memory mapped mode using the passed parameters.*/
  wspip->qspi->CCR = wspi_build_ccr(cmdp) |
                     QULDSPI_CCR_FMODE_MEMMAP;


  /* Kick off the transfer sequence.*/
  wspip->qspi->CR |= QSPI_CR_START;

  /* Mapped flash absolute base address.*/
  if (addrp != NULL) {
#if CH32_WSPI_USE_WSPI1
    if (&WSPID1 == wspip) {
      *addrp = (uint8_t *)QULDSPI_QSPI1_MEM_BASE;
    }
#endif
#if CH32_WSPI_USE_WSPI2
    if (&WSPID2 == wspip) {
      *addrp = (uint8_t *)QULDSPI_QSPI2_MEM_BASE;
    }
#endif
  }
}

/**
 * @brief   Unmaps from memory space a WSPI flash device.
 * @post    The memory flash device must be re-initialized for normal
 *          commands exchange.
 *
 * @param[in] wspip     pointer to the @p WSPIDriver object
 *
 * @notapi
 */
void wspi_lld_unmap_flash(WSPIDriver *wspip) {

  /* Aborting memory mapped mode.*/
  wspip->qspi->CR |= QSPI_CR_ABORT;
  while ((wspip->qspi->CR & QSPI_CR_ABORT) != 0U) {
  }

  /* Wait for idle.*/
  while (!wspi_lld_idle_p(wspip)) {
  }

  /* Clear flags.*/
  wspip->qspi->FCR = QSPI_FCR_CTEF | QSPI_FCR_CTCF |
                     QSPI_FCR_CSMF | QSPI_FCR_CTOF;

  /* Re-enabling DMA request, we are going back to indirect mode.*/
  wspip->qspi->CR |= QSPI_CR_DMAEN;
}
#endif /* WSPI_SUPPORTS_MEMMAP == TRUE */

#endif /* HAL_USE_WSPI */

/** @} */
