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
 * @file    hal_wspi_lld.c
 * @brief   CH32 WSPI subsystem low level driver source (QSPI peripheral).
 *
 * @addtogroup WSPI
 * @{
 */

#include "hal.h"

#if (HAL_USE_WSPI == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/**
 * @brief   QSPI CR register bit definitions.
 */
#define QSPI_CR_EN                          ((uint32_t)0x00000001)
#define QSPI_CR_ABORT                       ((uint32_t)0x00000002)
#define QSPI_CR_DMAEN                       ((uint32_t)0x00000004)
#define QSPI_CR_TCEN                        ((uint32_t)0x00000008)
#define QSPI_CR_START                       ((uint32_t)0x00000020)
#define QSPI_CR_DFM                         ((uint32_t)0x00000040)
#define QSPI_CR_FSEL                        ((uint32_t)0x00000080)
#define QSPI_CR_FTHRES_MASK                 ((uint32_t)0x00001F00)
#define QSPI_CR_FTHRES_SHIFT                8U
#define QSPI_CR_TEIE                        ((uint32_t)0x00010000)
#define QSPI_CR_TCIE                        ((uint32_t)0x00020000)
#define QSPI_CR_FTIE                        ((uint32_t)0x00040000)
#define QSPI_CR_SMIE                        ((uint32_t)0x00080000)
#define QSPI_CR_TOIE                        ((uint32_t)0x00100000)
#define QSPI_CR_APMS                        ((uint32_t)0x00400000)
#define QSPI_CR_PMM                         ((uint32_t)0x00800000)
#define QSPI_CR_PRESCALER_MASK              ((uint32_t)0xFF000000)
#define QSPI_CR_PRESCALER_SHIFT             24U

/**
 * @brief   QSPI SR register bit definitions.
 */
#define QSPI_SR_TEF                         ((uint32_t)0x00000001)
#define QSPI_SR_TCF                         ((uint32_t)0x00000002)
#define QSPI_SR_FTF                         ((uint32_t)0x00000004)
#define QSPI_SR_SMF                         ((uint32_t)0x00000008)
#define QSPI_SR_TOF                         ((uint32_t)0x00000010)
#define QSPI_SR_BUSY                        ((uint32_t)0x00000020)
#define QSPI_SR_FLEVEL_MASK                 ((uint32_t)0x00003F00)
#define QSPI_SR_IDLEF                       ((uint32_t)0x00010000)

/**
 * @brief   QSPI FCR register bit definitions.
 */
#define QSPI_FCR_CTEF                       ((uint32_t)0x00000001)
#define QSPI_FCR_CTCF                       ((uint32_t)0x00000002)
#define QSPI_FCR_CSMF                       ((uint32_t)0x00000008)
#define QSPI_FCR_CTOF                       ((uint32_t)0x00000010)

/**
 * @brief   QSPI DCR register bit definitions.
 */
#define QSPI_DCR_CKMODE                     ((uint32_t)0x00000001)
#define QSPI_DCR_CSHT_MASK                  ((uint32_t)0x00000700)
#define QSPI_DCR_CSHT_SHIFT                 8U
#define QSPI_DCR_FSIZE_MASK                 ((uint32_t)0x001F0000)
#define QSPI_DCR_FSIZE_SHIFT                16U

/**
 * @brief   QSPI CCR register bit definitions.
 */
#define QSPI_CCR_INSTRUCTION_MASK           ((uint32_t)0x000000FF)
#define QSPI_CCR_IMODE_MASK                 ((uint32_t)0x00000300)
#define QSPI_CCR_IMODE_SHIFT                8U
#define QSPI_CCR_ADMODE_MASK                ((uint32_t)0x00000C00)
#define QSPI_CCR_ADMODE_SHIFT               10U
#define QSPI_CCR_ADSIZE_MASK                ((uint32_t)0x00003000)
#define QSPI_CCR_ADSIZE_SHIFT               12U
#define QSPI_CCR_ABMODE_MASK                ((uint32_t)0x0000C000)
#define QSPI_CCR_ABMODE_SHIFT               14U
#define QSPI_CCR_ABSIZE_MASK                ((uint32_t)0x00030000)
#define QSPI_CCR_ABSIZE_SHIFT               16U
#define QSPI_CCR_DCYC_MASK                  ((uint32_t)0x007C0000)
#define QSPI_CCR_DCYC_SHIFT                 17U
#define QSPI_CCR_DMODE_MASK                 ((uint32_t)0x03000000)
#define QSPI_CCR_DMODE_SHIFT                24U
#define QSPI_CCR_FMODE_MASK                 ((uint32_t)0x0C000000)
#define QSPI_CCR_FMODE_SHIFT                26U
#define QSPI_CCR_SIOO                       ((uint32_t)0x10000000)

/**
 * @brief   QSPI functional modes.
 */
#define QSPI_FMODE_WRITE                    ((uint32_t)0x00000000)
#define QSPI_FMODE_READ                     ((uint32_t)0x01000000)
#define QSPI_FMODE_MEMORY_MAPPED            ((uint32_t)0x02000000)
#define QSPI_FMODE_AUTO_POLLING             ((uint32_t)0x03000000)

/**
 * @brief   QSPI line modes.
 */
#define QSPI_MODE_NONE                      ((uint32_t)0x00000000)
#define QSPI_MODE_1_LINE                    ((uint32_t)0x00000001)
#define QSPI_MODE_2_LINES                   ((uint32_t)0x00000002)
#define QSPI_MODE_4_LINES                   ((uint32_t)0x00000003)

/**
 * @brief   QSPI address/alt size values.
 */
#define QSPI_SIZE_8BIT                      ((uint32_t)0x00000000)
#define QSPI_SIZE_16BIT                     ((uint32_t)0x00000001)
#define QSPI_SIZE_24BIT                     ((uint32_t)0x00000002)
#define QSPI_SIZE_32BIT                     ((uint32_t)0x00000003)

/**
 * @brief   QSPI auto-polling match mode.
 */
#define QSPI_PMM_AND                        ((uint32_t)0x00000000)
#define QSPI_PMM_OR                         ((uint32_t)0x00800000)

/**
 * @brief   QSPI timeout value for polling operations.
 */
#define QSPI_TIMEOUT_VALUE                  0x00FFFFFFU

/**
 * @brief   Maps a ChibiOS WSPI line mode to QSPI line mode.
 */
static uint32_t wspi_cfg_to_qspi_mode(uint32_t cfg_field) {
  uint32_t mode = cfg_field & 0x07U;

  switch (mode) {
  case WSPI_CFG_CMD_MODE_NONE:
    return QSPI_MODE_NONE;
  case WSPI_CFG_CMD_MODE_ONE_LINE:
    return QSPI_MODE_1_LINE;
  case WSPI_CFG_CMD_MODE_TWO_LINES:
    return QSPI_MODE_2_LINES;
  case WSPI_CFG_CMD_MODE_FOUR_LINES:
    return QSPI_MODE_4_LINES;
  default:
    return QSPI_MODE_1_LINE;
  }
}

/**
 * @brief   Maps a ChibiOS WSPI address mode to QSPI line mode.
 */
static uint32_t wspi_cfg_to_qspi_addr_mode(uint32_t cfg) {
  uint32_t mode = (cfg >> 8U) & 0x07U;

  switch (mode) {
  case WSPI_CFG_ADDR_MODE_NONE:
    return QSPI_MODE_NONE;
  case WSPI_CFG_ADDR_MODE_ONE_LINE:
    return QSPI_MODE_1_LINE;
  case WSPI_CFG_ADDR_MODE_TWO_LINES:
    return QSPI_MODE_2_LINES;
  case WSPI_CFG_ADDR_MODE_FOUR_LINES:
    return QSPI_MODE_4_LINES;
  default:
    return QSPI_MODE_1_LINE;
  }
}

/**
 * @brief   Maps a ChibiOS WSPI data mode to QSPI line mode.
 */
static uint32_t wspi_cfg_to_qspi_data_mode(uint32_t cfg) {
  uint32_t mode = (cfg >> 24U) & 0x07U;

  switch (mode) {
  case WSPI_CFG_DATA_MODE_NONE:
    return QSPI_MODE_NONE;
  case WSPI_CFG_DATA_MODE_ONE_LINE:
    return QSPI_MODE_1_LINE;
  case WSPI_CFG_DATA_MODE_TWO_LINES:
    return QSPI_MODE_2_LINES;
  case WSPI_CFG_DATA_MODE_FOUR_LINES:
    return QSPI_MODE_4_LINES;
  default:
    return QSPI_MODE_1_LINE;
  }
}

/**
 * @brief   Maps a ChibiOS WSPI alt mode to QSPI line mode.
 */
static uint32_t wspi_cfg_to_qspi_alt_mode(uint32_t cfg) {
  uint32_t mode = (cfg >> 16U) & 0x07U;

  switch (mode) {
  case WSPI_CFG_ALT_MODE_NONE:
    return QSPI_MODE_NONE;
  case WSPI_CFG_ALT_MODE_ONE_LINE:
    return QSPI_MODE_1_LINE;
  case WSPI_CFG_ALT_MODE_TWO_LINES:
    return QSPI_MODE_2_LINES;
  case WSPI_CFG_ALT_MODE_FOUR_LINES:
    return QSPI_MODE_4_LINES;
  default:
    return QSPI_MODE_1_LINE;
  }
}

/**
 * @brief   Maps a ChibiOS WSPI address size to QSPI address size.
 */
static uint32_t wspi_cfg_to_qspi_addr_size(uint32_t cfg) {
  uint32_t size = (cfg >> 12U) & 0x03U;

  switch (size) {
  case WSPI_CFG_ADDR_SIZE_8:
    return QSPI_SIZE_8BIT;
  case WSPI_CFG_ADDR_SIZE_16:
    return QSPI_SIZE_16BIT;
  case WSPI_CFG_ADDR_SIZE_24:
    return QSPI_SIZE_24BIT;
  case WSPI_CFG_ADDR_SIZE_32:
    return QSPI_SIZE_32BIT;
  default:
    return QSPI_SIZE_24BIT;
  }
}

/**
 * @brief   Maps a ChibiOS WSPI alt size to QSPI alt size.
 */
static uint32_t wspi_cfg_to_qspi_alt_size(uint32_t cfg) {
  uint32_t size = (cfg >> 20U) & 0x03U;

  switch (size) {
  case WSPI_CFG_ALT_SIZE_8:
    return QSPI_SIZE_8BIT;
  case WSPI_CFG_ALT_SIZE_16:
    return QSPI_SIZE_16BIT;
  case WSPI_CFG_ALT_SIZE_24:
    return QSPI_SIZE_24BIT;
  case WSPI_CFG_ALT_SIZE_32:
    return QSPI_SIZE_32BIT;
  default:
    return QSPI_SIZE_8BIT;
  }
}

/**
 * @brief   Waits for QSPI idle state.
 *
 * @param[in] qspip     pointer to the QSPI registers block
 */
static void qspi_wait_idle(QSPI_TypeDef *qspip) {
  uint32_t timeout = QSPI_TIMEOUT_VALUE;

  while ((qspip->SR & QSPI_SR_BUSY) != 0U) {
    if (--timeout == 0U) {
      break;
    }
  }
}

/**
 * @brief   Configures and starts a QSPI transfer.
 *
 * @param[in] qspip     pointer to the QSPI registers block
 * @param[in] cmdp      pointer to the command descriptor
 * @param[in] fmode     functional mode (read/write/memory-mapped)
 */
static void qspi_start_transfer(QSPI_TypeDef *qspip,
                                const wspi_command_t *cmdp,
                                uint32_t fmode) {
  uint32_t ccr = 0U;

  /* Instruction phase. */
  ccr |= (cmdp->cmd & QSPI_CCR_INSTRUCTION_MASK);
  ccr |= (wspi_cfg_to_qspi_mode(cmdp->cfg) << QSPI_CCR_IMODE_SHIFT);

  /* Address phase. */
  ccr |= (wspi_cfg_to_qspi_addr_mode(cmdp->cfg) << QSPI_CCR_ADMODE_SHIFT);
  ccr |= (wspi_cfg_to_qspi_addr_size(cmdp->cfg) << QSPI_CCR_ADSIZE_SHIFT);

  /* Alternate bytes phase. */
  ccr |= (wspi_cfg_to_qspi_alt_mode(cmdp->cfg) << QSPI_CCR_ABMODE_SHIFT);
  ccr |= (wspi_cfg_to_qspi_alt_size(cmdp->cfg) << QSPI_CCR_ABSIZE_SHIFT);

  /* Dummy cycles. */
  ccr |= ((cmdp->dummy & 0x1FU) << QSPI_CCR_DCYC_SHIFT);

  /* Data phase. */
  ccr |= (wspi_cfg_to_qspi_data_mode(cmdp->cfg) << QSPI_CCR_DMODE_SHIFT);

  /* Functional mode. */
  ccr |= (fmode << QSPI_CCR_FMODE_SHIFT);

  /* Send-only-once mode. */
  if (cmdp->cfg & WSPI_CFG_SIOO) {
    ccr |= QSPI_CCR_SIOO;
  }

  /* Set address register. */
  qspip->AR = cmdp->addr;

  /* Set alternate bytes register. */
  qspip->ABR = cmdp->alt;

  /* Write CCR register to start the transfer. */
  qspip->CCR = ccr;
}

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

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

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
  WSPID1.qspi = QSPI1;
#endif
#if CH32_WSPI_USE_WSPI2
  wspiObjectInit(&WSPID2);
  WSPID2.qspi = QSPI2;
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
      const WSPIConfig *cfg = wspip->config;

      /* Enable QSPI1 clock. */
      enableHB1(RCC_QSPI1EN);

      /* Reset QSPI1. */
      resetHB1(RCC_QSPI1RST);

      /* Configure QSPI control register:
       * - Prescaler
       * - Flash memory mode (dual or single)
       * - Flash select
       * - FIFO threshold */
      wspip->qspi->CR = ((cfg->prescaler << QSPI_CR_PRESCALER_SHIFT) &
                          QSPI_CR_PRESCALER_MASK) |
                         (cfg->dflash ? QSPI_CR_DFM : 0U) |
                         (cfg->fselect ? QSPI_CR_FSEL : 0U) |
                         ((cfg->fifoThreshold << QSPI_CR_FTHRES_SHIFT) &
                          QSPI_CR_FTHRES_MASK);

      /* Configure QSPI device control register:
       * - Clock mode
       * - Chip select high time
       * - Flash size */
      wspip->qspi->DCR = (cfg->ckmode & QSPI_DCR_CKMODE) |
                          ((cfg->cshTime << QSPI_DCR_CSHT_SHIFT) &
                           QSPI_DCR_CSHT_MASK) |
                          ((cfg->fsize << QSPI_DCR_FSIZE_SHIFT) &
                           QSPI_DCR_FSIZE_MASK);

      /* Enable the QSPI peripheral. */
      wspip->qspi->CR |= QSPI_CR_EN;
    }
#endif
#if CH32_WSPI_USE_WSPI2
    if (&WSPID2 == wspip) {
      const WSPIConfig *cfg = wspip->config;

      /* Enable QSPI2 clock. */
      enableHB1(RCC_QSPI2EN);

      /* Reset QSPI2. */
      resetHB1(RCC_QSPI2RST);

      /* Configure QSPI control register:
       * - Prescaler
       * - Flash memory mode (dual or single)
       * - Flash select
       * - FIFO threshold */
      wspip->qspi->CR = ((cfg->prescaler << QSPI_CR_PRESCALER_SHIFT) &
                          QSPI_CR_PRESCALER_MASK) |
                         (cfg->dflash ? QSPI_CR_DFM : 0U) |
                         (cfg->fselect ? QSPI_CR_FSEL : 0U) |
                         ((cfg->fifoThreshold << QSPI_CR_FTHRES_SHIFT) &
                          QSPI_CR_FTHRES_MASK);

      /* Configure QSPI device control register:
       * - Clock mode
       * - Chip select high time
       * - Flash size */
      wspip->qspi->DCR = (cfg->ckmode & QSPI_DCR_CKMODE) |
                          ((cfg->cshTime << QSPI_DCR_CSHT_SHIFT) &
                           QSPI_DCR_CSHT_MASK) |
                          ((cfg->fsize << QSPI_DCR_FSIZE_SHIFT) &
                           QSPI_DCR_FSIZE_MASK);

      /* Enable the QSPI peripheral. */
      wspip->qspi->CR |= QSPI_CR_EN;
    }
#endif
  }
}

/**
 * @brief   Deactivates the WSPI peripheral.
 *
 * @param[in] wspip     pointer to the @p WSPIDriver object
 *
 * @notapi
 */
void wspi_lld_stop(WSPIDriver *wspip) {

  /* If in ready state then disables WSPI.*/
  if (wspip->state == WSPI_READY) {

#if CH32_WSPI_USE_WSPI1
    if (&WSPID1 == wspip) {

      /* Abort any ongoing transfer. */
      wspip->qspi->CR |= QSPI_CR_ABORT;
      while (wspip->qspi->CR & QSPI_CR_ABORT) {
        /* Wait for abort to complete. */
      }

      /* Disable the QSPI peripheral. */
      wspip->qspi->CR &= ~QSPI_CR_EN;

      /* Disable QSPI1 clock. */
      disableHB1(RCC_QSPI1EN);
    }
#endif
#if CH32_WSPI_USE_WSPI2
    if (&WSPID2 == wspip) {

      /* Abort any ongoing transfer. */
      wspip->qspi->CR |= QSPI_CR_ABORT;
      while (wspip->qspi->CR & QSPI_CR_ABORT) {
        /* Wait for abort to complete. */
      }

      /* Disable the QSPI peripheral. */
      wspip->qspi->CR &= ~QSPI_CR_EN;

      /* Disable QSPI2 clock. */
      disableHB1(RCC_QSPI2EN);
    }
#endif
  }
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
  QSPI_TypeDef *qspip = wspip->qspi;

  /* Wait for idle state. */
  qspi_wait_idle(qspip);

  /* Clear any pending flags. */
  qspip->FCR = QSPI_FCR_CTEF | QSPI_FCR_CTCF | QSPI_FCR_CSMF |
               QSPI_FCR_CTOF;

  /* Set data length to 0 (no data phase). */
  qspip->DLR = 0U;

  /* Start transfer in indirect write mode. */
  qspi_start_transfer(qspip, cmdp, QSPI_FMODE_WRITE);

  /* Wait for transfer complete. */
  while ((qspip->SR & QSPI_SR_TCF) == 0U) {
    /* Spin. */
  }

  /* Clear flags. */
  qspip->FCR = QSPI_FCR_CTEF | QSPI_FCR_CTCF | QSPI_FCR_CSMF |
               QSPI_FCR_CTOF;
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
  QSPI_TypeDef *qspip = wspip->qspi;
  size_t i;

  /* Wait for idle state. */
  qspi_wait_idle(qspip);

  /* Clear any pending flags. */
  qspip->FCR = QSPI_FCR_CTEF | QSPI_FCR_CTCF | QSPI_FCR_CSMF |
               QSPI_FCR_CTOF;

  /* Set data length. */
  qspip->DLR = (uint32_t)n;

  /* Start transfer in indirect write mode. */
  qspi_start_transfer(qspip, cmdp, QSPI_FMODE_WRITE);

  /* Send data byte by byte through FIFO. */
  for (i = 0U; i < n; i++) {
    /* Wait for FIFO threshold. */
    while ((qspip->SR & QSPI_SR_FTF) == 0U) {
      /* Spin. */
    }
    *((volatile uint8_t *)&qspip->DR) = txbuf[i];
  }

  /* Wait for transfer complete. */
  while ((qspip->SR & QSPI_SR_TCF) == 0U) {
    /* Spin. */
  }

  /* Clear flags. */
  qspip->FCR = QSPI_FCR_CTEF | QSPI_FCR_CTCF | QSPI_FCR_CSMF |
               QSPI_FCR_CTOF;
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
  QSPI_TypeDef *qspip = wspip->qspi;
  size_t i;

  /* Wait for idle state. */
  qspi_wait_idle(qspip);

  /* Clear any pending flags. */
  qspip->FCR = QSPI_FCR_CTEF | QSPI_FCR_CTCF | QSPI_FCR_CSMF |
               QSPI_FCR_CTOF;

  /* Set data length. */
  qspip->DLR = (uint32_t)n;

  /* Start transfer in indirect read mode. */
  qspi_start_transfer(qspip, cmdp, QSPI_FMODE_READ);

  /* Receive data byte by byte from FIFO. */
  for (i = 0U; i < n; i++) {
    /* Wait for FIFO threshold or data available. */
    while ((qspip->SR & QSPI_SR_FTF) == 0U) {
      if ((qspip->SR & QSPI_SR_TCF) != 0U) {
        /* Transfer complete, drain remaining FIFO. */
        break;
      }
    }
    rxbuf[i] = *((volatile const uint8_t *)&qspip->DR);
  }

  /* Wait for transfer complete. */
  while ((qspip->SR & QSPI_SR_TCF) == 0U) {
    /* Spin. */
  }

  /* Clear flags. */
  qspip->FCR = QSPI_FCR_CTEF | QSPI_FCR_CTCF | QSPI_FCR_CSMF |
               QSPI_FCR_CTOF;
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
  QSPI_TypeDef *qspip = wspip->qspi;

  /* Wait for idle state. */
  qspi_wait_idle(qspip);

  /* Clear any pending flags. */
  qspip->FCR = QSPI_FCR_CTEF | QSPI_FCR_CTCF | QSPI_FCR_CSMF |
               QSPI_FCR_CTOF;

  /* Start transfer in memory-mapped mode. */
  qspi_start_transfer(qspip, cmdp, QSPI_FMODE_MEMORY_MAPPED);

  /* Return the memory-mapped base address for the active QSPI bank. */
  if (addrp != NULL) {
#if CH32_WSPI_USE_WSPI1
    if (&WSPID1 == wspip) {
      *addrp = (uint8_t *)WSPI1_MEMMAP_BASE;
    }
    else
#endif
#if CH32_WSPI_USE_WSPI2
    if (&WSPID2 == wspip) {
      *addrp = (uint8_t *)WSPI2_MEMMAP_BASE;
    }
    else
#endif
    {
      *addrp = (uint8_t *)WSPI_MEMMAP_BASE;
    }
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
  QSPI_TypeDef *qspip = wspip->qspi;

  /* Abort the memory-mapped mode. */
  qspip->CR |= QSPI_CR_ABORT;
  while (qspip->CR & QSPI_CR_ABORT) {
    /* Wait for abort to complete. */
  }

  /* Clear flags. */
  qspip->FCR = QSPI_FCR_CTEF | QSPI_FCR_CTCF | QSPI_FCR_CSMF |
               QSPI_FCR_CTOF;
}
#endif /* WSPI_SUPPORTS_MEMMAP == TRUE */

#endif /* HAL_USE_WSPI */

/** @} */
