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
 * @file    hal_spi_lld.h
 * @brief   CH32 SPI (v1) subsystem low level driver header.
 *
 * @addtogroup SPI_V1
 * @{
 */

#ifndef HAL_SPI_LLD_H
#define HAL_SPI_LLD_H

#include "ch32_dma.h"

#if (HAL_USE_SPI == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Circular mode support flag.
 */
#define SPI_SUPPORTS_CIRCULAR           TRUE

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    CH32 configuration options
 * @{
 */
/**
 * @brief   SPI1 driver enable switch.
 * @details If set to @p TRUE the support for SPI1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SPI_USE_SPI1) || defined(__DOXYGEN__)
#define CH32_SPI_USE_SPI1                  FALSE
#endif

/**
 * @brief   SPI2 driver enable switch.
 * @details If set to @p TRUE the support for SPI2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SPI_USE_SPI2) || defined(__DOXYGEN__)
#define CH32_SPI_USE_SPI2                  FALSE
#endif

/**
 * @brief   SPI3 driver enable switch.
 * @details If set to @p TRUE the support for SPI3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SPI_USE_SPI3) || defined(__DOXYGEN__)
#define CH32_SPI_USE_SPI3                  FALSE
#endif

/**
 * @brief   SPI4 driver enable switch.
 * @details If set to @p TRUE the support for SPI4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SPI_USE_SPI4) || defined(__DOXYGEN__)
#define CH32_SPI_USE_SPI4                  FALSE
#endif

/**
 * @brief   SPI1 DMA priority.
 * @note    The default is @p 3 which correspond to the highest priority.
 */
#if !defined(CH32_SPI_SPI1_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_SPI_SPI1_DMA_PRIORITY         3
#endif

/**
 * @brief   SPI2 DMA priority.
 * @note    The default is @p 3 which correspond to the highest priority.
 */
#if !defined(CH32_SPI_SPI2_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_SPI_SPI2_DMA_PRIORITY         3
#endif

/**
 * @brief   SPI3 DMA priority.
 * @note    The default is @p 3 which correspond to the highest priority.
 */
#if !defined(CH32_SPI_SPI3_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_SPI_SPI3_DMA_PRIORITY         3
#endif

/**
 * @brief   SPI4 DMA priority.
 * @note    The default is @p 3 which correspond to the highest priority.
 */
#if !defined(CH32_SPI_SPI4_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_SPI_SPI4_DMA_PRIORITY         3
#endif

/**
 * @brief   SPI1 interrupt priority.
 */
#if !defined(CH32_SPI_SPI1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_SPI_SPI1_IRQ_PRIORITY         10
#endif

/**
 * @brief   SPI2 interrupt priority.
 */
#if !defined(CH32_SPI_SPI2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_SPI_SPI2_IRQ_PRIORITY         10
#endif

/**
 * @brief   SPI3 interrupt priority.
 */
#if !defined(CH32_SPI_SPI3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_SPI_SPI3_IRQ_PRIORITY         10
#endif

/**
 * @brief   SPI4 interrupt priority.
 */
#if !defined(CH32_SPI_SPI4_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_SPI_SPI4_IRQ_PRIORITY         10
#endif

/**
 * @brief   SPI1 RX DMA stream ID.
 */
#if !defined(CH32_SPI_SPI1_RX_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_SPI_SPI1_RX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   SPI1 TX DMA stream ID.
 */
#if !defined(CH32_SPI_SPI1_TX_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_SPI_SPI1_TX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   SPI2 RX DMA stream ID.
 */
#if !defined(CH32_SPI_SPI2_RX_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_SPI_SPI2_RX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   SPI2 TX DMA stream ID.
 */
#if !defined(CH32_SPI_SPI2_TX_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_SPI_SPI2_TX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   SPI3 RX DMA stream ID.
 */
#if !defined(CH32_SPI_SPI3_RX_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_SPI_SPI3_RX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   SPI3 TX DMA stream ID.
 */
#if !defined(CH32_SPI_SPI3_TX_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_SPI_SPI3_TX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   SPI4 RX DMA stream ID.
 */
#if !defined(CH32_SPI_SPI4_RX_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_SPI_SPI4_RX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   SPI4 TX DMA stream ID.
 */
#if !defined(CH32_SPI_SPI4_TX_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_SPI_SPI4_TX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   DMA error hook.
 */
#if !defined(CH32_SPI_DMA_ERROR_HOOK) || defined(__DOXYGEN__)
#define CH32_SPI_DMA_ERROR_HOOK(spip)                                         \
  osalSysHalt("DMA failure")
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/* DMA driver requirement.*/
#if !defined(CH32_DMA_REQUIRED)
#define CH32_DMA_REQUIRED                  TRUE
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Low level fields of the SPI driver structure.
 */
#define spi_lld_driver_fields                                               \
  /* SPI peripheral register base address.*/                                \
  SPI_TypeDef               *spi;                                           \
  /* RX DMA stream pointer.*/                                               \
  const ch32_dma_stream_t   *dmarx;                                         \
  /* TX DMA stream pointer.*/                                               \
  const ch32_dma_stream_t   *dmatx;                                         \
  /* RX DMA mode value.*/                                                   \
  uint32_t                  rxdmamode;                                      \
  /* TX DMA mode value.*/                                                   \
  uint32_t                  txdmamode;

/**
 * @brief   Low level fields of the SPI configuration structure.
 */
#define spi_lld_config_fields                                               \
  /* Initial value for the CTLR1 register.*/                                \
  uint16_t                  cr1;                                            \
  /* Initial value for the CTLR2 register.*/                                \
  uint16_t                  cr2;

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (CH32_SPI_USE_SPI1 == TRUE) && !defined(__DOXYGEN__)
extern SPIDriver SPID1;
#endif

#if (CH32_SPI_USE_SPI2 == TRUE) && !defined(__DOXYGEN__)
extern SPIDriver SPID2;
#endif

#if (CH32_SPI_USE_SPI3 == TRUE) && !defined(__DOXYGEN__)
extern SPIDriver SPID3;
#endif

#if (CH32_SPI_USE_SPI4 == TRUE) && !defined(__DOXYGEN__)
extern SPIDriver SPID4;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void spi_lld_init(void);
  void spi_lld_start(SPIDriver *spip);
  void spi_lld_stop(SPIDriver *spip);
#if (SPI_SELECT_MODE == SPI_SELECT_MODE_LLD) || defined(__DOXYGEN__)
  void spi_lld_select(SPIDriver *spip);
  void spi_lld_unselect(SPIDriver *spip);
#endif
  void spi_lld_ignore(SPIDriver *spip, size_t n);
  void spi_lld_exchange(SPIDriver *spip, size_t n,
                        const void *txbuf, void *rxbuf);
  void spi_lld_send(SPIDriver *spip, size_t n, const void *txbuf);
  void spi_lld_receive(SPIDriver *spip, size_t n, void *rxbuf);
#if (SPI_SUPPORTS_CIRCULAR == TRUE) || defined(__DOXYGEN__)
  void spi_lld_abort(SPIDriver *spip);
#endif
  uint16_t spi_lld_polled_exchange(SPIDriver *spip, uint16_t frame);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SPI == TRUE */

#endif /* HAL_SPI_LLD_H */

/** @} */
