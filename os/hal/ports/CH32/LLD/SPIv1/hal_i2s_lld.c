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
 * @file    SPIv1/hal_i2s_lld.c
 * @brief   CH32 I2S subsystem low level driver source.
 *
 * @addtogroup I2S
 * @{
 */

#include "hal.h"

#if (HAL_USE_I2S == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/**
 * @brief   DMA priority level mapping.
 */
#define CH32_I2S_DMA_PRIORITY_0           DMA_Priority_Low
#define CH32_I2S_DMA_PRIORITY_1           DMA_Priority_Medium
#define CH32_I2S_DMA_PRIORITY_2           DMA_Priority_High
#define CH32_I2S_DMA_PRIORITY_3           DMA_Priority_VeryHigh

/*
 * Static I2S settings for I2S1 (SPI1).
 */
#if !CH32_I2S_IS_MASTER(CH32_I2S_SPI1_MODE)
#if CH32_I2S_TX_ENABLED(CH32_I2S_SPI1_MODE)
#define CH32_I2S1_CFGR_CFG                 0
#endif
#if CH32_I2S_RX_ENABLED(CH32_I2S_SPI1_MODE)
#define CH32_I2S1_CFGR_CFG                 SPI_I2SCFGR_I2SCFG_0
#endif
#else /* !CH32_I2S_IS_MASTER(CH32_I2S_SPI1_MODE) */
#if CH32_I2S_TX_ENABLED(CH32_I2S_SPI1_MODE)
#define CH32_I2S1_CFGR_CFG                 SPI_I2SCFGR_I2SCFG_1
#endif
#if CH32_I2S_RX_ENABLED(CH32_I2S_SPI1_MODE)
#define CH32_I2S1_CFGR_CFG                 (SPI_I2SCFGR_I2SCFG_1 |         \
                                            SPI_I2SCFGR_I2SCFG_0)
#endif
#endif /* !CH32_I2S_IS_MASTER(CH32_I2S_SPI1_MODE) */

/*
 * Static I2S settings for I2S2 (SPI2).
 */
#if !CH32_I2S_IS_MASTER(CH32_I2S_SPI2_MODE)
#if CH32_I2S_TX_ENABLED(CH32_I2S_SPI2_MODE)
#define CH32_I2S2_CFGR_CFG                 0
#endif
#if CH32_I2S_RX_ENABLED(CH32_I2S_SPI2_MODE)
#define CH32_I2S2_CFGR_CFG                 SPI_I2SCFGR_I2SCFG_0
#endif
#else /* !CH32_I2S_IS_MASTER(CH32_I2S_SPI2_MODE) */
#if CH32_I2S_TX_ENABLED(CH32_I2S_SPI2_MODE)
#define CH32_I2S2_CFGR_CFG                 SPI_I2SCFGR_I2SCFG_1
#endif
#if CH32_I2S_RX_ENABLED(CH32_I2S_SPI2_MODE)
#define CH32_I2S2_CFGR_CFG                 (SPI_I2SCFGR_I2SCFG_1 |         \
                                            SPI_I2SCFGR_I2SCFG_0)
#endif
#endif /* !CH32_I2S_IS_MASTER(CH32_I2S_SPI2_MODE) */

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief I2S1 driver identifier.*/
#if (CH32_I2S_USE_I2S1 == TRUE) || defined(__DOXYGEN__)
I2SDriver I2SD1;
#endif

/** @brief I2S2 driver identifier.*/
#if (CH32_I2S_USE_I2S2 == TRUE) || defined(__DOXYGEN__)
I2SDriver I2SD2;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

#if CH32_I2S_RX_ENABLED(CH32_I2S_SPI1_MODE) ||                              \
    CH32_I2S_RX_ENABLED(CH32_I2S_SPI2_MODE) || defined(__DOXYGEN__)
/**
 * @brief   Shared end-of-rx service routine.
 *
 * @param[in] i2sp      pointer to the @p I2SDriver object
 * @param[in] flags     pre-shifted content of the ISR register
 */
static void i2s_lld_serve_rx_interrupt(I2SDriver *i2sp, uint32_t flags) {

  (void)i2sp;

  /* DMA errors handling.*/
#if defined(CH32_I2S_DMA_ERROR_HOOK)
  if ((flags & DMA_CFGR1_TEIE) != 0) {
    CH32_I2S_DMA_ERROR_HOOK(i2sp);
  }
#endif

  /* Callbacks handling, note it is portable code defined in the high
     level driver.*/
  if ((flags & DMA_CFGR1_TCIE) != 0) {
    /* Transfer complete processing.*/
    _i2s_isr_full_code(i2sp);
  }
  else if ((flags & DMA_CFGR1_HTIE) != 0) {
    /* Half transfer processing.*/
    _i2s_isr_half_code(i2sp);
  }
}
#endif

#if CH32_I2S_TX_ENABLED(CH32_I2S_SPI1_MODE) ||                              \
    CH32_I2S_TX_ENABLED(CH32_I2S_SPI2_MODE) || defined(__DOXYGEN__)
/**
 * @brief   Shared end-of-tx service routine.
 *
 * @param[in] i2sp      pointer to the @p I2SDriver object
 * @param[in] flags     pre-shifted content of the ISR register
 */
static void i2s_lld_serve_tx_interrupt(I2SDriver *i2sp, uint32_t flags) {

  (void)i2sp;

  /* DMA errors handling.*/
#if defined(CH32_I2S_DMA_ERROR_HOOK)
  if ((flags & DMA_CFGR1_TEIE) != 0) {
    CH32_I2S_DMA_ERROR_HOOK(i2sp);
  }
#endif

  /* Callbacks handling, note it is portable code defined in the high
     level driver.*/
  if ((flags & DMA_CFGR1_TCIE) != 0) {
    /* Transfer complete processing.*/
    _i2s_isr_full_code(i2sp);
  }
  else if ((flags & DMA_CFGR1_HTIE) != 0) {
    /* Half transfer processing.*/
    _i2s_isr_half_code(i2sp);
  }
}
#endif

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level I2S driver initialization.
 *
 * @notapi
 */
void i2s_lld_init(void) {

  /*
   * Common DMA mode base values.
   * I2S always uses 16-bit half-word transfers with circular mode.
   */
#if CH32_I2S_USE_I2S1
  i2sObjectInit(&I2SD1);
  I2SD1.spi       = SPI1;
  I2SD1.cfg       = CH32_I2S1_CFGR_CFG;
  I2SD1.dmarx     = NULL;
  I2SD1.dmatx     = NULL;
#if CH32_I2S_RX_ENABLED(CH32_I2S_SPI1_MODE)
  I2SD1.rxdmamode = DMA_PeripheralDataSize_HalfWord |
                    DMA_MemoryDataSize_HalfWord |
                    DMA_DIR_PeripheralSRC |
                    DMA_MemoryInc_Enable |
                    DMA_Mode_Circular |
                    DMA_CFGR1_HTIE |
                    DMA_CFGR1_TCIE |
                    DMA_CFGR1_TEIE;
#else
  I2SD1.rxdmamode = 0;
#endif
#if CH32_I2S_TX_ENABLED(CH32_I2S_SPI1_MODE)
  I2SD1.txdmamode = DMA_PeripheralDataSize_HalfWord |
                    DMA_MemoryDataSize_HalfWord |
                    DMA_DIR_PeripheralDST |
                    DMA_MemoryInc_Enable |
                    DMA_Mode_Circular |
                    DMA_CFGR1_HTIE |
                    DMA_CFGR1_TCIE |
                    DMA_CFGR1_TEIE;
#else
  I2SD1.txdmamode = 0;
#endif
#endif

#if CH32_I2S_USE_I2S2
  i2sObjectInit(&I2SD2);
  I2SD2.spi       = SPI2;
  I2SD2.cfg       = CH32_I2S2_CFGR_CFG;
  I2SD2.dmarx     = NULL;
  I2SD2.dmatx     = NULL;
#if CH32_I2S_RX_ENABLED(CH32_I2S_SPI2_MODE)
  I2SD2.rxdmamode = DMA_PeripheralDataSize_HalfWord |
                    DMA_MemoryDataSize_HalfWord |
                    DMA_DIR_PeripheralSRC |
                    DMA_MemoryInc_Enable |
                    DMA_Mode_Circular |
                    DMA_CFGR1_HTIE |
                    DMA_CFGR1_TCIE |
                    DMA_CFGR1_TEIE;
#else
  I2SD2.rxdmamode = 0;
#endif
#if CH32_I2S_TX_ENABLED(CH32_I2S_SPI2_MODE)
  I2SD2.txdmamode = DMA_PeripheralDataSize_HalfWord |
                    DMA_MemoryDataSize_HalfWord |
                    DMA_DIR_PeripheralDST |
                    DMA_MemoryInc_Enable |
                    DMA_Mode_Circular |
                    DMA_CFGR1_HTIE |
                    DMA_CFGR1_TCIE |
                    DMA_CFGR1_TEIE;
#else
  I2SD2.txdmamode = 0;
#endif
#endif
}

/**
 * @brief   Configures and activates the I2S peripheral.
 *
 * @param[in] i2sp      pointer to the @p I2SDriver object
 *
 * @notapi
 */
void i2s_lld_start(I2SDriver *i2sp) {

  /* If in stopped state then enables the SPI and DMA clocks.*/
  if (i2sp->state == I2S_STOP) {

#if CH32_I2S_USE_I2S1
    if (&I2SD1 == i2sp) {

      /* Enabling I2S unit clock.*/
      resetHB2(RCC_SPI1RST);
      enableHB2(RCC_SPI1EN);

#if CH32_I2S_RX_ENABLED(CH32_I2S_SPI1_MODE)
      i2sp->dmarx = dmaStreamAllocI(CH32_I2S_SPI1_RX_DMA_STREAM,
                                    CH32_I2S_SPI1_IRQ_PRIORITY,
                                    (ch32_dmaisr_t)i2s_lld_serve_rx_interrupt,
                                    (void *)i2sp);
      osalDbgAssert(i2sp->dmarx != NULL, "unable to allocate stream");

      /* Setting DMAMUX request source.*/
      dmaSetRequestSource(i2sp->dmarx, DMA_MUX_SPI1_RX);

      /* CRs settings are done here because those never changes until
         the driver is stopped.*/
      i2sp->spi->CTLR1 = 0;
      i2sp->spi->CTLR2 = SPI_CTLR2_RXDMAEN;
#endif
#if CH32_I2S_TX_ENABLED(CH32_I2S_SPI1_MODE)
      i2sp->dmatx = dmaStreamAllocI(CH32_I2S_SPI1_TX_DMA_STREAM,
                                    CH32_I2S_SPI1_IRQ_PRIORITY,
                                    (ch32_dmaisr_t)i2s_lld_serve_tx_interrupt,
                                    (void *)i2sp);
      osalDbgAssert(i2sp->dmatx != NULL, "unable to allocate stream");

      /* Setting DMAMUX request source.*/
      dmaSetRequestSource(i2sp->dmatx, DMA_MUX_SPI1_TX);

      /* CRs settings are done here because those never changes until
         the driver is stopped.*/
      i2sp->spi->CTLR1 = 0;
      i2sp->spi->CTLR2 = SPI_CTLR2_TXDMAEN;
#endif
    }
#endif

#if CH32_I2S_USE_I2S2
    if (&I2SD2 == i2sp) {

      /* Enabling I2S unit clock.*/
      resetHB1(RCC_SPI2RST);
      enableHB1(RCC_SPI2EN);

#if CH32_I2S_RX_ENABLED(CH32_I2S_SPI2_MODE)
      i2sp->dmarx = dmaStreamAllocI(CH32_I2S_SPI2_RX_DMA_STREAM,
                                    CH32_I2S_SPI2_IRQ_PRIORITY,
                                    (ch32_dmaisr_t)i2s_lld_serve_rx_interrupt,
                                    (void *)i2sp);
      osalDbgAssert(i2sp->dmarx != NULL, "unable to allocate stream");

      /* Setting DMAMUX request source.*/
      dmaSetRequestSource(i2sp->dmarx, DMA_MUX_SPI2_RX);

      /* CRs settings are done here because those never changes until
         the driver is stopped.*/
      i2sp->spi->CTLR1 = 0;
      i2sp->spi->CTLR2 = SPI_CTLR2_RXDMAEN;
#endif
#if CH32_I2S_TX_ENABLED(CH32_I2S_SPI2_MODE)
      i2sp->dmatx = dmaStreamAllocI(CH32_I2S_SPI2_TX_DMA_STREAM,
                                    CH32_I2S_SPI2_IRQ_PRIORITY,
                                    (ch32_dmaisr_t)i2s_lld_serve_tx_interrupt,
                                    (void *)i2sp);
      osalDbgAssert(i2sp->dmatx != NULL, "unable to allocate stream");

      /* Setting DMAMUX request source.*/
      dmaSetRequestSource(i2sp->dmatx, DMA_MUX_SPI2_TX);

      /* CRs settings are done here because those never changes until
         the driver is stopped.*/
      i2sp->spi->CTLR1 = 0;
      i2sp->spi->CTLR2 = SPI_CTLR2_TXDMAEN;
#endif
    }
#endif

    /* DMA priority setting.*/
    {
      uint32_t dma_priority;

#if CH32_I2S_USE_I2S1
      if (&I2SD1 == i2sp) {
        dma_priority = CH32_I2S_SPI1_DMA_PRIORITY;
      }
      else
#endif
#if CH32_I2S_USE_I2S2
      if (&I2SD2 == i2sp) {
        dma_priority = CH32_I2S_SPI2_DMA_PRIORITY;
      }
      else
#endif
      {
        dma_priority = 0;
        osalDbgAssert(false, "invalid I2S instance");
      }

      {
        static const uint32_t dma_priorities[4] = {
          CH32_I2S_DMA_PRIORITY_0,
          CH32_I2S_DMA_PRIORITY_1,
          CH32_I2S_DMA_PRIORITY_2,
          CH32_I2S_DMA_PRIORITY_3
        };
        dma_priority = dma_priorities[dma_priority & 3U];
      }

      if (i2sp->rxdmamode != 0) {
        i2sp->rxdmamode = (i2sp->rxdmamode & ~DMA_Priority_VeryHigh) |
                          dma_priority;
      }
      if (i2sp->txdmamode != 0) {
        i2sp->txdmamode = (i2sp->txdmamode & ~DMA_Priority_VeryHigh) |
                          dma_priority;
      }
    }
  }

  /* I2S (re)configuration.*/
  i2sp->spi->I2SPR   = (uint16_t)i2sp->config->i2spr;
  i2sp->spi->I2SCFGR = (uint16_t)(i2sp->config->i2scfgr | i2sp->cfg |
                                   SPI_I2SCFGR_I2SMOD);
}

/**
 * @brief   Deactivates the I2S peripheral.
 *
 * @param[in] i2sp      pointer to the @p I2SDriver object
 *
 * @notapi
 */
void i2s_lld_stop(I2SDriver *i2sp) {

  /* If in ready state then disables the SPI clock.*/
  if (i2sp->state == I2S_READY) {

    /* SPI disable.*/
    i2sp->spi->CTLR2 = 0;
    if (NULL != i2sp->dmarx) {
      dmaStreamFreeI(i2sp->dmarx);
      i2sp->dmarx = NULL;
    }
    if (NULL != i2sp->dmatx) {
      dmaStreamFreeI(i2sp->dmatx);
      i2sp->dmatx = NULL;
    }

#if CH32_I2S_USE_I2S1
    if (&I2SD1 == i2sp) {
      resetHB2(RCC_SPI1RST);
      disableHB2(RCC_SPI1EN);
    }
#endif

#if CH32_I2S_USE_I2S2
    if (&I2SD2 == i2sp) {
      resetHB1(RCC_SPI2RST);
      disableHB1(RCC_SPI2EN);
    }
#endif
  }
}

/**
 * @brief   Starts a I2S data exchange.
 *
 * @param[in] i2sp      pointer to the @p I2SDriver object
 *
 * @notapi
 */
void i2s_lld_start_exchange(I2SDriver *i2sp) {
  size_t size = i2sp->config->size;

  /* In 32 bit modes the DMA has to perform double operations because fetches
     are always performed using 16 bit accesses.
     DATLEN   CHLEN   SIZE
     00 (16)  0 (16)  16
     00 (16)  1 (32)  16
     01 (24)  X       32
     10 (32)  X       32
     11 (NA)  X       NA
     */
  if ((i2sp->config->i2scfgr & SPI_I2SCFGR_DATLEN) != 0)
    size *= 2;

  /* RX DMA setup.*/
  if (NULL != i2sp->dmarx) {
    dmaStreamSetMode(i2sp->dmarx, i2sp->rxdmamode);
    dmaStreamSetPeripheral(i2sp->dmarx, &i2sp->spi->DATAR);
    dmaStreamSetMemory0(i2sp->dmarx, i2sp->config->rx_buffer);
    dmaStreamSetTransactionSize(i2sp->dmarx, size);
    dmaStreamEnable(i2sp->dmarx);
  }

  /* TX DMA setup.*/
  if (NULL != i2sp->dmatx) {
    dmaStreamSetMode(i2sp->dmatx, i2sp->txdmamode);
    dmaStreamSetPeripheral(i2sp->dmatx, &i2sp->spi->DATAR);
    dmaStreamSetMemory0(i2sp->dmatx, i2sp->config->tx_buffer);
    dmaStreamSetTransactionSize(i2sp->dmatx, size);
    dmaStreamEnable(i2sp->dmatx);
  }

  /* Starting transfer.*/
  i2sp->spi->I2SCFGR |= SPI_I2SCFGR_I2SE;
}

/**
 * @brief   Stops the ongoing data exchange.
 * @details The ongoing data exchange, if any, is stopped, if the driver
 *          was not active the function does nothing.
 *
 * @param[in] i2sp      pointer to the @p I2SDriver object
 *
 * @notapi
 */
void i2s_lld_stop_exchange(I2SDriver *i2sp) {

  /* Stop TX DMA, if enabled.*/
  if (NULL != i2sp->dmatx) {
    dmaStreamDisable(i2sp->dmatx);

    /* From the RM: To switch off the I2S, by clearing I2SE, it is mandatory
       to wait for TXE = 1 and BSY = 0.*/
    while ((i2sp->spi->STATR & (SPI_STATR_TXE | SPI_STATR_BSY)) !=
           SPI_STATR_TXE)
      ;

    /* Stop SPI/I2S peripheral.*/
    i2sp->spi->I2SCFGR &= ~SPI_I2SCFGR_I2SE;
  }

  /* Stop RX DMA, if enabled then draining the RX DR.*/
  if (NULL != i2sp->dmarx) {
    dmaStreamDisable(i2sp->dmarx);

    /* Waiting for some data to be present in RX DR.*/
    while ((i2sp->spi->STATR & SPI_STATR_RXNE) != SPI_STATR_RXNE)
      ;

    /* Stop SPI/I2S peripheral.*/
    i2sp->spi->I2SCFGR &= ~SPI_I2SCFGR_I2SE;

    /* Purging data in DR.*/
    while ((i2sp->spi->STATR & SPI_STATR_RXNE) != 0)
      (void) i2sp->spi->DATAR;
  }
}

#endif /* HAL_USE_I2S */

/** @} */
