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
 * @file    SDIOv1/hal_sdc_lld.c
 * @brief   CH32 SDC subsystem low level driver source (SDIO peripheral).
 *
 * @addtogroup SDC
 * @{
 */

#include <string.h>

#include "hal.h"

#if HAL_USE_SDC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/**
 * @brief   SDIO CLKCR register bit definitions.
 */
#define SDIO_CLKCR_CLKDIV_MASK              ((uint32_t)0x000000FF)
#define SDIO_CLKCR_PWRSAV                   ((uint32_t)0x00000400)
#define SDIO_CLKCR_BYPASS                   ((uint32_t)0x00000200)
#define SDIO_CLKCR_WIDE_4BIT                ((uint32_t)0x00001800)
#define SDIO_CLKCR_WIDE_1BIT                ((uint32_t)0x00000000)
#define SDIO_CLKCR_WIDE_4BIT_EN             ((uint32_t)0x00001000)
#define SDIO_CLKCR_HWFC_EN                  ((uint32_t)0x00000400)

/**
 * @brief   SDIO CMD register bit definitions.
 */
#define SDIO_CMD_CMDINDEX_MASK              ((uint32_t)0x0000003F)
#define SDIO_CMD_WAITRESP_MASK              ((uint32_t)0x000000C0)
#define SDIO_CMD_WAITRESP_SHORT             ((uint32_t)0x00000040)
#define SDIO_CMD_WAITRESP_LONG              ((uint32_t)0x000000C0)
#define SDIO_CMD_WAITINT                    ((uint32_t)0x00000100)
#define SDIO_CMD_WAITPEND                   ((uint32_t)0x00000200)
#define SDIO_CMD_CPSMEN                     ((uint32_t)0x00000400)

/**
 * @brief   SDIO DCTRL register bit definitions.
 */
#define SDIO_DCTRL_DBLOCKSIZE_MASK          ((uint32_t)0x0000000F)
#define SDIO_DCTRL_DTMODE                   ((uint32_t)0x00000010)
#define SDIO_DCTRL_DTDIR                    ((uint32_t)0x00000020)
#define SDIO_DCTRL_DTEN                     ((uint32_t)0x00000040)
#define SDIO_DCTRL_DMAEN                    ((uint32_t)0x00000080)
#define SDIO_DCTRL_IRQSTOP                  ((uint32_t)0x00000100)
#define SDIO_DCTRL_IRQSDIOEN                ((uint32_t)0x00000200)

/**
 * @brief   SDIO POWER register bit definitions.
 */
#define SDIO_POWER_PWRCTRL_MASK             ((uint32_t)0x00000003)
#define SDIO_POWER_PWRCTRL_ON               ((uint32_t)0x00000003)
#define SDIO_POWER_PWRCTRL_OFF              ((uint32_t)0x00000000)

/**
 * @brief   SDIO STA (status) register bit definitions.
 */
#define SDIO_STA_CCRCFAIL                   ((uint32_t)0x00000001)
#define SDIO_STA_DCRCFAIL                   ((uint32_t)0x00000002)
#define SDIO_STA_CTIMEOUT                   ((uint32_t)0x00000004)
#define SDIO_STA_DTIMEOUT                   ((uint32_t)0x00000008)
#define SDIO_STA_TXUNDERR                   ((uint32_t)0x00000010)
#define SDIO_STA_RXOVERR                    ((uint32_t)0x00000020)
#define SDIO_STA_CMDREND                    ((uint32_t)0x00000040)
#define SDIO_STA_CMDSENT                    ((uint32_t)0x00000080)
#define SDIO_STA_DATAEND                    ((uint32_t)0x00000100)
#define SDIO_STA_STBITERR                   ((uint32_t)0x00000200)
#define SDIO_STA_DBCKEND                    ((uint32_t)0x00000400)
#define SDIO_STA_CMDACT                     ((uint32_t)0x00000800)
#define SDIO_STA_TXACT                      ((uint32_t)0x00001000)
#define SDIO_STA_RXACT                      ((uint32_t)0x00002000)
#define SDIO_STA_TXFIFOHF                   ((uint32_t)0x00004000)
#define SDIO_STA_RXFIFOHF                   ((uint32_t)0x00008000)
#define SDIO_STA_TXFIFOF                    ((uint32_t)0x00010000)
#define SDIO_STA_RXFIFOF                    ((uint32_t)0x00020000)
#define SDIO_STA_TXFIFOE                    ((uint32_t)0x00040000)
#define SDIO_STA_RXFIFOE                    ((uint32_t)0x00080000)
#define SDIO_STA_TXDAVL                     ((uint32_t)0x00100000)
#define SDIO_STA_RXDAVL                     ((uint32_t)0x00200000)
#define SDIO_STA_SDIOIT                     ((uint32_t)0x00400000)
#define SDIO_STA_CEATAEND                   ((uint32_t)0x00800000)

/**
 * @brief   Mask of all static status flags.
 */
#define SDIO_STA_STATIC_FLAGS               (SDIO_STA_CCRCFAIL |      \
                                             SDIO_STA_DCRCFAIL |      \
                                             SDIO_STA_CTIMEOUT |      \
                                             SDIO_STA_DTIMEOUT |      \
                                             SDIO_STA_TXUNDERR |      \
                                             SDIO_STA_RXOVERR  |      \
                                             SDIO_STA_CMDREND  |      \
                                             SDIO_STA_CMDSENT  |      \
                                             SDIO_STA_DATAEND  |      \
                                             SDIO_STA_STBITERR |      \
                                             SDIO_STA_DBCKEND)

/**
 * @brief   Mask of command response error flags.
 */
#define SDIO_STA_CMD_ERROR_MASK             (SDIO_STA_CCRCFAIL |      \
                                             SDIO_STA_CTIMEOUT)

/**
 * @brief   Mask of data transfer error flags.
 */
#define SDIO_STA_DATA_ERROR_MASK            (SDIO_STA_DCRCFAIL |      \
                                             SDIO_STA_DTIMEOUT |      \
                                             SDIO_STA_TXUNDERR |      \
                                             SDIO_STA_RXOVERR  |      \
                                             SDIO_STA_STBITERR)

/**
 * @brief   SDIO FIFO half-full threshold (8 words = 32 bytes).
 */
#define SDIO_HALFFIFO                       8U

/**
 * @brief   SDIO FIFO half-full size in bytes.
 */
#define SDIO_HALFFIFOBYTES                  (SDIO_HALFFIFO * 4U)

/**
 * @brief   Maximum data timeout value.
 */
#define SDIO_MAX_DATA_TIMEOUT               0x00FFFFFFU

/**
 * @brief   SDIO command timeout (iteration count).
 */
#define SDIO_CMD_TIMEOUT                    0x00010000U

/**
 * @brief   Calculates the CLKCR divider value for a target frequency.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] f         desired frequency in Hz
 * @return              divider value for CLKCR register
 */
static uint32_t sdc_lld_clkdiv(SDCDriver *sdcp, uint32_t f) {

  if (f > 400000U) {
    return sdcp->clkfreq / f;
  }
  else {
    return sdcp->clkfreq / 64U / f + 1U;
  }
}

/**
 * @brief   Waits for command completion and checks for errors.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] timeout   busy-wait timeout in iterations
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 */
static bool sdc_lld_wait_cmd_done(SDCDriver *sdcp, uint32_t timeout) {
  uint32_t sta;

  do {
    sta = sdcp->sdio->STA;

    /* Check for command received (short/long response). */
    if (sta & SDIO_STA_CMDREND) {
      sdcp->sdio->ICR = SDIO_STA_CMDREND;
      return HAL_SUCCESS;
    }

    /* Check for command sent (no response). */
    if (sta & SDIO_STA_CMDSENT) {
      sdcp->sdio->ICR = SDIO_STA_CMDSENT;
      return HAL_SUCCESS;
    }

    /* Check for errors. */
    if (sta & SDIO_STA_CMD_ERROR_MASK) {
      if (sta & SDIO_STA_CTIMEOUT) {
        sdcp->errors |= SDC_COMMAND_TIMEOUT;
        sdcp->sdio->ICR = SDIO_STA_CTIMEOUT;
      }
      if (sta & SDIO_STA_CCRCFAIL) {
        sdcp->errors |= SDC_CMD_CRC_ERROR;
        sdcp->sdio->ICR = SDIO_STA_CCRCFAIL;
      }
      return HAL_FAILED;
    }

    if (--timeout == 0U) {
      sdcp->errors |= SDC_COMMAND_TIMEOUT;
      sdcp->sdio->ICR = SDIO_STA_STATIC_FLAGS;
      return HAL_FAILED;
    }

  } while (true);
}

/**
 * @brief   Waits for data transfer completion and checks for errors.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] timeout   busy-wait timeout in iterations
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 */
static bool sdc_lld_wait_data_done(SDCDriver *sdcp, uint32_t timeout) {
  uint32_t sta;

  do {
    sta = sdcp->sdio->STA;

    /* Check for transfer done. */
    if (sta & SDIO_STA_DATAEND) {
      sdcp->sdio->ICR = SDIO_STA_DATAEND;
      return HAL_SUCCESS;
    }

    /* Check for block end. */
    if (sta & SDIO_STA_DBCKEND) {
      sdcp->sdio->ICR = SDIO_STA_DBCKEND;
      return HAL_SUCCESS;
    }

    /* Check for errors. */
    if (sta & SDIO_STA_DATA_ERROR_MASK) {
      if (sta & SDIO_STA_DTIMEOUT) {
        sdcp->errors |= SDC_DATA_TIMEOUT;
        sdcp->sdio->ICR = SDIO_STA_DTIMEOUT;
      }
      if (sta & SDIO_STA_DCRCFAIL) {
        sdcp->errors |= SDC_DATA_CRC_ERROR;
        sdcp->sdio->ICR = SDIO_STA_DCRCFAIL;
      }
      if (sta & SDIO_STA_RXOVERR) {
        sdcp->errors |= SDC_RX_OVERRUN;
        sdcp->sdio->ICR = SDIO_STA_RXOVERR;
      }
      if (sta & SDIO_STA_TXUNDERR) {
        sdcp->errors |= SDC_TX_UNDERRUN;
        sdcp->sdio->ICR = SDIO_STA_TXUNDERR;
      }
      if (sta & SDIO_STA_STBITERR) {
        sdcp->errors |= SDC_STARTBIT_ERROR;
        sdcp->sdio->ICR = SDIO_STA_STBITERR;
      }
      return HAL_FAILED;
    }

    if (--timeout == 0U) {
      sdcp->errors |= SDC_DATA_TIMEOUT;
      sdcp->sdio->ICR = SDIO_STA_STATIC_FLAGS;
      return HAL_FAILED;
    }

  } while (true);
}

/**
 * @brief   Reads one or more blocks (aligned buffer) using FIFO polling.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] startblk  first block to read
 * @param[out] buf      pointer to the read buffer (must be 4-byte aligned)
 * @param[in] blocks    number of blocks to read
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 */
static bool sdc_lld_read_aligned(SDCDriver *sdcp, uint32_t startblk,
                                 uint8_t *buf, uint32_t blocks) {
  uint32_t resp[1];
  uint32_t blksize = MMCSD_BLOCK_SIZE;
  uint32_t *tempbuf = (uint32_t *)buf;
  uint32_t count;

  osalDbgCheck(blocks < 0x1000000U / MMCSD_BLOCK_SIZE);

  /* Convert to byte address for standard capacity cards. */
  if (!(sdcp->cardmode & SDC_MODE_HIGH_CAPACITY)) {
    startblk *= MMCSD_BLOCK_SIZE;
  }

  /* Clear any pending flags. */
  sdcp->sdio->ICR = SDIO_STA_STATIC_FLAGS;

  /* Reset data control register. */
  sdcp->sdio->DCTRL = 0U;

  /* Set data timeout. */
  sdcp->sdio->DTIMER = SDIO_MAX_DATA_TIMEOUT;

  /* Set data length. */
  sdcp->sdio->DLEN = (uint32_t)blksize * blocks;

  /* Configure data control:
     * DBLOCKSIZE = 9 (512 bytes)
     * DTDIR = 1 (card to SDIO)
     * DTMODE = 0 (block mode)
     * DTEN = 1 (data transfer enabled) */
  sdcp->sdio->DCTRL = (9U << 0) |    /* DBLOCKSIZE = 512 bytes */
                       SDIO_DCTRL_DTDIR |
                       SDIO_DCTRL_DTEN;

  /* Send read command. */
  if (blocks > 1U) {
    if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_READ_MULTIPLE_BLOCK,
                                   startblk, resp) ||
        MMCSD_R1_ERROR(resp[0])) {
      sdcp->sdio->DCTRL = 0U;
      return HAL_FAILED;
    }
  }
  else {
    if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_READ_SINGLE_BLOCK,
                                   startblk, resp) ||
        MMCSD_R1_ERROR(resp[0])) {
      sdcp->sdio->DCTRL = 0U;
      return HAL_FAILED;
    }
  }

  /* Wait for data transfer to complete using FIFO polling. */
  {
    uint32_t total_words = (blksize * blocks) / 4;
    uint32_t words_read = 0;
    uint32_t timeout = SDIO_MAX_DATA_TIMEOUT;

    while (words_read < total_words) {
      uint32_t sta = sdcp->sdio->STA;

      /* Check for errors. */
      if (sta & SDIO_STA_DATA_ERROR_MASK) {
        if (sta & SDIO_STA_DTIMEOUT) {
          sdcp->errors |= SDC_DATA_TIMEOUT;
          sdcp->sdio->ICR = SDIO_STA_DTIMEOUT;
        }
        if (sta & SDIO_STA_DCRCFAIL) {
          sdcp->errors |= SDC_DATA_CRC_ERROR;
          sdcp->sdio->ICR = SDIO_STA_DCRCFAIL;
        }
        if (sta & SDIO_STA_RXOVERR) {
          sdcp->errors |= SDC_RX_OVERRUN;
          sdcp->sdio->ICR = SDIO_STA_RXOVERR;
        }
        if (sta & SDIO_STA_STBITERR) {
          sdcp->errors |= SDC_STARTBIT_ERROR;
          sdcp->sdio->ICR = SDIO_STA_STBITERR;
        }
        sdcp->sdio->DCTRL = 0U;
        return HAL_FAILED;
      }

      /* Check for data end. */
      if (sta & SDIO_STA_DATAEND) {
        sdcp->sdio->ICR = SDIO_STA_DATAEND;
        break;
      }

      /* Read from FIFO when half-full. */
      if (sta & SDIO_STA_RXFIFOHF) {
        for (count = 0; count < SDIO_HALFFIFO; count++) {
          *tempbuf++ = sdcp->sdio->FIFO;
        }
        words_read += SDIO_HALFFIFO;
        timeout = SDIO_MAX_DATA_TIMEOUT;
      }
      else {
        if (--timeout == 0U) {
          sdcp->errors |= SDC_DATA_TIMEOUT;
          sdcp->sdio->DCTRL = 0U;
          return HAL_FAILED;
        }
      }
    }

    /* Drain any remaining data in FIFO. */
    while (sdcp->sdio->STA & SDIO_STA_RXDAVL) {
      *tempbuf++ = sdcp->sdio->FIFO;
    }
  }

  /* Clear status flags. */
  sdcp->sdio->ICR = SDIO_STA_STATIC_FLAGS;

  /* Disable data transfer. */
  sdcp->sdio->DCTRL = 0U;

  /* For multi-block reads, send stop command. */
  if (blocks > 1U) {
    return sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_STOP_TRANSMISSION, 0,
                                      resp);
  }

  return HAL_SUCCESS;
}

/**
 * @brief   Writes one or more blocks (aligned buffer) using FIFO polling.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] startblk  first block to write
 * @param[in] buf       pointer to the write buffer (must be 4-byte aligned)
 * @param[in] blocks    number of blocks to write
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 */
static bool sdc_lld_write_aligned(SDCDriver *sdcp, uint32_t startblk,
                                  const uint8_t *buf, uint32_t blocks) {
  uint32_t resp[1];
  uint32_t blksize = MMCSD_BLOCK_SIZE;
  const uint32_t *tempbuf = (const uint32_t *)buf;
  uint32_t count;

  /* Convert to byte address for standard capacity cards. */
  if (!(sdcp->cardmode & SDC_MODE_HIGH_CAPACITY)) {
    startblk *= MMCSD_BLOCK_SIZE;
  }

  /* Clear any pending flags. */
  sdcp->sdio->ICR = SDIO_STA_STATIC_FLAGS;

  /* Reset data control register. */
  sdcp->sdio->DCTRL = 0U;

  /* Set data timeout. */
  sdcp->sdio->DTIMER = SDIO_MAX_DATA_TIMEOUT;

  /* Set data length. */
  sdcp->sdio->DLEN = (uint32_t)blksize * blocks;

  /* Send write command. */
  if (blocks > 1U) {
    if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_WRITE_MULTIPLE_BLOCK,
                                   startblk, resp) ||
        MMCSD_R1_ERROR(resp[0])) {
      sdcp->sdio->DCTRL = 0U;
      return HAL_FAILED;
    }
  }
  else {
    if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_WRITE_BLOCK,
                                   startblk, resp) ||
        MMCSD_R1_ERROR(resp[0])) {
      sdcp->sdio->DCTRL = 0U;
      return HAL_FAILED;
    }
  }

  /* Configure data control:
     * DBLOCKSIZE = 9 (512 bytes)
     * DTDIR = 0 (SDIO to card)
     * DTMODE = 0 (block mode)
     * DTEN = 1 (data transfer enabled) */
  sdcp->sdio->DCTRL = (9U << 0) |    /* DBLOCKSIZE = 512 bytes */
                       SDIO_DCTRL_DTEN;

  /* Wait for data transfer to complete using FIFO polling. */
  {
    uint32_t total_words = (blksize * blocks) / 4;
    uint32_t words_written = 0;
    uint32_t timeout = SDIO_MAX_DATA_TIMEOUT;

    while (words_written < total_words) {
      uint32_t sta = sdcp->sdio->STA;

      /* Check for errors. */
      if (sta & SDIO_STA_DATA_ERROR_MASK) {
        if (sta & SDIO_STA_DTIMEOUT) {
          sdcp->errors |= SDC_DATA_TIMEOUT;
          sdcp->sdio->ICR = SDIO_STA_DTIMEOUT;
        }
        if (sta & SDIO_STA_DCRCFAIL) {
          sdcp->errors |= SDC_DATA_CRC_ERROR;
          sdcp->sdio->ICR = SDIO_STA_DCRCFAIL;
        }
        if (sta & SDIO_STA_TXUNDERR) {
          sdcp->errors |= SDC_TX_UNDERRUN;
          sdcp->sdio->ICR = SDIO_STA_TXUNDERR;
        }
        if (sta & SDIO_STA_STBITERR) {
          sdcp->errors |= SDC_STARTBIT_ERROR;
          sdcp->sdio->ICR = SDIO_STA_STBITERR;
        }
        sdcp->sdio->DCTRL = 0U;
        return HAL_FAILED;
      }

      /* Check for data end. */
      if (sta & SDIO_STA_DATAEND) {
        sdcp->sdio->ICR = SDIO_STA_DATAEND;
        break;
      }

      /* Write to FIFO when half-empty. */
      if (sta & SDIO_STA_TXFIFOHF) {
        uint32_t remaining = total_words - words_written;
        uint32_t to_write = (remaining < SDIO_HALFFIFO) ? remaining
                                                         : SDIO_HALFFIFO;
        for (count = 0; count < to_write; count++) {
          sdcp->sdio->FIFO = *tempbuf++;
        }
        words_written += to_write;
        timeout = SDIO_MAX_DATA_TIMEOUT;
      }
      else {
        if (--timeout == 0U) {
          sdcp->errors |= SDC_DATA_TIMEOUT;
          sdcp->sdio->DCTRL = 0U;
          return HAL_FAILED;
        }
      }
    }
  }

  /* Clear status flags. */
  sdcp->sdio->ICR = SDIO_STA_STATIC_FLAGS;

  /* Disable data transfer. */
  sdcp->sdio->DCTRL = 0U;

  /* For multi-block writes, send stop command. */
  if (blocks > 1U) {
    return sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_STOP_TRANSMISSION, 0,
                                      resp);
  }

  return HAL_SUCCESS;
}

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   SDCD1 driver identifier.
 */
#if CH32_SDC_USE_SDC1 || defined(__DOXYGEN__)
SDCDriver SDCD1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Default SDC configuration.
 */
static const SDCConfig sdc_default_cfg = {
  SDC_MODE_4BIT
};

#if CH32_SDC_USE_SDC1 || defined(__DOXYGEN__)
static uint8_t __attribute__((aligned(16))) sd1_bounce_buf[MMCSD_BLOCK_SIZE];
static uint32_t __attribute__((aligned(4))) sd1_resp_buf[1];
#endif

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   SDIO interrupt handler.
 * @details Wakes the transaction thread when transfer is done or error.
 *
 * @isr
 */
#if CH32_SDC_USE_SDC1 || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(CH32_SDIO_HANDLER) {

  osalSysLockFromISR();

  /* Disable all SDIO interrupts. */
  SDCD1.sdio->MASK = 0U;

  osalThreadResumeI(&SDCD1.thread, MSG_OK);

  osalSysUnlockFromISR();
}
#endif

/**
 * @brief   Low level SDC driver initialization.
 *
 * @notapi
 */
void sdc_lld_init(void) {

#if CH32_SDC_USE_SDC1
  sdcObjectInit(&SDCD1);
  SDCD1.thread  = NULL;
  SDCD1.sdio    = SDIO;
  SDCD1.clkfreq = CH32_SDC_SDIO_CLOCK;
  SDCD1.buf     = sd1_bounce_buf;
  SDCD1.resp    = sd1_resp_buf;
#endif
}

/**
 * @brief   Configures and activates the SDC peripheral.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @notapi
 */
void sdc_lld_start(SDCDriver *sdcp) {

  /* Check configuration, use default if NULL. */
  if (sdcp->config == NULL) {
    sdcp->config = &sdc_default_cfg;
  }

  /* If in stopped state, enable peripheral clock. */
  if (sdcp->state == BLK_STOP) {

#if CH32_SDC_USE_SDC1
    if (&SDCD1 == sdcp) {
      /* Enable SDIO clock and reset. */
      resetHB2(RCC_SDIOEN);
      enableHB2(RCC_SDIOEN);
    }
#endif
  }

  /* Power on the SDIO card interface. */
  sdcp->sdio->POWER = SDIO_POWER_PWRCTRL_ON;

  /* Clear all interrupt flags. */
  sdcp->sdio->ICR = SDIO_STA_STATIC_FLAGS;

  /* Disable all interrupts. */
  sdcp->sdio->MASK = 0U;

  NVIC_SetPriority(SDIO_IRQn, CH32_SDC_SDIO_IRQ_PRIORITY);
  NVIC_EnableIRQ(SDIO_IRQn);

  /* Set default clock configuration:
     * CLKDIV = 0 (maximum speed)
     * BYPASS = 0
     * HWFC_EN = 0
     * WIDE_4BIT = 0 (1-bit mode) */
  sdcp->sdio->CLKCR = 0U;

  /* Set default data timeout. */
  sdcp->sdio->DTIMER = 0U;

  /* Clear data length and control. */
  sdcp->sdio->DLEN = 0U;
  sdcp->sdio->DCTRL = 0U;
}

/**
 * @brief   Deactivates the SDC peripheral.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @notapi
 */
void sdc_lld_stop(SDCDriver *sdcp) {

  if (sdcp->state != BLK_STOP) {

    /* Disable data transfer. */
    sdcp->sdio->DCTRL = 0U;

    /* Disable interrupts. */
    sdcp->sdio->MASK = 0U;
    sdcp->sdio->ICR = SDIO_STA_STATIC_FLAGS;

    /* Stop clock. */
    sdcp->sdio->CLKCR = 0U;

    /* Power off. */
    sdcp->sdio->POWER = SDIO_POWER_PWRCTRL_OFF;

    /* Disable peripheral clock. */
#if CH32_SDC_USE_SDC1
    if (&SDCD1 == sdcp) {
      disableHB2(RCC_SDIOEN);
    }
#endif
  }
}

/**
 * @brief   Starts the SDIO clock and sets it to init mode (400kHz or less).
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @notapi
 */
void sdc_lld_start_clk(SDCDriver *sdcp) {
  uint32_t div;

  /* Calculate divider for ~400kHz init clock. */
  div = sdc_lld_clkdiv(sdcp, 400000U);

  /* Set clock divider and enable clock output. */
  sdcp->sdio->CLKCR = (div & SDIO_CLKCR_CLKDIV_MASK);

  /* Wait for clock to stabilize. */
  osalThreadSleep(OSAL_MS2I(CH32_SDC_CLOCK_DELAY));
}

/**
 * @brief   Sets the SDIO clock to data mode.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] clk       the clock mode
 *
 * @notapi
 */
void sdc_lld_set_data_clk(SDCDriver *sdcp, sdcbusclk_t clk) {
  uint32_t div;

  if (SDC_CLK_50MHz == clk) {
    div = sdc_lld_clkdiv(sdcp, 50000000U);
  }
  else {
    div = sdc_lld_clkdiv(sdcp, 25000000U);
  }

  /* Preserve current bus width setting. */
  uint32_t clkcr = sdcp->sdio->CLKCR & SDIO_CLKCR_WIDE_4BIT;
  clkcr |= (div & SDIO_CLKCR_CLKDIV_MASK);
  sdcp->sdio->CLKCR = clkcr;
}

/**
 * @brief   Stops the SDIO clock.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @notapi
 */
void sdc_lld_stop_clk(SDCDriver *sdcp) {

  /* Set clock divider to max (stopped), keep bus width bits. */
  sdcp->sdio->CLKCR = sdcp->sdio->CLKCR & SDIO_CLKCR_WIDE_4BIT;
}

/**
 * @brief   Switches the bus to 1, 4 or 8 bits mode.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] mode      bus mode
 *
 * @notapi
 */
void sdc_lld_set_bus_mode(SDCDriver *sdcp, sdcbusmode_t mode) {
  uint32_t clkcr;

  clkcr = sdcp->sdio->CLKCR & ~SDIO_CLKCR_WIDE_4BIT;

  switch (mode) {
  case SDC_MODE_1BIT:
    /* Bus width 1 bit: WIDE_4BIT = 0. */
    sdcp->sdio->CLKCR = clkcr;
    break;
  case SDC_MODE_4BIT:
    /* Bus width 4 bits: WIDE_4BIT = 1. */
    sdcp->sdio->CLKCR = clkcr | SDIO_CLKCR_WIDE_4BIT_EN;
    break;
  case SDC_MODE_8BIT:
    /* SDIO does not support 8-bit mode, fall back to 4-bit. */
    sdcp->sdio->CLKCR = clkcr | SDIO_CLKCR_WIDE_4BIT_EN;
    break;
  default:
    osalDbgAssert(false, "invalid bus mode");
    break;
  }
}

/**
 * @brief   Sends an SDIO command with no response expected.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] cmd       card command
 * @param[in] arg       command argument
 *
 * @notapi
 */
void sdc_lld_send_cmd_none(SDCDriver *sdcp, uint8_t cmd, uint32_t arg) {

  /* Clear residual flags before issuing new command. */
  sdcp->sdio->ICR = SDIO_STA_STATIC_FLAGS;

  /* Set argument. */
  sdcp->sdio->ARG = arg;

  /* Send command (no response, CPSM enabled). */
  sdcp->sdio->CMD = ((uint32_t)(cmd & 0x3F) |
                     SDIO_CMD_CPSMEN);

  /* Wait for command done (sent). */
  {
    uint32_t timeout = SDIO_CMD_TIMEOUT;
    while ((--timeout > 0U) &&
           ((sdcp->sdio->STA & SDIO_STA_CMDSENT) == 0U)) {
      /* Spin. */
    }
  }

  /* Clear flags. */
  sdcp->sdio->ICR = SDIO_STA_STATIC_FLAGS;
}

/**
 * @brief   Sends an SDIO command with a short response expected.
 * @note    The CRC is not verified.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] cmd       card command
 * @param[in] arg       command argument
 * @param[out] resp     pointer to the response buffer (one word)
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 *
 * @notapi
 */
bool sdc_lld_send_cmd_short(SDCDriver *sdcp, uint8_t cmd, uint32_t arg,
                            uint32_t *resp) {

  /* Clear residual flags before issuing new command. */
  sdcp->sdio->ICR = SDIO_STA_STATIC_FLAGS;

  /* Set argument. */
  sdcp->sdio->ARG = arg;

  /* Send command with short response expected, CPSM enabled. */
  sdcp->sdio->CMD = ((uint32_t)(cmd & 0x3F) |
                     SDIO_CMD_WAITRESP_SHORT |
                     SDIO_CMD_CPSMEN);

  /* Wait for response. */
  if (sdc_lld_wait_cmd_done(sdcp, SDIO_CMD_TIMEOUT)) {
    return HAL_FAILED;
  }

  /* Read short response from RESP1. */
  *resp = sdcp->sdio->RESP1;

  return HAL_SUCCESS;
}

/**
 * @brief   Sends an SDIO command with a short response expected and CRC.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] cmd       card command
 * @param[in] arg       command argument
 * @param[out] resp     pointer to the response buffer (one word)
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 *
 * @notapi
 */
bool sdc_lld_send_cmd_short_crc(SDCDriver *sdcp, uint8_t cmd, uint32_t arg,
                                uint32_t *resp) {

  /* Clear residual flags before issuing new command. */
  sdcp->sdio->ICR = SDIO_STA_STATIC_FLAGS;

  /* Set argument. */
  sdcp->sdio->ARG = arg;

  /* Send command with short response expected and CRC check, CPSM enabled. */
  sdcp->sdio->CMD = ((uint32_t)(cmd & 0x3F) |
                     SDIO_CMD_WAITRESP_SHORT |
                     SDIO_CMD_CPSMEN);

  /* Wait for response. */
  if (sdc_lld_wait_cmd_done(sdcp, SDIO_CMD_TIMEOUT)) {
    return HAL_FAILED;
  }

  /* Read short response from RESP1. */
  *resp = sdcp->sdio->RESP1;

  return HAL_SUCCESS;
}

/**
 * @brief   Sends an SDIO command with a long response expected and CRC.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] cmd       card command
 * @param[in] arg       command argument
 * @param[out] resp     pointer to the response buffer (four words)
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 *
 * @notapi
 */
bool sdc_lld_send_cmd_long_crc(SDCDriver *sdcp, uint8_t cmd, uint32_t arg,
                               uint32_t *resp) {

  /* Clear residual flags before issuing new command. */
  sdcp->sdio->ICR = SDIO_STA_STATIC_FLAGS;

  /* Set argument. */
  sdcp->sdio->ARG = arg;

  /* Send command with long response expected and CRC check, CPSM enabled. */
  sdcp->sdio->CMD = ((uint32_t)(cmd & 0x3F) |
                     SDIO_CMD_WAITRESP_LONG |
                     SDIO_CMD_CPSMEN);

  /* Wait for response. */
  if (sdc_lld_wait_cmd_done(sdcp, SDIO_CMD_TIMEOUT)) {
    return HAL_FAILED;
  }

  /* Read long response: RESP1-4.
     RESP1 = card status (LSB), RESP4 = CID/CSD (MSB). */
  resp[0] = sdcp->sdio->RESP1;
  resp[1] = sdcp->sdio->RESP2;
  resp[2] = sdcp->sdio->RESP3;
  resp[3] = sdcp->sdio->RESP4;

  return HAL_SUCCESS;
}

/**
 * @brief   Reads special registers using data bus.
 * @details Needed only during card detection procedure (SCR, etc.).
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[out] buf      pointer to the read buffer
 * @param[in] bytes     number of bytes to read
 * @param[in] cmd       card command
 * @param[in] argument   command argument
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 *
 * @notapi
 */
bool sdc_lld_read_special(SDCDriver *sdcp, uint8_t *buf, size_t bytes,
                          uint8_t cmd, uint32_t argument) {
  uint32_t resp[1];
  uint32_t blksize = (uint32_t)bytes;
  uint32_t *tempbuf = (uint32_t *)buf;

  osalDbgCheck(bytes < 0x1000000U);

  /* Clear flags. */
  sdcp->sdio->ICR = SDIO_STA_STATIC_FLAGS;

  /* Reset data control. */
  sdcp->sdio->DCTRL = 0U;

  /* Set data timeout. */
  sdcp->sdio->DTIMER = SDIO_MAX_DATA_TIMEOUT;

  /* Set data length. */
  sdcp->sdio->DLEN = blksize;

  /* Configure data control for single block read.
     Calculate block size power: bytes -> log2. */
  {
    uint32_t power = 0;
    uint32_t tmp = blksize;
    while (tmp > 1U) {
      tmp >>= 1;
      power++;
    }
    sdcp->sdio->DCTRL = (power << 0) |
                         SDIO_DCTRL_DTDIR |
                         SDIO_DCTRL_DTEN;
  }

  /* Send command. */
  if (sdc_lld_send_cmd_short_crc(sdcp, cmd, argument, resp) ||
      MMCSD_R1_ERROR(resp[0])) {
    sdcp->sdio->DCTRL = 0U;
    return HAL_FAILED;
  }

  /* Wait for data transfer. */
  {
    uint32_t total_words = (blksize + 3U) / 4U;
    uint32_t words_read = 0;
    uint32_t timeout = SDIO_MAX_DATA_TIMEOUT;

    while (words_read < total_words) {
      uint32_t sta = sdcp->sdio->STA;

      if (sta & SDIO_STA_DATA_ERROR_MASK) {
        sdcp->sdio->ICR = SDIO_STA_STATIC_FLAGS;
        sdcp->sdio->DCTRL = 0U;
        return HAL_FAILED;
      }

      if (sta & SDIO_STA_DATAEND) {
        sdcp->sdio->ICR = SDIO_STA_DATAEND;
        break;
      }

      if (sta & SDIO_STA_RXDAVL) {
        *tempbuf++ = sdcp->sdio->FIFO;
        words_read++;
        timeout = SDIO_MAX_DATA_TIMEOUT;
      }
      else {
        if (--timeout == 0U) {
          sdcp->sdio->DCTRL = 0U;
          return HAL_FAILED;
        }
      }
    }
  }

  /* Clear flags. */
  sdcp->sdio->ICR = SDIO_STA_STATIC_FLAGS;

  /* Disable data transfer. */
  sdcp->sdio->DCTRL = 0U;

  return HAL_SUCCESS;
}

/**
 * @brief   Reads one or more blocks.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] startblk  first block to read
 * @param[out] buf      pointer to the read buffer
 * @param[in] n         number of blocks to read
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 *
 * @notapi
 */
bool sdc_lld_read(SDCDriver *sdcp, uint32_t startblk,
                  uint8_t *buf, uint32_t n) {

#if CH32_SDC_UNALIGNED_SUPPORT
  /* Check for unaligned buffer (must be 4-byte aligned for FIFO). */
  if (((uintptr_t)buf & 0x3U) != 0U) {
    uint32_t i;
    for (i = 0U; i < n; i++) {
      if (sdc_lld_read_aligned(sdcp, startblk, sdcp->buf, 1)) {
        return HAL_FAILED;
      }
      memcpy(buf, sdcp->buf, MMCSD_BLOCK_SIZE);
      buf += MMCSD_BLOCK_SIZE;
      startblk++;
    }
    return HAL_SUCCESS;
  }
#else
  osalDbgAssert(((uintptr_t)buf & 0x3U) == 0U, "unaligned buffer");
#endif

  return sdc_lld_read_aligned(sdcp, startblk, buf, n);
}

/**
 * @brief   Writes one or more blocks.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] startblk  first block to write
 * @param[in] buf       pointer to the write buffer
 * @param[in] n         number of blocks to write
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 *
 * @notapi
 */
bool sdc_lld_write(SDCDriver *sdcp, uint32_t startblk,
                   const uint8_t *buf, uint32_t n) {

#if CH32_SDC_UNALIGNED_SUPPORT
  /* Check for unaligned buffer (must be 4-byte aligned for FIFO). */
  if (((uintptr_t)buf & 0x3U) != 0U) {
    uint32_t i;
    for (i = 0U; i < n; i++) {
      memcpy(sdcp->buf, buf, MMCSD_BLOCK_SIZE);
      if (sdc_lld_write_aligned(sdcp, startblk, sdcp->buf, 1)) {
        return HAL_FAILED;
      }
      buf += MMCSD_BLOCK_SIZE;
      startblk++;
    }
    return HAL_SUCCESS;
  }
#else
  osalDbgAssert(((uintptr_t)buf & 0x3U) == 0U, "unaligned buffer");
#endif

  return sdc_lld_write_aligned(sdcp, startblk, buf, n);
}

/**
 * @brief   Waits for card idle condition.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  the operation succeeded.
 * @retval HAL_FAILED   the operation failed.
 *
 * @api
 */
bool sdc_lld_sync(SDCDriver *sdcp) {

  /* SDIO does not have a direct DATA0 status bit like SDMMC.
     Use CMD13 (SEND_STATUS) to poll card state. */
  {
    uint32_t retry = 500;
    uint32_t resp;

    while (retry-- > 0U) {
      if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_SEND_STATUS,
                                     (sdcp->rca << 16), &resp)) {
        return HAL_FAILED;
      }

      /* Check card state bits [12:9]: 0 = idle, 7 = transfer. */
      if (((resp >> 9) & 0xFU) == 4U) {
        return HAL_SUCCESS;
      }

      osalThreadSleep(OSAL_MS2I(1));
    }
  }

  return HAL_FAILED;
}

#endif /* HAL_USE_SDC */

/** @} */
