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
 * @file    SDMMCv1/hal_sdc_lld.c
 * @brief   CH32 SDC subsystem low level driver source.
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
 * @brief   Mask of all interrupt flags.
 */
#define SDMMC_IF_ALL_FLAGS      ((uint16_t)0x03FFU)

/**
 * @brief   Mask of command response error flags.
 */
#define SDMMC_IF_CMD_ERROR_MASK                                               \
  (SDMMC_IF_RE_TMOUT | SDMMC_IF_RECRC_WR | SDMMC_IF_REIDX_ER)

/**
 * @brief   Mask of data transfer error flags.
 */
#define SDMMC_IF_DATA_ERROR_MASK                                              \
  (SDMMC_IF_DATTMO | SDMMC_IF_TRANERR | SDMMC_IF_FIFO_OV)

/**
 * @brief   Response type encoding for CMD_SET register.
 */
#define SDMMC_RESP_NONE         0U
#define SDMMC_RESP_SHORT        2U
#define SDMMC_RESP_LONG         1U

/**
 * @brief   Builds a CMD_SET register value.
 *
 * @param[in] cmd       command index (0-63)
 * @param[in] rpt       response type (0=none, 1=short, 2=long)
 */
#define SDMMC_CMD_SET(cmd, rpt)                                               \
  ((uint16_t)(((cmd) & 0x003F) |                                             \
              (((rpt) & 0x0003) << 8) |                                       \
              0))

/**
 * @brief   Calculates the CLK_DIV divider value for a target frequency.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] f         desired frequency in Hz
 * @return              divider value for CLK_DIV register
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
  uint32_t intfg;

  do {
    intfg = sdcp->sdmmc->INT_FG;

    /* Check for command done. */
    if (intfg & SDMMC_IF_CMDDONE) {
      /* Clear command-related flags. */
      sdcp->sdmmc->INT_FG = (SDMMC_IF_CMDDONE | SDMMC_IF_CMD_ERROR_MASK) &
                            SDMMC_IF_ALL_FLAGS;

      return HAL_SUCCESS;
    }

    /* Check for errors. */
    if (intfg & SDMMC_IF_CMD_ERROR_MASK) {
      sta = sdcp->sdmmc->INT_FG;

      /* Collect error flags. */
      if (sta & SDMMC_IF_RE_TMOUT) {
        sdcp->errors |= SDC_COMMAND_TIMEOUT;
      }
      if (sta & SDMMC_IF_RECRC_WR) {
        sdcp->errors |= SDC_CMD_CRC_ERROR;
      }
      if (sta & SDMMC_IF_REIDX_ER) {
        sdcp->errors |= SDC_STARTBIT_ERROR;
      }

      /* Clear error flags. */
      sdcp->sdmmc->INT_FG = (sta & SDMMC_IF_ALL_FLAGS);

      return HAL_FAILED;
    }

    if (--timeout == 0U) {
      sdcp->errors |= SDC_COMMAND_TIMEOUT;
      sdcp->sdmmc->INT_FG = SDMMC_IF_ALL_FLAGS;
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
  uint32_t intfg;

  do {
    intfg = sdcp->sdmmc->INT_FG;

    /* Check for transfer done. */
    if (intfg & SDMMC_IF_TRANDONE) {
      sdcp->sdmmc->INT_FG = (intfg & SDMMC_IF_ALL_FLAGS);
      return HAL_SUCCESS;
    }

    /* Check for block gap (used in multi-block with auto gap stop). */
    if (intfg & SDMMC_IF_BKGAP) {
      sdcp->sdmmc->INT_FG = SDMMC_IF_BKGAP;
      return HAL_SUCCESS;
    }

    /* Check for errors. */
    if (intfg & SDMMC_IF_DATA_ERROR_MASK) {
      if (intfg & SDMMC_IF_DATTMO) {
        sdcp->errors |= SDC_DATA_TIMEOUT;
      }
      if (intfg & SDMMC_IF_TRANERR) {
        sdcp->errors |= SDC_DATA_CRC_ERROR;
      }
      if (intfg & SDMMC_IF_FIFO_OV) {
        sdcp->errors |= SDC_RX_OVERRUN;
      }

      sdcp->sdmmc->INT_FG = (intfg & SDMMC_IF_ALL_FLAGS);
      return HAL_FAILED;
    }

    if (--timeout == 0U) {
      sdcp->errors |= SDC_DATA_TIMEOUT;
      sdcp->sdmmc->INT_FG = SDMMC_IF_ALL_FLAGS;
      return HAL_FAILED;
    }

  } while (true);
}

/**
 * @brief   Reads one or more blocks (aligned buffer).
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] startblk  first block to read
 * @param[out] buf      pointer to the read buffer (must be 16-byte aligned)
 * @param[in] blocks    number of blocks to read
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 */
static bool sdc_lld_read_aligned(SDCDriver *sdcp, uint32_t startblk,
                                 uint8_t *buf, uint32_t blocks) {
  uint32_t resp[1];
  uint32_t blksize = MMCSD_BLOCK_SIZE;

  osalDbgCheck(blocks < 0x1000000U / MMCSD_BLOCK_SIZE);

  /* Convert to byte address for standard capacity cards. */
  if (!(sdcp->cardmode & SDC_MODE_HIGH_CAPACITY)) {
    startblk *= MMCSD_BLOCK_SIZE;
  }

  /* Wait for card ready (DATA0 line high). */
  {
    uint32_t retry = 500000;
    while ((--retry > 0U) && ((sdcp->sdmmc->STATUS & SDMMC_DAT0STA) == 0U)) {
      /* Spin waiting for DATA0 to go high. */
    }
    if (retry == 0U) {
      return HAL_FAILED;
    }
  }

  /* Clear any pending flags. */
  sdcp->sdmmc->INT_FG = SDMMC_IF_ALL_FLAGS;

  /* Configure block settings: first clear, then set. */
  sdcp->sdmmc->BLOCK_CFG = 0U;

  /* Configure transfer mode: DMA receive, no dual DMA, no boot. */
  // sdcp->sdmmc->TRAN_MODE = (blocks > 1U) ? SDMMC_AUTOGAPSTOP : 0U;
  sdcp->sdmmc->TRAN_MODE = 0;

  /* Enable DMA in CONTROL register. */
  sdcp->sdmmc->CONTROL |= SDMMC_DMAEN;

  /* Set DMA address. */
  sdcp->sdmmc->DMA_BEG1 = (uint32_t)buf;

  /* Set block size and count. */
  sdcp->sdmmc->BLOCK_CFG = ((uint32_t)blksize << 16) | blocks;

  /* Send read command. */
  if (blocks > 1U) {
    if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_READ_MULTIPLE_BLOCK,
                                   startblk, resp) ||
        MMCSD_R1_ERROR(resp[0])) {
      sdcp->sdmmc->CONTROL &= ~SDMMC_DMAEN;
      sdcp->sdmmc->BLOCK_CFG = 0U;
      sdcp->sdmmc->TRAN_MODE = 0U;
      return HAL_FAILED;
    }
  }
  else {
    if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_READ_SINGLE_BLOCK,
                                   startblk, resp) ||
        MMCSD_R1_ERROR(resp[0])) {
      sdcp->sdmmc->CONTROL &= ~SDMMC_DMAEN;
      sdcp->sdmmc->BLOCK_CFG = 0U;
      sdcp->sdmmc->TRAN_MODE = 0U;
      return HAL_FAILED;
    }
  }

  /* Wait for data transfer to complete. */
  {
    uint32_t timeout = 10000000;
    if (sdc_lld_wait_data_done(sdcp, timeout)) {
      sdcp->sdmmc->CONTROL &= ~SDMMC_DMAEN;
      sdcp->sdmmc->BLOCK_CFG = 0U;
      sdcp->sdmmc->TRAN_MODE = 0U;
      return HAL_FAILED;
    }
  }

  /* Disable DMA and clear config. */
  sdcp->sdmmc->CONTROL &= ~SDMMC_DMAEN;
  sdcp->sdmmc->BLOCK_CFG = 0U;
  sdcp->sdmmc->TRAN_MODE = 0U;

  /* For multi-block reads, send stop command. */
  if (blocks > 1U) {
    return sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_STOP_TRANSMISSION, 0,
                                      resp);
  }

  return HAL_SUCCESS;
}

/**
 * @brief   Writes one or more blocks (aligned buffer).
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] startblk  first block to write
 * @param[in] buf       pointer to the write buffer (must be 16-byte aligned)
 * @param[in] blocks    number of blocks to write
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 */
static bool sdc_lld_write_aligned(SDCDriver *sdcp, uint32_t startblk,
                                  const uint8_t *buf, uint32_t blocks) {
  uint32_t resp[1];
  uint32_t blksize = MMCSD_BLOCK_SIZE;

  /* Convert to byte address for standard capacity cards. */
  if (!(sdcp->cardmode & SDC_MODE_HIGH_CAPACITY)) {
    startblk *= MMCSD_BLOCK_SIZE;
  }

  /* Wait for data line released (DATA0 high) before sending command,
     as required by the CH32 SDMMC manual. */
  {
    uint32_t retry = 500000;
    while ((--retry > 0U) && ((sdcp->sdmmc->STATUS & SDMMC_DAT0STA) == 0U)) {
      /* Spin waiting for DATA0 to go high. */
    }
    if (retry == 0U) {
      sdcp->sdmmc->CONTROL &= ~SDMMC_DMAEN;
      sdcp->sdmmc->BLOCK_CFG = 0U;
      sdcp->sdmmc->TRAN_MODE = 0U;
      return HAL_FAILED;
    }
  }

  /* Send write command. */
  if (blocks > 1U) {
    if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_WRITE_MULTIPLE_BLOCK,
                                   startblk, resp) ||
        MMCSD_R1_ERROR(resp[0])) {
      sdcp->sdmmc->CONTROL &= ~SDMMC_DMAEN;
      sdcp->sdmmc->BLOCK_CFG = 0U;
      sdcp->sdmmc->TRAN_MODE = 0U;
      return HAL_FAILED;
    }
  }
  else {
    if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_WRITE_BLOCK,
                                   startblk, resp) ||
        MMCSD_R1_ERROR(resp[0])) {
      sdcp->sdmmc->CONTROL &= ~SDMMC_DMAEN;
      sdcp->sdmmc->BLOCK_CFG = 0U;
      sdcp->sdmmc->TRAN_MODE = 0U;
      return HAL_FAILED;
    }
  }

  /* Clear any pending flags. */
  sdcp->sdmmc->INT_FG = SDMMC_IF_ALL_FLAGS;

  /* Configure block settings: first clear, then set. */
  sdcp->sdmmc->BLOCK_CFG = 0U;

  /* Configure transfer mode: DMA send, no dual DMA, no boot. */
  sdcp->sdmmc->TRAN_MODE = SDMMC_DMA_DIR;

  /* Enable DMA in CONTROL register. */
  sdcp->sdmmc->CONTROL |= SDMMC_DMAEN;

  /* Set DMA address. */
  sdcp->sdmmc->DMA_BEG1 = (uint32_t)buf;

    /* Set block size and count. */
  sdcp->sdmmc->BLOCK_CFG = ((uint32_t)blksize << 16) | blocks;

  /* DMA transfer starts automatically after command response.
     No explicit trigger via WRITE_CONT is needed. */


  /* Wait for data transfer to complete. */
  {
    uint32_t timeout = 10000000;
    if (sdc_lld_wait_data_done(sdcp, timeout)) {
      sdcp->sdmmc->CONTROL &= ~SDMMC_DMAEN;
      sdcp->sdmmc->BLOCK_CFG = 0U;
      sdcp->sdmmc->TRAN_MODE = 0U;
      return HAL_FAILED;
    }
  }

  /* Disable DMA and clear config. */
  sdcp->sdmmc->CONTROL &= ~SDMMC_DMAEN;
  sdcp->sdmmc->BLOCK_CFG = 0U;
  sdcp->sdmmc->TRAN_MODE = 0U;

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
 * @brief   SDMMC interrupt handler.
 * @details Wakes the transaction thread when transfer is done or error.
 *          Note: CH32 QINGKE RISC-V port uses unified IRQ entry, so
 *          OSAL_IRQ_PROLOGUE/EPILOGUE are handled by the dispatcher.
 *
 * @isr
 */
#if CH32_SDC_USE_SDC1 || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(CH32_SDMMC_HANDLER) {

  osalSysLockFromISR();

  /* Disable all SDMMC interrupts. */
  SDCD1.sdmmc->INT_EN = 0U;

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
  SDCD1.sdmmc   = SDMMC;
  SDCD1.clkfreq = CH32_SDC_SDMMC_CLOCK;
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
      /* Enable SDMMC clock and reset. */
      resetHB(RCC_SDMMCRST);
      enableHB(RCC_SDMMCEN);
    }
#endif
  }

  /* Reset SDMMC peripheral. */
  sdcp->sdmmc->CONTROL |= SDMMC_RST_LGC | SDMMC_NEGSMP;
  sdcp->sdmmc->CONTROL &= ~SDMMC_RST_LGC;
  sdcp->sdmmc->CONTROL &= ~SDMMC_ALL_CLR;

  /* Clear all interrupt flags. */
  sdcp->sdmmc->INT_FG = SDMMC_IF_ALL_FLAGS;

  /* Enable all status flags in INT_EN so INT_FG gets latched by hardware.
     Even in polling mode, the controller needs these bits set to record
     command/data completion and error events into INT_FG. */
  sdcp->sdmmc->INT_EN = (uint16_t)(SDMMC_IE_CMDDONE |
                                   SDMMC_IE_RE_TMOUT |
                                   SDMMC_IE_RECRC_WR |
                                   SDMMC_IE_REIDX_ER |
                                   SDMMC_IE_TRANDONE |
                                   SDMMC_IE_DATTMO |
                                   SDMMC_IE_TRANERR |
                                   SDMMC_IE_FIFO_OV);

  NVIC_EnableIRQ(SDMMC_IRQn);

  /* Stop clock. */
  sdcp->sdmmc->CLK_DIV = 0U;

  /* Set timeout. */
  sdcp->sdmmc->TIMEOUT = 0x0CU;

  /* Clear transfer mode and block config. */
  sdcp->sdmmc->TRAN_MODE = 0U;
  sdcp->sdmmc->BLOCK_CFG = 0U;
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

    /* Stop clock. */
    sdcp->sdmmc->CLK_DIV = 0U;

    /* Reset SDMMC. */
    sdcp->sdmmc->CONTROL |= SDMMC_RST_LGC;
    sdcp->sdmmc->CONTROL &= ~SDMMC_RST_LGC;

    /* Disable interrupts. */
    sdcp->sdmmc->INT_EN = 0U;
    sdcp->sdmmc->INT_FG = SDMMC_IF_ALL_FLAGS;

    /* Disable peripheral clock. */
#if CH32_SDC_USE_SDC1
    if (&SDCD1 == sdcp) {
      disableHB(RCC_SDMMCEN);
    }
#endif
  }
}

/**
 * @brief   Starts the SDMMC clock and sets it to init mode (400kHz or less).
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @notapi
 */
void sdc_lld_start_clk(SDCDriver *sdcp) {
  uint32_t div;

  sdcp->sdmmc->CONTROL &= ~SDMMC_LW_MASK;
  
  /* Calculate divider for ~400kHz init clock. */
  div = sdc_lld_clkdiv(sdcp, 400000U);

  /* Set clock divider and enable clock output. */
  sdcp->sdmmc->CLK_DIV = (uint16_t)(div & SDMMC_DIV_MASK) | SDMMC_CLKOE;

  /* Wait for clock to stabilize. */
  osalThreadSleep(OSAL_MS2I(CH32_SDC_CLOCK_DELAY));
}

/**
 * @brief   Sets the SDMMC clock to data mode.
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

  /* Set clock divider while keeping clock enabled. */
  sdcp->sdmmc->CLK_DIV = (uint16_t)(div & SDMMC_DIV_MASK) | SDMMC_CLKOE | SDMMC_CLKMode;
}

/**
 * @brief   Stops the SDMMC clock.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @notapi
 */
void sdc_lld_stop_clk(SDCDriver *sdcp) {

  /* Disable clock output, keep divider. */
  sdcp->sdmmc->CLK_DIV &= ~(uint16_t)SDMMC_CLKOE;
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
  uint16_t ctrl;

  ctrl = sdcp->sdmmc->CONTROL & ~SDMMC_LW_MASK;

  switch (mode) {
  case SDC_MODE_1BIT:
    /* Bus width 1 bit: LW = 0. */
    sdcp->sdmmc->CONTROL = ctrl;
    break;
  case SDC_MODE_4BIT:
    /* Bus width 4 bits: LW = 1. */
    sdcp->sdmmc->CONTROL = ctrl | SDMMC_LW_MASK_0;
    break;
  case SDC_MODE_8BIT:
    /* Bus width 8 bits: LW = 2. */
    sdcp->sdmmc->CONTROL = ctrl | SDMMC_LW_MASK_1;
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
  sdcp->sdmmc->INT_FG = SDMMC_IF_ALL_FLAGS;

  /* Set argument. */
  sdcp->sdmmc->ARGUMENT = arg;

  /* Send command (no response, no CRC/index check). */
  sdcp->sdmmc->CMD_SET = (uint16_t)(cmd & 0x003F);

  /* Wait for command done. */
  {
    uint32_t timeout = 1000000U;
    while ((--timeout > 0U) &&
           ((sdcp->sdmmc->INT_FG & SDMMC_IF_CMDDONE) == 0U)) {
      /* Spin. */
    }
  }

  /* Clear flag. */
  sdcp->sdmmc->INT_FG = SDMMC_IF_ALL_FLAGS;
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
  sdcp->sdmmc->INT_FG = SDMMC_IF_ALL_FLAGS;

  /* Set argument. */
  sdcp->sdmmc->ARGUMENT = arg;

  /* Send command with short response expected, CRC check disabled.
     Note: CKCRC not set to allow CRC-less commands like CMD1/ACMD41. */
  sdcp->sdmmc->CMD_SET = SDMMC_CMD_SET(cmd, SDMMC_RESP_SHORT) &
                          ~((uint16_t)SDMMC_CKCRC);

  /* Wait for response. */
  if (sdc_lld_wait_cmd_done(sdcp, 1000000U)) {
    return HAL_FAILED;
  }

  /* Read short response from RESPONSE3. */
  *resp = sdcp->sdmmc->RESPONSE3;

  return HAL_SUCCESS;
}

/**
 * @brief   Sends an SDIO command with a long response expected.
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
  sdcp->sdmmc->INT_FG = SDMMC_IF_ALL_FLAGS;

  /* Set argument. */
  sdcp->sdmmc->ARGUMENT = arg;

  /* Send command with short response expected and CRC check. */
  sdcp->sdmmc->CMD_SET = SDMMC_CMD_SET(cmd, SDMMC_RESP_SHORT);

  /* Wait for response. */
  if (sdc_lld_wait_cmd_done(sdcp, 1000000U)) {
    return HAL_FAILED;
  }

  /* Read short response from RESPONSE3. */
  *resp = sdcp->sdmmc->RESPONSE3;

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
  sdcp->sdmmc->INT_FG = SDMMC_IF_ALL_FLAGS;

  /* Set argument. */
  sdcp->sdmmc->ARGUMENT = arg;

  /* Send command with long response expected and CRC check. */
  sdcp->sdmmc->CMD_SET = SDMMC_CMD_SET(cmd, SDMMC_RESP_LONG);

  /* Wait for response. */
  if (sdc_lld_wait_cmd_done(sdcp, 1000000U)) {
    return HAL_FAILED;
  }

  /* Read long response: RESPONSE3 = first received word (MSB),
     RESPONSE0 = last received word (LSB).
     Same order as STM32: RESP1=card response low, RESP4=card response high. */
  resp[0] = sdcp->sdmmc->RESPONSE0;
  resp[1] = sdcp->sdmmc->RESPONSE1;
  resp[2] = sdcp->sdmmc->RESPONSE2;
  resp[3] = sdcp->sdmmc->RESPONSE3;

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

  osalDbgCheck(bytes < 0x1000000U);

  /* Wait for card ready. */
  {
    uint32_t retry = 500000;
    while ((--retry > 0U) && ((sdcp->sdmmc->STATUS & SDMMC_DAT0STA) == 0U)) {
      /* Spin. */
    }
    if (retry == 0U) {
      return HAL_FAILED;
    }
  }

  /* Clear flags. */
  sdcp->sdmmc->INT_FG = SDMMC_IF_ALL_FLAGS;

  /* Configure block: first clear, then set. */
  sdcp->sdmmc->BLOCK_CFG = 0U;

  /* Transfer mode: DMA receive, single block. */
  sdcp->sdmmc->TRAN_MODE = 0U;

  /* Set block count=1, size=bytes. */
  sdcp->sdmmc->BLOCK_CFG = ((uint32_t)blksize << 16) | 1U;

  /* Set DMA address. */
  sdcp->sdmmc->DMA_BEG1 = (uint32_t)buf;

  /* Enable DMA. */
  sdcp->sdmmc->CONTROL |= SDMMC_DMAEN;

  /* Send command. */
  if (sdc_lld_send_cmd_short_crc(sdcp, cmd, argument, resp) ||
      MMCSD_R1_ERROR(resp[0])) {
    sdcp->sdmmc->CONTROL &= ~SDMMC_DMAEN;
    sdcp->sdmmc->BLOCK_CFG = 0U;
    sdcp->sdmmc->TRAN_MODE = 0U;
    return HAL_FAILED;
  }

  /* Wait for data transfer. */
  {
    uint32_t timeout = 10000000;
    if (sdc_lld_wait_data_done(sdcp, timeout)) {
      sdcp->sdmmc->CONTROL &= ~SDMMC_DMAEN;
      sdcp->sdmmc->BLOCK_CFG = 0U;
      sdcp->sdmmc->TRAN_MODE = 0U;
      return HAL_FAILED;
    }
  }

  /* Clean up. */
  sdcp->sdmmc->CONTROL &= ~SDMMC_DMAEN;
  sdcp->sdmmc->BLOCK_CFG = 0U;
  sdcp->sdmmc->TRAN_MODE = 0U;

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
  /* Check for unaligned buffer (must be 16-byte aligned for DMA). */
  if (((uintptr_t)buf & 0xFU) != 0U) {
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
  osalDbgAssert(((uintptr_t)buf & 0xFU) == 0U, "unaligned buffer");
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
  /* Check for unaligned buffer (must be 16-byte aligned for DMA). */
  if (((uintptr_t)buf & 0xFU) != 0U) {
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
  osalDbgAssert(((uintptr_t)buf & 0xFU) == 0U, "unaligned buffer");
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

  /* Wait for DATA0 line to go high indicating card is not busy. */
  {
    uint32_t retry = 500000;
    while ((--retry > 0U) && ((sdcp->sdmmc->STATUS & SDMMC_DAT0STA) == 0U)) {
      /* Spin. */
    }
    if (retry == 0U) {
      return HAL_FAILED;
    }
  }

  return HAL_SUCCESS;
}

#endif /* HAL_USE_SDC */

/** @} */
