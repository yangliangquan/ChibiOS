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
 * @file    SDIOv1/hal_sdc_lld.h
 * @brief   CH32 SDC subsystem low level driver header.
 *
 * @addtogroup SDC
 * @{
 */

#ifndef HAL_SDC_LLD_H
#define HAL_SDC_LLD_H

#if HAL_USE_SDC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*
 * SDIO register bit definitions.
 * CH32 CMSIS header only provides the register structure, not the bit
 * definitions. These are defined here for driver use.
 */

/* POWER register */
#define SDIO_POWER_PWRCTRL_0                ((uint32_t)0x00000001U)
#define SDIO_POWER_PWRCTRL_1                ((uint32_t)0x00000002U)

/* CLKCR register */
#define SDIO_CLKCR_CLKDIV_Msk               ((uint32_t)0x000000FFU)
#define SDIO_CLKCR_CLKEN                    ((uint32_t)0x00000100U)
#define SDIO_CLKCR_PWRSAV                   ((uint32_t)0x00000200U)  /* bit 9 */
#define SDIO_CLKCR_BYPASS                   ((uint32_t)0x00000400U)  /* bit 10 */
#define SDIO_CLKCR_WIDBUS                   ((uint32_t)0x00001800U)
#define SDIO_CLKCR_WIDBUS_0                 ((uint32_t)0x00000800U)
#define SDIO_CLKCR_WIDBUS_1                 ((uint32_t)0x00001000U)
#define SDIO_CLKCR_NEGEDGE                  ((uint32_t)0x00002000U)
#define SDIO_CLKCR_HWFC_EN                  ((uint32_t)0x00004000U)

/* CMD register (CH32H417 layout — different from STM32!) */
#define SDIO_CMD_CMDINDEX_Msk               ((uint32_t)0x0000003FU)  /* bits 0-5 */
#define SDIO_CMD_WAITRESP_0                 ((uint32_t)0x00000040U)  /* bit 6: Response_Short */
#define SDIO_CMD_WAITRESP_1                 ((uint32_t)0x00000080U)  /* bit 7: (Long=0xC0) */
#define SDIO_CMD_WAITINT                    ((uint32_t)0x00000100U)  /* bit 8: Wait_IT */
#define SDIO_CMD_WAITPEND                   ((uint32_t)0x00000200U)  /* bit 9: Wait_Pend */
#define SDIO_CMD_CPSMEN                     ((uint32_t)0x00000400U)  /* bit 10: CPSM Enable */

/* STA register */
#define SDIO_STA_CCRCFAIL                   ((uint32_t)0x00000001U)
#define SDIO_STA_DCRCFAIL                   ((uint32_t)0x00000002U)
#define SDIO_STA_CTIMEOUT                   ((uint32_t)0x00000004U)
#define SDIO_STA_DTIMEOUT                   ((uint32_t)0x00000008U)
#define SDIO_STA_TXUNDERR                   ((uint32_t)0x00000010U)
#define SDIO_STA_RXOVERR                    ((uint32_t)0x00000020U)
#define SDIO_STA_CMDREND                    ((uint32_t)0x00000040U)
#define SDIO_STA_CMDSENT                    ((uint32_t)0x00000080U)
#define SDIO_STA_DATAEND                    ((uint32_t)0x00000100U)
#define SDIO_STA_STBITERR                   ((uint32_t)0x00000200U)
#define SDIO_STA_DBCKEND                    ((uint32_t)0x00000400U)
#define SDIO_STA_CMDACT                     ((uint32_t)0x00000800U)
#define SDIO_STA_TXACT                      ((uint32_t)0x00001000U)
#define SDIO_STA_RXACT                      ((uint32_t)0x00002000U)
#define SDIO_STA_TXFIFOHE                   ((uint32_t)0x00004000U)
#define SDIO_STA_RXFIFOHF                   ((uint32_t)0x00008000U)
#define SDIO_STA_TXFIFOF                    ((uint32_t)0x00010000U)
#define SDIO_STA_RXFIFOF                    ((uint32_t)0x00020000U)
#define SDIO_STA_TXFIFOE                    ((uint32_t)0x00040000U)
#define SDIO_STA_RXFIFOE                    ((uint32_t)0x00080000U)
#define SDIO_STA_TXDAVL                     ((uint32_t)0x00100000U)
#define SDIO_STA_RXDAVL                     ((uint32_t)0x00200000U)
#define SDIO_STA_SDIOIT                     ((uint32_t)0x00400000U)
#define SDIO_STA_CEATAEND                   ((uint32_t)0x00800000U)

/* Error flags mask */
#define SDIO_STA_ERROR_MASK                                                 \
  (SDIO_STA_CCRCFAIL | SDIO_STA_DCRCFAIL |                                  \
   SDIO_STA_CTIMEOUT | SDIO_STA_DTIMEOUT |                                  \
   SDIO_STA_TXUNDERR | SDIO_STA_RXOVERR  |                                  \
   SDIO_STA_STBITERR)

/* DCTRL register */
#define SDIO_DCTRL_DTEN                     ((uint32_t)0x00000001U)
#define SDIO_DCTRL_DTDIR                    ((uint32_t)0x00000002U)
#define SDIO_DCTRL_DTMODE                   ((uint32_t)0x00000004U)
#define SDIO_DCTRL_DMAEN                    ((uint32_t)0x00000008U)
#define SDIO_DCTRL_DBLOCKSIZE_Msk           ((uint32_t)0x000000F0U)
#define SDIO_DCTRL_DBLOCKSIZE_0             ((uint32_t)0x00000010U)
#define SDIO_DCTRL_DBLOCKSIZE_1             ((uint32_t)0x00000020U)
#define SDIO_DCTRL_DBLOCKSIZE_2             ((uint32_t)0x00000040U)
#define SDIO_DCTRL_DBLOCKSIZE_3             ((uint32_t)0x00000080U)
#define SDIO_DCTRL_RWSTART                  ((uint32_t)0x00000100U)
#define SDIO_DCTRL_RWSTOP                   ((uint32_t)0x00000200U)
#define SDIO_DCTRL_SDIOEN                   ((uint32_t)0x00000800U)

/* ICR register bits (same as STA) */
#define SDIO_ICR_CCRCFAILC                  ((uint32_t)0x00000001U)
#define SDIO_ICR_DCRCFAILC                  ((uint32_t)0x00000002U)
#define SDIO_ICR_CTIMEOUTC                  ((uint32_t)0x00000004U)
#define SDIO_ICR_DTIMEOUTC                  ((uint32_t)0x00000008U)
#define SDIO_ICR_TXUNDERRC                  ((uint32_t)0x00000010U)
#define SDIO_ICR_RXOVERRC                   ((uint32_t)0x00000020U)
#define SDIO_ICR_CMDRENDC                   ((uint32_t)0x00000040U)
#define SDIO_ICR_CMDSENTC                   ((uint32_t)0x00000080U)
#define SDIO_ICR_DATAENDC                   ((uint32_t)0x00000100U)
#define SDIO_ICR_STBITERRC                  ((uint32_t)0x00000200U)
#define SDIO_ICR_DBCKENDC                   ((uint32_t)0x00000400U)

#define SDIO_ICR_ALL_FLAGS                  ((uint32_t)0xFFFFFFFFU)

/* MASK register bits (same as STA) */
#define SDIO_MASK_CCRCFAILIE                ((uint32_t)0x00000001U)
#define SDIO_MASK_DCRCFAILIE                ((uint32_t)0x00000002U)
#define SDIO_MASK_CTIMEOUTIE                ((uint32_t)0x00000004U)
#define SDIO_MASK_DTIMEOUTIE                ((uint32_t)0x00000008U)
#define SDIO_MASK_TXUNDERRIE                ((uint32_t)0x00000010U)
#define SDIO_MASK_RXOVERRIE                 ((uint32_t)0x00000020U)
#define SDIO_MASK_CMDRENDIE                 ((uint32_t)0x00000040U)
#define SDIO_MASK_CMDSENTIE                 ((uint32_t)0x00000080U)
#define SDIO_MASK_DATAENDIE                 ((uint32_t)0x00000100U)
#define SDIO_MASK_STBITERRIE                ((uint32_t)0x00000200U)
#define SDIO_MASK_DBCKENDIE                 ((uint32_t)0x00000400U)

/* FIFO threshold: half FIFO = 8 words (32 bytes) */
#define SDIO_HALFIFO                        8U
#define SDIO_HALFIFO_BYTES                  32U

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   SDC1 driver enable switch.
 * @details If set to @p TRUE the support for SDC1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SDC_USE_SDC1) || defined(__DOXYGEN__)
#define CH32_SDC_USE_SDC1                   FALSE
#endif

/**
 * @brief   Write timeout in milliseconds.
 */
#if !defined(CH32_SDC_WRITE_TIMEOUT) || defined(__DOXYGEN__)
#define CH32_SDC_WRITE_TIMEOUT              10000
#endif

/**
 * @brief   Read timeout in milliseconds.
 */
#if !defined(CH32_SDC_READ_TIMEOUT) || defined(__DOXYGEN__)
#define CH32_SDC_READ_TIMEOUT               10000
#endif

/**
 * @brief   Card clock activation delay in milliseconds.
 */
#if !defined(CH32_SDC_CLOCK_DELAY) || defined(__DOXYGEN__)
#define CH32_SDC_CLOCK_DELAY                10
#endif

/**
 * @brief   Support for unaligned transfers.
 * @note    Unaligned transfers are much slower because a bounce buffer
 *          is used.
 */
#if !defined(CH32_SDC_UNALIGNED_SUPPORT) || defined(__DOXYGEN__)
#define CH32_SDC_UNALIGNED_SUPPORT          TRUE
#endif

/**
 * @brief   SDIO IRQ priority level setting.
 */
#if !defined(CH32_SDC_SDIO_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_SDC_SDIO_IRQ_PRIORITY          9
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !CH32_HAS_SDIO1
#error "SDIO not present in the selected device"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of card flags.
 */
typedef uint32_t sdcmode_t;

/**
 * @brief   SDC Driver condition flags type.
 */
typedef uint32_t sdcflags_t;

/**
 * @brief   Type of a structure representing an SDC driver.
 */
typedef struct SDCDriver SDCDriver;

/**
 * @brief   Driver configuration structure.
 */
typedef struct {
  /**
   * @brief   Bus width.
   */
  sdcbusmode_t  bus_width;
  /* End of the mandatory fields.*/
} SDCConfig;

/**
 * @brief   @p SDCDriver specific methods.
 */
#define _sdc_driver_methods                                                 \
  _mmcsd_block_device_methods

/**
 * @extends MMCSDBlockDeviceVMT
 *
 * @brief   @p SDCDriver virtual methods table.
 */
struct SDCDriverVMT {
  _sdc_driver_methods
};

/**
 * @brief   Structure representing an SDC driver.
 */
struct SDCDriver {
  /**
   * @brief Virtual Methods Table.
   */
  const struct SDCDriverVMT *vmt;
  _mmcsd_block_device_data
  /**
   * @brief Current configuration data.
   */
  const SDCConfig           *config;
  /**
   * @brief Various flags regarding the mounted card.
   */
  sdcmode_t                 cardmode;
  /**
   * @brief Errors flags.
   */
  sdcflags_t                errors;
  /**
   * @brief Card RCA.
   */
  uint32_t                  rca;
  /**
   * @brief   Pointer to the SDIO registers block.
   */
  SDIO_TypeDef              *sdio;
  /**
   * @brief   SDIO peripheral clock frequency.
   */
  uint32_t                  clkfreq;
  /**
   * @brief   Transaction thread reference, used in I-class APIs.
   */
  thread_reference_t        thread;
  /* End of the mandatory fields.*/
  /**
   * @brief   Buffer for internal operations (bounce buffer).
   */
  uint8_t                   *buf;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (CH32_SDC_USE_SDC1 == TRUE) && !defined(__DOXYGEN__)
extern SDCDriver SDCD1;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void sdc_lld_init(void);
  void sdc_lld_start(SDCDriver *sdcp);
  void sdc_lld_stop(SDCDriver *sdcp);
  void sdc_lld_start_clk(SDCDriver *sdcp);
  void sdc_lld_set_data_clk(SDCDriver *sdcp, sdcbusclk_t clk);
  void sdc_lld_stop_clk(SDCDriver *sdcp);
  void sdc_lld_set_bus_mode(SDCDriver *sdcp, sdcbusmode_t mode);
  void sdc_lld_send_cmd_none(SDCDriver *sdcp, uint8_t cmd, uint32_t arg);
  bool sdc_lld_send_cmd_short(SDCDriver *sdcp, uint8_t cmd, uint32_t arg,
                              uint32_t *resp);
  bool sdc_lld_send_cmd_short_crc(SDCDriver *sdcp, uint8_t cmd, uint32_t arg,
                                  uint32_t *resp);
  bool sdc_lld_send_cmd_long_crc(SDCDriver *sdcp, uint8_t cmd, uint32_t arg,
                                 uint32_t *resp);
  bool sdc_lld_read_special(SDCDriver *sdcp, uint8_t *buf, size_t bytes,
                            uint8_t cmd, uint32_t argument);
  bool sdc_lld_read(SDCDriver *sdcp, uint32_t startblk,
                    uint8_t *buf, uint32_t n);
  bool sdc_lld_write(SDCDriver *sdcp, uint32_t startblk,
                     const uint8_t *buf, uint32_t n);
  bool sdc_lld_sync(SDCDriver *sdcp);
  bool sdc_lld_is_card_inserted(SDCDriver *sdcp);
  bool sdc_lld_is_write_protected(SDCDriver *sdcp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SDC */

#endif /* HAL_SDC_LLD_H */

/** @} */
