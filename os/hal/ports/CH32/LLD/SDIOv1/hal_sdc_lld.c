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
 * @brief   CH32 SDC subsystem low level driver source.
 *
 * @details CH32H417 SDIO peripheral driver.
 *
 * @note    The CH32 SDIO controller has a quirk: for response types that
 *          do not carry a valid CRC (R3, R7), the hardware still performs
 *          CRC checking and sets the CCRCFAIL flag even though the response
 *          was received correctly. This driver ignores CCRCFAIL for those
 *          response types, matching the official WCH EVT example behavior.
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

/*
 * SDIO clock source frequency.  The CH32H417 HB2 bus driving the SDIO
 * peripheral can run at various frequencies; 100 MHz is the common default
 * from mcuconf.h.
 */
#if !defined(CH32_SDC_SDIO_CLOCK) || defined(__DOXYGEN__)
#define CH32_SDC_SDIO_CLOCK             100000000U
#endif

/*
 * SDIO command timeout in iterations.
 */
#define SDIO_CMD_TIMEOUT                0x000FFFFFU

/*
 * SDIO data timeout in SDIO clock cycles.
 */
#define SDIO_DATATIMEOUT                0x00FFFFFFU

/*
 * SDIO read/write timeout in milliseconds.
 */
#if !defined(CH32_SDC_READ_TIMEOUT) || defined(__DOXYGEN__)
#define CH32_SDC_READ_TIMEOUT           10000
#endif

#if !defined(CH32_SDC_WRITE_TIMEOUT) || defined(__DOXYGEN__)
#define CH32_SDC_WRITE_TIMEOUT          10000
#endif

/*
 * SDIO FIFO half-watermark: 8 words = 32 bytes.
 */
#define SDIO_HALFIFO                    8U
#define SDIO_HALFIFO_BYTES              32U

/* * CMD register clear mask (matches WCH ch32h417_sdio.c).
 * Clears bits 0-10, preserves bits 11-31.
 */
#define SDIO_CMD_CLEAR_MASK                ((uint32_t)0xFFFFF800U)

/*
 * SDIO_INIT_CLK_DIV from WCH example (0xB2 = 178).
 */
#define SDIO_INIT_CLK_DIV                  0xB2U

/*
 * Number of CMD0 retries during card init (SD spec: >= 74 clock cycles).
 */
#define SDIO_CMD0_MAX_RETRIES              74U

/* * All SDIO interrupt/clear flags used for static clearing.
 */
#define SDIO_STA_STATIC_FLAGS                                                \
  (SDIO_STA_CCRCFAIL | SDIO_STA_DCRCFAIL |                                  \
   SDIO_STA_CTIMEOUT | SDIO_STA_DTIMEOUT |                                  \
   SDIO_STA_TXUNDERR | SDIO_STA_RXOVERR  |                                  \
   SDIO_STA_CMDREND  | SDIO_STA_CMDSENT  |                                  \
   SDIO_STA_DATAEND  | SDIO_STA_STBITERR |                                  \
   SDIO_STA_DBCKEND)

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

#if (CH32_SDC_USE_SDC1 == TRUE) || defined(__DOXYGEN__)
/** @brief SDCD1 driver identifier.*/
SDCDriver SDCD1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/** @brief Default SDC configuration.*/
static const SDCConfig sdc_default_cfg = {
  SDC_MODE_4BIT
};

#if (CH32_SDC_USE_SDC1 == TRUE) || defined(__DOXYGEN__)
/** @brief Bounce buffer for unaligned transfers.*/
static uint8_t __attribute__((aligned(4))) sd1_bounce_buf[MMCSD_BLOCK_SIZE];
#endif

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Calculates a clock divider for the specified frequency.
 *
 * @param[in] f         desired frequency in Hz
 * @return              The CLKCR divider value.
 */
static uint32_t sdc_lld_clkdiv(uint32_t f) {
  uint32_t div;

  div = CH32_SDC_SDIO_CLOCK / f;
  if (div == 1U) {
    return SDIO_CLKCR_BYPASS;
  }
  return div - 2U;
}

/**
 * @brief   Calculates the DTIMER value for a given timeout in milliseconds.
 *
 * @param[in] ms        timeout in milliseconds
 * @return              The DTIMER value in SDIO clock cycles.
 */
static uint32_t sdc_lld_get_timeout(uint32_t ms) {
  uint32_t div, clkcr;

  clkcr = SDCD1.sdio->CLKCR;
  if ((clkcr & SDIO_CLKCR_BYPASS) != 0U) {
    div = 1U;
  }
  else {
    div = (clkcr & SDIO_CLKCR_CLKDIV_Msk) + 2U;
  }
  return ((CH32_SDC_SDIO_CLOCK / (div)) / 1000U) * ms;
}

/**
 * @brief   Collects error flags from the STA register.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] sta       value of the STA register
 */
static void sdc_lld_collect_errors(SDCDriver *sdcp, uint32_t sta) {
  uint32_t errors = SDC_NO_ERROR;

  if (sta & SDIO_STA_CCRCFAIL)
    errors |= SDC_CMD_CRC_ERROR;
  if (sta & SDIO_STA_DCRCFAIL)
    errors |= SDC_DATA_CRC_ERROR;
  if (sta & SDIO_STA_CTIMEOUT)
    errors |= SDC_COMMAND_TIMEOUT;
  if (sta & SDIO_STA_DTIMEOUT)
    errors |= SDC_DATA_TIMEOUT;
  if (sta & SDIO_STA_TXUNDERR)
    errors |= SDC_TX_UNDERRUN;
  if (sta & SDIO_STA_RXOVERR)
    errors |= SDC_RX_OVERRUN;
  if (sta & SDIO_STA_STBITERR)
    errors |= SDC_STARTBIT_ERROR;

  sdcp->errors |= errors;
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/**
 * @brief   SDIO IRQ handler.
 * @details Wakes the transaction thread on command/data completion or error.
 */
#if (CH32_SDC_USE_SDC1 == TRUE) || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(CH32_SDIO_HANDLER) {

  osalSysLockFromISR();

  /* Disable all SDIO interrupts but don't clear flags;
     the polling functions will read and clear them. */
  SDCD1.sdio->MASK = 0U;

  osalThreadResumeI(&SDCD1.thread, MSG_OK);

  osalSysUnlockFromISR();

}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level SDC driver initialization.
 *
 * @notapi
 */
void sdc_lld_init(void) {

#if (CH32_SDC_USE_SDC1 == TRUE)
  sdcObjectInit(&SDCD1);
  SDCD1.thread = NULL;
  SDCD1.sdio   = SDIO;
  SDCD1.buf    = sd1_bounce_buf;
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

  /* Checking configuration, using a default if NULL has been passed.*/
  if (sdcp->config == NULL) {
    sdcp->config = &sdc_default_cfg;
  }

  if (sdcp->state == BLK_STOP) {

    /* Enable SDIO peripheral clock and reset. */
    enableHB2(RCC_SDIOEN);
    resetHB2(RCC_SDIORST);

    /* Enable NVIC interrupt. */
    NVIC_SetPriority(CH32_SDIO_NUMBER, CH32_SDC_SDIO_IRQ_PRIORITY);
    NVIC_EnableIRQ(CH32_SDIO_NUMBER);
  }

  /* Reset SDIO peripheral registers to a known state, matching
   * WCH SDIO_DeInit sequence. */
  sdcp->sdio->POWER  = 0U;
  sdcp->sdio->CLKCR  = 0U;
  sdcp->sdio->ARG    = 0U;
  sdcp->sdio->CMD    = 0U;
  sdcp->sdio->DTIMER = 0U;
  sdcp->sdio->DLEN   = 0U;
  sdcp->sdio->DCTRL  = 0U;
  sdcp->sdio->ICR    = 0x00C007FFU;
  sdcp->sdio->MASK   = 0U;
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

    /* SDIO deactivation.*/
    sdcp->sdio->POWER  = 0U;
    sdcp->sdio->CLKCR  = 0U;
    sdcp->sdio->DCTRL  = 0U;
    sdcp->sdio->DTIMER = 0U;

    /* Disable NVIC interrupt. */
    NVIC_DisableIRQ(CH32_SDIO_NUMBER);

    /* Disable SDIO peripheral clock. */
    disableHB2(RCC_SDIOEN);
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

  /* Initial clock setting: ~400kHz, 1bit mode.
   * Match WCH SD_PowerON: set CLKCR first, then power, then CLKEN.
   * Use WCH init divider for reliable 400kHz. */
  sdcp->sdio->CLKCR = sdc_lld_clkdiv(400000U);
  sdcp->sdio->POWER |= SDIO_POWER_PWRCTRL_0 | SDIO_POWER_PWRCTRL_1;
  sdcp->sdio->CLKCR |= SDIO_CLKCR_CLKEN;

  /* Clock activation delay.*/
  osalThreadSleep(OSAL_MS2I(CH32_SDC_CLOCK_DELAY));
}

/**
 * @brief   Sets the SDIO clock to data mode (25/50 MHz or less).
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] clk       the clock mode
 *
 * @notapi
 */
void sdc_lld_set_data_clk(SDCDriver *sdcp, sdcbusclk_t clk) {

  if (SDC_CLK_50MHz == clk) {
    sdcp->sdio->CLKCR = (sdcp->sdio->CLKCR & ~(SDIO_CLKCR_BYPASS |
                                                 SDIO_CLKCR_CLKDIV_Msk)) |
                         sdc_lld_clkdiv(50000000U);
  }
  else {
    sdcp->sdio->CLKCR = (sdcp->sdio->CLKCR & ~(SDIO_CLKCR_BYPASS |
                                                 SDIO_CLKCR_CLKDIV_Msk)) |
                         sdc_lld_clkdiv(25000000U);
  }
}

/**
 * @brief   Stops the SDIO clock.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @notapi
 */
void sdc_lld_stop_clk(SDCDriver *sdcp) {

  sdcp->sdio->CLKCR = 0U;
  sdcp->sdio->POWER = 0U;
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
  uint32_t clk = sdcp->sdio->CLKCR & ~SDIO_CLKCR_WIDBUS;

  switch (mode) {
  case SDC_MODE_1BIT:
    sdcp->sdio->CLKCR = clk;
    break;
  case SDC_MODE_4BIT:
    sdcp->sdio->CLKCR = clk | SDIO_CLKCR_WIDBUS_0;
    break;
  case SDC_MODE_8BIT:
    sdcp->sdio->CLKCR = clk | SDIO_CLKCR_WIDBUS_1;
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
  uint32_t timeout, i;
  uint32_t tmpreg;
  uint32_t retries = (cmd == 0U) ? SDIO_CMD0_MAX_RETRIES : 1U;

  for (i = 0U; i < retries; i++) {
    sdcp->sdio->ARG = arg;

    /* Read-modify-write CMD register (matching WCH SDIO_SendCommand). */
    tmpreg  = sdcp->sdio->CMD;
    tmpreg &= SDIO_CMD_CLEAR_MASK;
    tmpreg |= (uint32_t)cmd | SDIO_CMD_CPSMEN;
    sdcp->sdio->CMD = tmpreg;

    timeout = SDIO_CMD_TIMEOUT;
    while ((sdcp->sdio->STA & SDIO_STA_CMDSENT) == 0U) {
      if (--timeout == 0U) {
        break;
      }
    }
    if (timeout > 0U) {
      sdcp->sdio->ICR = SDIO_STA_CMDSENT;
      return;
    }
  }
  /* All retries exhausted. */
  sdcp->sdio->ICR = SDIO_ICR_ALL_FLAGS;
}

/**
 * @brief   Sends an SDIO command with a short response expected.
 * @note    The CRC is not verified.  For responses without a valid CRC
 *          (R3, R7), the CH32 SDIO hardware still sets CCRCFAIL because
 *          it always checks CRC.  This function ignores CCRCFAIL.
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
  uint32_t sta;
  uint32_t timeout;
  uint32_t tmpreg;

  sdcp->sdio->ARG = arg;

  /* Read-modify-write CMD register. */
  tmpreg  = sdcp->sdio->CMD;
  tmpreg &= SDIO_CMD_CLEAR_MASK;
  tmpreg |= (uint32_t)cmd | SDIO_CMD_WAITRESP_0 | SDIO_CMD_CPSMEN;
  sdcp->sdio->CMD = tmpreg;

  /* Wait for response: CMDREND, CCRCFAIL, or CTIMEOUT.
   * For R3/R7 responses, CCRCFAIL is expected and harmless - the CH32
   * hardware checks CRC even for responses that don't have valid CRC per
   * the SD spec.  We only treat CTIMEOUT as a real error. */
  timeout = SDIO_CMD_TIMEOUT;
  while (((sta = sdcp->sdio->STA) & (SDIO_STA_CMDREND |
                                     SDIO_STA_CTIMEOUT |
                                     SDIO_STA_CCRCFAIL)) == 0U) {
    if (--timeout == 0U) {
      sdcp->sdio->ICR = SDIO_ICR_ALL_FLAGS;
      sdcp->errors |= SDC_COMMAND_TIMEOUT;
      return HAL_FAILED;
    }
  }

  /* Clear the flags we observed. */
  sdcp->sdio->ICR = sta & (SDIO_STA_CMDREND | SDIO_STA_CTIMEOUT |
                           SDIO_STA_CCRCFAIL);

  /* Only timeout is fatal here.  CCRCFAIL is ignored because this
   * function is used for responses without valid CRC (R3, R7). */
  if ((sta & SDIO_STA_CTIMEOUT) != 0U) {
    sdc_lld_collect_errors(sdcp, sta);
    return HAL_FAILED;
  }

  *resp = sdcp->sdio->RESP1;
  return HAL_SUCCESS;
}

/**
 * @brief   Sends an SDIO command with a short response expected and CRC.
 * @note    Both CTIMEOUT and CCRCFAIL are treated as errors.
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
  uint32_t sta;
  uint32_t timeout;
  uint32_t tmpreg;

  sdcp->sdio->ARG = arg;

  /* Read-modify-write CMD register. */
  tmpreg  = sdcp->sdio->CMD;
  tmpreg &= SDIO_CMD_CLEAR_MASK;
  tmpreg |= (uint32_t)cmd | SDIO_CMD_WAITRESP_0 | SDIO_CMD_CPSMEN;
  sdcp->sdio->CMD = tmpreg;

  timeout = SDIO_CMD_TIMEOUT;
  while (((sta = sdcp->sdio->STA) & (SDIO_STA_CMDREND |
                                     SDIO_STA_CTIMEOUT |
                                     SDIO_STA_CCRCFAIL)) == 0U) {
    if (--timeout == 0U) {
      sdcp->sdio->ICR = SDIO_ICR_ALL_FLAGS;
      sdcp->errors |= SDC_COMMAND_TIMEOUT;
      return HAL_FAILED;
    }
  }

  sdcp->sdio->ICR = sta & (SDIO_STA_CMDREND | SDIO_STA_CTIMEOUT |
                           SDIO_STA_CCRCFAIL);

  /* CH32 SDIO quirk: for responses without valid CRC (R3/R7), the
   * hardware always checks CRC and sets CCRCFAIL instead of CMDREND,
   * even though the response was received correctly.  Only CTIMEOUT
   * is a real error.  This matches WCH CmdResp7Error/CmdResp3Error. */
  if ((sta & SDIO_STA_CTIMEOUT) != 0U) {
    sdc_lld_collect_errors(sdcp, sta);
    return HAL_FAILED;
  }

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
  uint32_t sta;
  uint32_t timeout;
  uint32_t tmpreg;

  sdcp->sdio->ARG = arg;

  /* Read-modify-write CMD register. */
  tmpreg  = sdcp->sdio->CMD;
  tmpreg &= SDIO_CMD_CLEAR_MASK;
  tmpreg |= (uint32_t)cmd | SDIO_CMD_WAITRESP_0 |
                            SDIO_CMD_WAITRESP_1 |
                            SDIO_CMD_CPSMEN;
  sdcp->sdio->CMD = tmpreg;

  timeout = SDIO_CMD_TIMEOUT;
  while (((sta = sdcp->sdio->STA) & (SDIO_STA_CMDREND |
                                     SDIO_STA_CTIMEOUT |
                                     SDIO_STA_CCRCFAIL)) == 0U) {
    if (--timeout == 0U) {
      sdcp->sdio->ICR = SDIO_ICR_ALL_FLAGS;
      sdcp->errors |= SDC_COMMAND_TIMEOUT;
      return HAL_FAILED;
    }
  }

  sdcp->sdio->ICR = sta & (SDIO_STA_CMDREND | SDIO_STA_CTIMEOUT |
                           SDIO_STA_CCRCFAIL);

  /* Only CTIMEOUT is a real error.  CCRCFAIL alone means the long
   * response was received but CRC check failed (CH32 quirk). */
  if ((sta & SDIO_STA_CTIMEOUT) != 0U) {
    sdc_lld_collect_errors(sdcp, sta);
    return HAL_FAILED;
  }

  /* Save bytes in MSB-first order as received from the peripheral. */
  *resp++ = sdcp->sdio->RESP4;
  *resp++ = sdcp->sdio->RESP3;
  *resp++ = sdcp->sdio->RESP2;
  *resp   = sdcp->sdio->RESP1;
  return HAL_SUCCESS;
}

/**
 * @brief   Reads special registers using the data bus.
 * @details Used during card detection (SCR, etc.).
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[out] buf      pointer to the read buffer
 * @param[in] bytes     number of bytes to read
 * @param[in] cmd       card command
 * @param[in] argument  command argument
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
  uint32_t sta;
  uint32_t timeout;

  osalDbgCheck(bytes < 0x1000000U);

  /* Set data timeout. */
  sdcp->sdio->DTIMER = sdc_lld_get_timeout(CH32_SDC_READ_TIMEOUT);

  /* Clear all flags. */
  sdcp->sdio->ICR = SDIO_ICR_ALL_FLAGS;

  /* Configure data transfer: single block, card-to-host, 512-byte blocks. */
  sdcp->sdio->DLEN  = (uint32_t)bytes;

  /* DCTRL: data direction = from card, 9-bit block size (512), DMA disabled,
   * data transfer enabled. */
  sdcp->sdio->DCTRL = SDIO_DCTRL_DTDIR |
                      SDIO_DCTRL_DTMODE |        /* data transfer mode */
                      SDIO_DCTRL_DBLOCKSIZE_3 |  /* bit 4: block size bit 3 */
                      SDIO_DCTRL_DBLOCKSIZE_0 |  /* block size = 512 (9 bits) */
                      SDIO_DCTRL_DTEN;

  /* Enable relevant interrupts for thread wake-up. */
  sdcp->sdio->MASK = SDIO_MASK_DCRCFAILIE |
                     SDIO_MASK_DTIMEOUTIE |
                     SDIO_MASK_STBITERRIE |
                     SDIO_MASK_RXOVERRIE  |
                     SDIO_MASK_DATAENDIE;

  /* Send the command. */
  if (sdc_lld_send_cmd_short_crc(sdcp, cmd, argument, resp) ||
      MMCSD_R1_ERROR(resp[0])) {
    sdcp->sdio->DCTRL = 0U;
    sdcp->sdio->MASK  = 0U;
    sdcp->sdio->ICR   = SDIO_ICR_ALL_FLAGS;
    return HAL_FAILED;
  }

  /* Polling loop: read data from FIFO as it arrives (WCH approach).
   * The FIFO is only 32 words (128 bytes), so we must read while
   * receiving — cannot wait for DATAEND first or data will overflow. */
  timeout = SDIO_DATATIMEOUT;
  while (1) {
    sta = sdcp->sdio->STA;

    /* When FIFO is half-full (8 words), read them out immediately. */
    if (sta & SDIO_STA_RXFIFOHF) {
      uint32_t i;
      for (i = 0U; i < SDIO_HALFIFO; i++) {
        *(uint32_t *)buf = sdcp->sdio->FIFO;
        buf += 4U;
      }
      timeout = SDIO_DATATIMEOUT;  /* Reset timeout on progress. */
    }

    if (sta & SDIO_STA_DATAEND) {
      /* Drain any remaining bytes from FIFO. */
      while (sdcp->sdio->FIFOCNT > 0U) {
        *(uint32_t *)buf = sdcp->sdio->FIFO;
        buf += 4U;
      }
      sdcp->sdio->ICR = SDIO_STA_DATAEND;
      sdcp->sdio->DCTRL = 0U;
      sdcp->sdio->MASK  = 0U;
      return HAL_SUCCESS;
    }

    if (sta & SDIO_STA_ERROR_MASK) {
      sdcp->sdio->DCTRL = 0U;
      sdcp->sdio->MASK  = 0U;
      sdc_lld_collect_errors(sdcp, sta);
      sdcp->sdio->ICR = sta & SDIO_ICR_ALL_FLAGS;
      return HAL_FAILED;
    }

    if (--timeout == 0U) {
      sdcp->sdio->DCTRL = 0U;
      sdcp->sdio->MASK  = 0U;
      sdcp->sdio->ICR   = SDIO_ICR_ALL_FLAGS;
      sdcp->errors |= SDC_DATA_TIMEOUT;
      return HAL_FAILED;
    }
  }
}

/**
 * @brief   Reads one or more blocks.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] startblk  first block to read
 * @param[out] buf      pointer to the read buffer
 * @param[in] blocks    number of blocks to read
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 *
 * @notapi
 */
bool sdc_lld_read(SDCDriver *sdcp, uint32_t startblk,
                  uint8_t *buf, uint32_t blocks) {
  uint32_t resp[1];
  uint32_t sta;
  uint32_t timeout;
  uint32_t blksize = MMCSD_BLOCK_SIZE;

#if CH32_SDC_UNALIGNED_SUPPORT
  if (((uintptr_t)buf & 3U) != 0U) {
    uint32_t i;
    for (i = 0U; i < blocks; i++) {
      if (sdc_lld_read(sdcp, startblk, sdcp->buf, 1)) {
        return HAL_FAILED;
      }
      memcpy(buf, sdcp->buf, MMCSD_BLOCK_SIZE);
      buf += MMCSD_BLOCK_SIZE;
      startblk++;
    }
    return HAL_SUCCESS;
  }
#endif

  osalDbgCheck(blocks < 0x1000000U / MMCSD_BLOCK_SIZE);

  /* Convert to byte address for non-HC cards. */
  if (!(sdcp->cardmode & SDC_MODE_HIGH_CAPACITY))
    startblk *= MMCSD_BLOCK_SIZE;

  /* Set data timeout. */
  sdcp->sdio->DTIMER = sdc_lld_get_timeout(CH32_SDC_READ_TIMEOUT);

  /* Checks for errors and waits for the card to be ready for reading.*/
  if (_sdc_wait_for_transfer_state(sdcp))
    return HAL_FAILED;

  sdcp->sdio->DLEN  = 0;
  sdcp->sdio->DCTRL = SDIO_DCTRL_DTEN;

  /* Clear all flags. */
  sdcp->sdio->ICR = SDIO_ICR_ALL_FLAGS;

  /* Configure data transfer BEFORE sending command (WCH sequence).
   * DCTRL must be set up so the data path state machine (DPSM) is ready
   * to receive data when the card starts responding. */
  sdcp->sdio->DLEN  = (uint32_t)(blocks * blksize);
  sdcp->sdio->DCTRL = SDIO_DCTRL_DTDIR |
                      SDIO_DCTRL_DBLOCKSIZE_3 |
                      SDIO_DCTRL_DBLOCKSIZE_0 |
                      SDIO_DCTRL_DTEN;

  /* Enable relevant interrupts. */
  sdcp->sdio->MASK = SDIO_MASK_DCRCFAILIE |
                     SDIO_MASK_DTIMEOUTIE |
                     SDIO_MASK_STBITERRIE |
                     SDIO_MASK_RXOVERRIE  |
                     SDIO_MASK_DATAENDIE;

  /* Send read command. */
  if (blocks > 1U) {
    if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_READ_MULTIPLE_BLOCK,
                                   startblk, resp) ||
        MMCSD_R1_ERROR(resp[0])) {
      goto error;
    }
  }
  else {
    if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_READ_SINGLE_BLOCK,
                                   startblk, resp) ||
        MMCSD_R1_ERROR(resp[0])) {
      goto error;
    }
  }

  /* Polling loop: read data from FIFO as it arrives (WCH approach). */
  timeout = SDIO_DATATIMEOUT;
  while (1) {
    sta = sdcp->sdio->STA;

    /* When FIFO is half-full (8 words), read them out immediately. */
    if (sta & SDIO_STA_RXFIFOHF) {
      uint32_t i;
      for (i = 0U; i < SDIO_HALFIFO; i++) {
        *(uint32_t *)buf = sdcp->sdio->FIFO;
        buf += 4U;
      }
      timeout = SDIO_DATATIMEOUT;  /* Reset timeout on progress. */
    }

    if (sta & SDIO_STA_DATAEND) {
      /* Drain any remaining bytes from FIFO. */
      while (sdcp->sdio->FIFOCNT > 0U) {
        *(uint32_t *)buf = sdcp->sdio->FIFO;
        buf += 4U;
      }
      sdcp->sdio->ICR = SDIO_STA_DATAEND;
      sdcp->sdio->DCTRL = 0U;
      sdcp->sdio->MASK  = 0U;
      goto done;
    }

    if (sta & SDIO_STA_ERROR_MASK) {
      sdcp->sdio->DCTRL = 0U;
      sdcp->sdio->MASK  = 0U;
      sdc_lld_collect_errors(sdcp, sta);
      sdcp->sdio->ICR = sta & SDIO_ICR_ALL_FLAGS;
      goto error_stop;
    }

    if (--timeout == 0U) {
      sdcp->sdio->DCTRL = 0U;
      sdcp->sdio->MASK  = 0U;
      sdcp->sdio->ICR   = SDIO_ICR_ALL_FLAGS;
      sdcp->errors |= SDC_DATA_TIMEOUT;
      goto error;
    }
  }

done:
  /* For multi-block reads, send stop command. */
  if (blocks > 1U) {
    return sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_STOP_TRANSMISSION,
                                      0, resp);
  }
  return HAL_SUCCESS;

error_stop:
  if (blocks > 1U) {
    sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_STOP_TRANSMISSION, 0, resp);
  }

error:
  return HAL_FAILED;
}

/**
 * @brief   Writes one or more blocks.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] startblk  first block to write
 * @param[in] buf       pointer to the write buffer
 * @param[in] blocks    number of blocks to write
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 *
 * @notapi
 */
bool sdc_lld_write(SDCDriver *sdcp, uint32_t startblk,
                   const uint8_t *buf, uint32_t blocks) {
  uint32_t resp[1];
  uint32_t sta;
  uint32_t timeout;
  uint32_t blksize = MMCSD_BLOCK_SIZE;

#if CH32_SDC_UNALIGNED_SUPPORT
  if (((uintptr_t)buf & 3U) != 0U) {
    uint32_t i;
    for (i = 0U; i < blocks; i++) {
      memcpy(sdcp->buf, buf, MMCSD_BLOCK_SIZE);
      buf += MMCSD_BLOCK_SIZE;
      if (sdc_lld_write(sdcp, startblk, sdcp->buf, 1))
        return HAL_FAILED;
      startblk++;
    }
    return HAL_SUCCESS;
  }
#endif

  osalDbgCheck(blocks < 0x1000000U / MMCSD_BLOCK_SIZE);

  /* Convert to byte address for non-HC cards. */
  if (!(sdcp->cardmode & SDC_MODE_HIGH_CAPACITY))
    startblk *= MMCSD_BLOCK_SIZE;

  /* Set data timeout. */
  sdcp->sdio->DTIMER = sdc_lld_get_timeout(CH32_SDC_WRITE_TIMEOUT);

  /* Checks for errors and waits for the card to be ready for reading.*/
  if (_sdc_wait_for_transfer_state(sdcp))
    return HAL_FAILED;

  /* Clear all flags. */
  sdcp->sdio->ICR = SDIO_ICR_ALL_FLAGS;

  /* Configure data transfer BEFORE sending command (WCH sequence).
   * DCTRL must be set up so the data path state machine (DPSM) is ready
   * to send data when the card is ready. */
  sdcp->sdio->DLEN  = (uint32_t)(blocks * blksize);
  sdcp->sdio->DCTRL = SDIO_DCTRL_DBLOCKSIZE_3 |
                      SDIO_DCTRL_DBLOCKSIZE_0 |
                      SDIO_DCTRL_DTEN;

  /* Enable relevant interrupts. */
  sdcp->sdio->MASK = SDIO_MASK_DCRCFAILIE |
                     SDIO_MASK_DTIMEOUTIE |
                     SDIO_MASK_STBITERRIE |
                     SDIO_MASK_TXUNDERRIE |
                     SDIO_MASK_DATAENDIE;

  /* Send write command. */
  if (blocks > 1U) {
    if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_WRITE_MULTIPLE_BLOCK,
                                   startblk, resp) ||
        MMCSD_R1_ERROR(resp[0])) {
      goto error;
    }
  }
  else {
    if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_WRITE_BLOCK,
                                   startblk, resp) ||
        MMCSD_R1_ERROR(resp[0])) {
      goto error;
    }
  }

  /* Polling loop: wait for DATAEND or error flags. */
  timeout = SDIO_CMD_TIMEOUT;
  while (1) {
    sta = sdcp->sdio->STA;

    if (sta & SDIO_STA_DATAEND) {
      sdcp->sdio->ICR = SDIO_STA_DATAEND;
      sdcp->sdio->DCTRL = 0U;
      sdcp->sdio->MASK  = 0U;
      goto done;
    }

    if (sta & SDIO_STA_ERROR_MASK) {
      sdcp->sdio->DCTRL = 0U;
      sdcp->sdio->MASK  = 0U;
      sdc_lld_collect_errors(sdcp, sta);
      sdcp->sdio->ICR = sta & SDIO_ICR_ALL_FLAGS;
      goto error_stop;
    }

    /* Check for FIFO half-empty and write data. */
    if (sta & SDIO_STA_TXFIFOHE) {
      uint32_t i;
      for (i = 0U; i < SDIO_HALFIFO; i++) {
        sdcp->sdio->FIFO = *(const uint32_t *)buf;
        buf += 4U;
      }
    }

    if (--timeout == 0U) {
      sdcp->sdio->DCTRL = 0U;
      sdcp->sdio->MASK  = 0U;
      sdcp->sdio->ICR   = SDIO_ICR_ALL_FLAGS;
      sdcp->errors |= SDC_DATA_TIMEOUT;
      goto error;
    }
  }

done:
  /* For multi-block writes, send stop command. */
  if (blocks > 1U) {
    return sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_STOP_TRANSMISSION,
                                      0, resp);
  }
  return HAL_SUCCESS;

error_stop:
  if (blocks > 1U) {
    sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_STOP_TRANSMISSION, 0, resp);
  }

error:
  return HAL_FAILED;
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

  (void)sdcp;
  return HAL_SUCCESS;
}

#endif /* HAL_USE_SDC == TRUE */

/** @} */
