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
 * @file    hal_wspi_lld.h
 * @brief   CH32 WSPI subsystem low level driver header (QSPI peripheral).
 *
 * @addtogroup WSPI
 * @{
 */

#ifndef HAL_WSPI_LLD_H
#define HAL_WSPI_LLD_H

#if (HAL_USE_WSPI == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    WSPI implementation capabilities
 * @{
 */
#define WSPI_SUPPORTS_MEMMAP                TRUE
#define WSPI_DEFAULT_CFG_MASKS              TRUE
/** @} */

/**
 * @name    CH32H417 QSPI memory-mapped base addresses
 * @{
 */
#define WSPI_MEMMAP_BASE                    0x90000000U
#define WSPI1_MEMMAP_BASE                   0x90000000U
#define WSPI2_MEMMAP_BASE                   0x98000000U
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
 * @details If set to @p TRUE the support for WSPID1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_WSPI_USE_WSPI1) || defined(__DOXYGEN__)
#define CH32_WSPI_USE_WSPI1             FALSE
#endif

/**
 * @brief   WSPID2 driver enable switch.
 * @details If set to @p TRUE the support for WSPID2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_WSPI_USE_WSPI2) || defined(__DOXYGEN__)
#define CH32_WSPI_USE_WSPI2             FALSE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

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
  /* Pointer to the QSPI registers block.*/                                 \
  QSPI_TypeDef                *qspi;

/**
 * @brief   Low level fields of the WSPI configuration structure.
 */
#define wspi_lld_config_fields                                              \
  /* QSPI prescaler value.*/                                                \
  uint32_t                    prescaler;                                    \
  /* QSPI clock mode (0 or 3).*/                                            \
  uint32_t                    ckmode;                                       \
  /* QSPI chip select high time (cycles).*/                                 \
  uint32_t                    cshTime;                                      \
  /* QSPI flash size (2^(FSIZE+1) bytes).*/                                 \
  uint32_t                    fsize;                                        \
  /* QSPI flash select (0 or 1).*/                                          \
  uint32_t                    fselect;                                      \
  /* QSPI dual flash enable.*/                                              \
  uint32_t                    dflash;                                       \
  /* QSPI FIFO threshold.*/                                                 \
  uint32_t                    fifoThreshold;

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
