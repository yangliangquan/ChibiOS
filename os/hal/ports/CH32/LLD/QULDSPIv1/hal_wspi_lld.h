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
 * @file    QULDSPIv1/hal_wspi_lld.h
 * @brief   CH32 WSPI subsystem low level driver header.
 *
 * @addtogroup WSPI
 * @{
 */

#ifndef HAL_WSPI_LLD_H
#define HAL_WSPI_LLD_H

#if (HAL_USE_WSPI == TRUE) || defined(__DOXYGEN__)

#include "ch32_dma.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    WSPI implementation capabilities
 * @{
 */
#define WSPI_SUPPORTS_MEMMAP                TRUE
#define WSPI_DEFAULT_CFG_MASKS              FALSE
/** @} */

/**
 * @name    Transfer options
 * @{
 */
#define WSPI_CFG_CMD_MODE_MASK              (3U << 8)
#define WSPI_CFG_CMD_MODE_NONE              (0U << 8)
#define WSPI_CFG_CMD_MODE_ONE_LINE          (1U << 8)
#define WSPI_CFG_CMD_MODE_TWO_LINES         (2U << 8)
#define WSPI_CFG_CMD_MODE_FOUR_LINES        (3U << 8)
#define WSPI_CFG_CMD_MODE_EIGHT_LINES       0U          /* Fake.*/

#define WSPI_CFG_CMD_DTR                    (1U << 31)

#define WSPI_CFG_CMD_SIZE_MASK              0U
#define WSPI_CFG_CMD_SIZE_8                 0U
#define WSPI_CFG_CMD_SIZE_16                0U          /* Fake.*/
#define WSPI_CFG_CMD_SIZE_24                0U          /* Fake.*/
#define WSPI_CFG_CMD_SIZE_32                0U          /* Fake.*/

#define WSPI_CFG_ADDR_MODE_MASK             (3U << 10)
#define WSPI_CFG_ADDR_MODE_NONE             (0U << 10)
#define WSPI_CFG_ADDR_MODE_ONE_LINE         (1U << 10)
#define WSPI_CFG_ADDR_MODE_TWO_LINES        (2U << 10)
#define WSPI_CFG_ADDR_MODE_FOUR_LINES       (3U << 10)
#define WSPI_CFG_ADDR_MODE_EIGHT_LINES      0U          /* Fake.*/

#define WSPI_CFG_ADDR_DTR                   (1U << 31)

#define WSPI_CFG_ADDR_SIZE_MASK             (3U << 12)
#define WSPI_CFG_ADDR_SIZE_8                (0U << 12)
#define WSPI_CFG_ADDR_SIZE_16               (1U << 12)
#define WSPI_CFG_ADDR_SIZE_24               (2U << 12)
#define WSPI_CFG_ADDR_SIZE_32               (3U << 12)

#define WSPI_CFG_ALT_MODE_MASK              (3U << 14)
#define WSPI_CFG_ALT_MODE_NONE              (0U << 14)
#define WSPI_CFG_ALT_MODE_ONE_LINE          (1U << 14)
#define WSPI_CFG_ALT_MODE_TWO_LINES         (2U << 14)
#define WSPI_CFG_ALT_MODE_FOUR_LINES        (3U << 14)
#define WSPI_CFG_ALT_MODE_EIGHT_LINES       0U          /* Fake.*/

#define WSPI_CFG_ALT_DTR                    (1U << 31)

#define WSPI_CFG_ALT_SIZE_MASK              (3U << 16)
#define WSPI_CFG_ALT_SIZE_8                 (0U << 16)
#define WSPI_CFG_ALT_SIZE_16                (1U << 16)
#define WSPI_CFG_ALT_SIZE_24                (2U << 16)
#define WSPI_CFG_ALT_SIZE_32                (3U << 16)

#define WSPI_CFG_DATA_MODE_MASK             (3U << 24)
#define WSPI_CFG_DATA_MODE_NONE             (0U << 24)
#define WSPI_CFG_DATA_MODE_ONE_LINE         (1U << 24)
#define WSPI_CFG_DATA_MODE_TWO_LINES        (2U << 24)
#define WSPI_CFG_DATA_MODE_FOUR_LINES       (3U << 24)
#define WSPI_CFG_DATA_MODE_EIGHT_LINES      0U          /* Fake.*/

#define WSPI_CFG_DATA_DTR                   (1U << 31)

#define WSPI_CFG_DQS_ENABLE                 0U          /* Fake.*/

#define WSPI_CFG_SIOO                       (1U << 28)

#define WSPI_CFG_ALL_DTR                    (WSPI_CFG_CMD_DTR   |           \
                                             WSPI_CFG_ADDR_DTR  |           \
                                             WSPI_CFG_ALT_DTR   |           \
                                             WSPI_CFG_DATA_DTR)
/** @} */

/**
 * @name    Helpers for CCR register
 * @{
 */
#define QULDSPI_CCR_DUMMY_CYCLES_MASK       (0x1FU << 18)
#define QULDSPI_CCR_DUMMY_CYCLES(n)         ((n) << 18)
/** @} */

/**
 * @name    CR register options
 * @{
 */
#define QULDSPI_CR_SIOXEN                   (1U << 13)
/** @} */

/**
 * @name    DCR register options
 * @{
 */
#define QULDSPI_DCR_CKMODE                  (1U << 0)
#define QULDSPI_DCR_CSHT_MASK               (7U << 8)
#define QULDSPI_DCR_CSHT(n)                 ((n) << 8)
#define QULDSPI_DCR_FSIZE_MASK              (31U << 16)
#define QULDSPI_DCR_FSIZE(n)                ((n) << 16)
/** @} */

/**
 * @name    FMODE values
 * @{
 */
#define QULDSPI_CCR_FMODE_INDWR             (0U << 26)
#define QULDSPI_CCR_FMODE_INDREAD           (1U << 26)
#define QULDSPI_CCR_FMODE_AUTOPOLL          (2U << 26)
#define QULDSPI_CCR_FMODE_MEMMAP            (3U << 26)
/** @} */

/**
 * @name    Memory-mapped base addresses
 * @{
 */
#define QULDSPI_QSPI1_MEM_BASE              0x90000000U
#define QULDSPI_QSPI2_MEM_BASE              0x70000000U
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   WSPID1 driver enable switch.
 * @details If set to @p TRUE the support for QSPI1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_WSPI_USE_WSPI1) || defined(__DOXYGEN__)
#define CH32_WSPI_USE_WSPI1                 FALSE
#endif

/**
 * @brief   WSPID2 driver enable switch.
 * @details If set to @p TRUE the support for QSPI2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_WSPI_USE_WSPI2) || defined(__DOXYGEN__)
#define CH32_WSPI_USE_WSPI2                 FALSE
#endif

/**
 * @brief   QSPI1 DMA priority (0..3|lowest..highest).
 */
#if !defined(CH32_WSPI_QSPI1_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_WSPI_QSPI1_DMA_PRIORITY        3
#endif

/**
 * @brief   QSPI2 DMA priority (0..3|lowest..highest).
 */
#if !defined(CH32_WSPI_QSPI2_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_WSPI_QSPI2_DMA_PRIORITY        3
#endif

/**
 * @brief   QSPI1 interrupt priority level setting.
 */
#if !defined(CH32_WSPI_QSPI1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_WSPI_QSPI1_IRQ_PRIORITY        10
#endif

/**
 * @brief   QSPI2 interrupt priority level setting.
 */
#if !defined(CH32_WSPI_QSPI2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_WSPI_QSPI2_IRQ_PRIORITY        10
#endif

/**
 * @brief   QSPI1 DMA interrupt priority level setting.
 */
#if !defined(CH32_WSPI_QSPI1_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_WSPI_QSPI1_DMA_IRQ_PRIORITY    10
#endif

/**
 * @brief   QSPI2 DMA interrupt priority level setting.
 */
#if !defined(CH32_WSPI_QSPI2_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_WSPI_QSPI2_DMA_IRQ_PRIORITY    10
#endif

/**
 * @brief   WSPI DMA error hook.
 */
#if !defined(CH32_WSPI_DMA_ERROR_HOOK) || defined(__DOXYGEN__)
#define CH32_WSPI_DMA_ERROR_HOOK(wspip)     osalSysHalt("DMA failure")
#endif

/**
 * @brief   QSPI1 DMA stream ID.
 */
#if !defined(CH32_WSPI_QSPI1_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_WSPI_QSPI1_DMA_STREAM          CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   QSPI2 DMA stream ID.
 */
#if !defined(CH32_WSPI_QSPI2_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_WSPI_QSPI2_DMA_STREAM          CH32_DMA_STREAM_ID_ANY
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !defined(CH32_HAS_QSPI1)
#define CH32_HAS_QSPI1                      FALSE
#endif

#if !defined(CH32_HAS_QSPI2)
#define CH32_HAS_QSPI2                      FALSE
#endif

#if CH32_WSPI_USE_WSPI1 && !CH32_HAS_QSPI1
#error "QSPI1 not present in the selected device"
#endif

#if CH32_WSPI_USE_WSPI2 && !CH32_HAS_QSPI2
#error "QSPI2 not present in the selected device"
#endif

#if !CH32_WSPI_USE_WSPI1 && !CH32_WSPI_USE_WSPI2
#error "WSPI driver activated but no QSPI peripheral assigned"
#endif

#if !defined(CH32_DMA_REQUIRED)
#define CH32_DMA_REQUIRED
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Low level fields of the WSPI driver structure.
 */
#define wspi_lld_driver_fields                                              \
  /* Pointer to the QSPIx registers block.*/                                \
  QSPI_TypeDef              *qspi;                                          \
  /* QSPI DMA stream.*/                                                     \
  const ch32_dma_stream_t   *dma;                                           \
  /* QSPI DMA mode bit mask.*/                                              \
  uint32_t                  dmamode;

/**
 * @brief   Low level fields of the WSPI configuration structure.
 */
#define wspi_lld_config_fields                                              \
  /* Clock prescaler value (0-255).*/                                        \
  uint32_t                  prescaler;                                       \
  /* Clock mode (0=Mode0, 1=Mode3).*/                                       \
  uint32_t                  ckmode;                                          \
  /* Chip select high time (0-7 cycles).*/                                   \
  uint32_t                  cshtime;                                         \
  /* Flash memory size (0-31, 2^(fsize+1) bytes).*/                         \
  uint32_t                  fsize;                                           \
  /* FIFO threshold level (0-15).*/                                          \
  uint32_t                  fifo_threshold;                                  \
  /* Flash select (0=NCS0 primary, 1=NCS1 secondary).*/                     \
  uint32_t                  fselect;                                         \
  /* Dual-flash mode (0=single, 1=dual).*/                                  \
  uint32_t                  dfm;

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (CH32_WSPI_USE_WSPI1 == TRUE) && !defined(__DOXYGEN__)
extern WSPIDriver WSPID1;
#endif

#if (CH32_WSPI_USE_WSPI2 == TRUE) && !defined(__DOXYGEN__)
extern WSPIDriver WSPID2;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void wspi_lld_init(void);
  void wspi_lld_start(WSPIDriver *wspip);
  void wspi_lld_stop(WSPIDriver *wspip);
  void wspi_lld_command(WSPIDriver *wspip, const wspi_command_t *cmdp);
  void wspi_lld_send(WSPIDriver *wspip, const wspi_command_t *cmdp,
                     size_t n, const uint8_t *txbuf);
  void wspi_lld_receive(WSPIDriver *wspip, const wspi_command_t *cmdp,
                        size_t n, uint8_t *rxbuf);
  void wspi_lld_set_dual_flash(WSPIDriver *wspip, bool enable);
  void wspi_lld_select_flash(WSPIDriver *wspip, uint8_t fselect);
#if WSPI_SUPPORTS_MEMMAP == TRUE
  void wspi_lld_map_flash(WSPIDriver *wspip,
                          const wspi_command_t *cmdp,
                          uint8_t **addrp);
  void wspi_lld_unmap_flash(WSPIDriver *wspip);
#endif
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_WSPI */

#endif /* HAL_WSPI_LLD_H */

/** @} */
