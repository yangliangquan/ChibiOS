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
 * @file    hal_i2s_lld.h
 * @brief   CH32 I2S subsystem low level driver header.
 *
 * @addtogroup I2S
 * @{
 */

#ifndef HAL_I2S_LLD_H
#define HAL_I2S_LLD_H

#include "ch32_dma.h"

#if (HAL_USE_I2S == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    Static I2S modes
 * @{
 */
#define CH32_I2S_MODE_SLAVE                 0
#define CH32_I2S_MODE_MASTER                1
#define CH32_I2S_MODE_RX                    2
#define CH32_I2S_MODE_TX                    4
#define CH32_I2S_MODE_RXTX                  (CH32_I2S_MODE_RX |             \
                                             CH32_I2S_MODE_TX)
/** @} */

/**
 * @name    Mode checks
 * @{
 */
#define CH32_I2S_IS_MASTER(mode)            ((mode) & CH32_I2S_MODE_MASTER)
#define CH32_I2S_RX_ENABLED(mode)           ((mode) & CH32_I2S_MODE_RX)
#define CH32_I2S_TX_ENABLED(mode)           ((mode) & CH32_I2S_MODE_TX)
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   I2S1 driver enable switch.
 * @details If set to @p TRUE the support for I2S1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_I2S_USE_I2S1) || defined(__DOXYGEN__)
#define CH32_I2S_USE_I2S1                  FALSE
#endif

/**
 * @brief   I2S2 driver enable switch.
 * @details If set to @p TRUE the support for I2S2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_I2S_USE_I2S2) || defined(__DOXYGEN__)
#define CH32_I2S_USE_I2S2                  FALSE
#endif

/**
 * @brief   I2S1 mode.
 */
#if !defined(CH32_I2S_SPI1_MODE) || defined(__DOXYGEN__)
#define CH32_I2S_SPI1_MODE                 (CH32_I2S_MODE_MASTER |           \
                                            CH32_I2S_MODE_TX)
#endif

/**
 * @brief   I2S2 mode.
 */
#if !defined(CH32_I2S_SPI2_MODE) || defined(__DOXYGEN__)
#define CH32_I2S_SPI2_MODE                 (CH32_I2S_MODE_MASTER |           \
                                            CH32_I2S_MODE_TX)
#endif

/**
 * @brief   I2S1 interrupt priority level setting.
 */
#if !defined(CH32_I2S_SPI1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_I2S_SPI1_IRQ_PRIORITY         10
#endif

/**
 * @brief   I2S2 interrupt priority level setting.
 */
#if !defined(CH32_I2S_SPI2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_I2S_SPI2_IRQ_PRIORITY         10
#endif

/**
 * @brief   I2S1 DMA priority (0..3|lowest..highest).
 */
#if !defined(CH32_I2S_SPI1_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_I2S_SPI1_DMA_PRIORITY         1
#endif

/**
 * @brief   I2S2 DMA priority (0..3|lowest..highest).
 */
#if !defined(CH32_I2S_SPI2_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_I2S_SPI2_DMA_PRIORITY         1
#endif

/**
 * @brief   I2S1 RX DMA stream ID.
 */
#if !defined(CH32_I2S_SPI1_RX_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_I2S_SPI1_RX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   I2S1 TX DMA stream ID.
 */
#if !defined(CH32_I2S_SPI1_TX_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_I2S_SPI1_TX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   I2S2 RX DMA stream ID.
 */
#if !defined(CH32_I2S_SPI2_RX_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_I2S_SPI2_RX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   I2S2 TX DMA stream ID.
 */
#if !defined(CH32_I2S_SPI2_TX_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_I2S_SPI2_TX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   I2S DMA error hook.
 */
#if !defined(CH32_I2S_DMA_ERROR_HOOK) || defined(__DOXYGEN__)
#define CH32_I2S_DMA_ERROR_HOOK(i2sp)      osalSysHalt("DMA failure")
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if CH32_I2S_USE_I2S1 && !CH32_SPI1_SUPPORTS_I2S
#error "SPI1 does not support I2S mode"
#endif

#if CH32_I2S_USE_I2S2 && !CH32_SPI2_SUPPORTS_I2S
#error "SPI2 does not support I2S mode"
#endif

#if CH32_I2S_RX_ENABLED(CH32_I2S_SPI1_MODE) &&                              \
    CH32_I2S_TX_ENABLED(CH32_I2S_SPI1_MODE)
#error "I2S1 RX and TX mode not supported in this driver implementation"
#endif

#if CH32_I2S_RX_ENABLED(CH32_I2S_SPI2_MODE) &&                              \
    CH32_I2S_TX_ENABLED(CH32_I2S_SPI2_MODE)
#error "I2S2 RX and TX mode not supported in this driver implementation"
#endif

#if CH32_I2S_USE_I2S1 && !CH32_HAS_SPI1
#error "SPI1 not present in the selected device"
#endif

#if CH32_I2S_USE_I2S2 && !CH32_HAS_SPI2
#error "SPI2 not present in the selected device"
#endif

#if !CH32_I2S_USE_I2S1 && !CH32_I2S_USE_I2S2
#error "I2S driver activated but no SPI peripheral assigned"
#endif

#if CH32_I2S_USE_I2S1 &&                                                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(CH32_I2S_SPI1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to I2S1"
#endif

#if CH32_I2S_USE_I2S2 &&                                                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(CH32_I2S_SPI2_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to I2S2"
#endif

#if CH32_I2S_USE_I2S1 &&                                                    \
    !CH32_DMA_IS_VALID_PRIORITY(CH32_I2S_SPI1_DMA_PRIORITY)
#error "Invalid DMA priority assigned to I2S1"
#endif

#if CH32_I2S_USE_I2S2 &&                                                    \
    !CH32_DMA_IS_VALID_PRIORITY(CH32_I2S_SPI2_DMA_PRIORITY)
#error "Invalid DMA priority assigned to I2S2"
#endif

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
 * @brief   Low level fields of the I2S driver structure.
 */
#define i2s_lld_driver_fields                                               \
  /* Pointer to the SPIx registers block.*/                                 \
  SPI_TypeDef               *spi;                                           \
  /* Calculated part of the I2SCFGR register.*/                             \
  uint16_t                  cfg;                                            \
  /* Receive DMA stream or @p NULL.*/                                       \
  const ch32_dma_stream_t   *dmarx;                                         \
  /* Transmit DMA stream or @p NULL.*/                                      \
  const ch32_dma_stream_t   *dmatx;                                         \
  /* RX DMA mode bit mask.*/                                                \
  uint32_t                  rxdmamode;                                      \
  /* TX DMA mode bit mask.*/                                                \
  uint32_t                  txdmamode;

/**
 * @brief   Low level fields of the I2S configuration structure.
 */
#define i2s_lld_config_fields                                               \
  /* Configuration of the I2SCFGR register.                                 \
     NOTE: See the CH32 reference manual, this register is used for         \
           the I2S configuration, the following bits must not be            \
           specified because handled directly by the driver:                \
           - I2SMOD                                                         \
           - I2SE                                                           \
           - I2SCFG                                                         \
   */                                                                       \
  int16_t                   i2scfgr;                                        \
  /* Configuration of the I2SPR register.                                   \
     NOTE: See the CH32 reference manual, this register is used for         \
           the I2S clock setup.*/                                           \
  int16_t                   i2spr;

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (CH32_I2S_USE_I2S1 == TRUE) && !defined(__DOXYGEN__)
extern I2SDriver I2SD1;
#endif

#if (CH32_I2S_USE_I2S2 == TRUE) && !defined(__DOXYGEN__)
extern I2SDriver I2SD2;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void i2s_lld_init(void);
  void i2s_lld_start(I2SDriver *i2sp);
  void i2s_lld_stop(I2SDriver *i2sp);
  void i2s_lld_start_exchange(I2SDriver *i2sp);
  void i2s_lld_stop_exchange(I2SDriver *i2sp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_I2S == TRUE */

#endif /* HAL_I2S_LLD_H */

/** @} */
