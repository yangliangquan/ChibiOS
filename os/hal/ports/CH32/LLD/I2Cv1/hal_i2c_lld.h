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
 * @file    hal_i2c_lld.h
 * @brief   CH32 I2C (v1) subsystem low level driver header.
 *
 * @addtogroup I2C
 * @{
 */

#ifndef HAL_I2C_LLD_H
#define HAL_I2C_LLD_H

#include "ch32_dma.h"

#if (HAL_USE_I2C == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   I2C error flags extension.
 */
#define I2C_ERR_BERR            I2C_BUS_ERROR
#define I2C_ERR_ARLO            I2C_ARBITRATION_LOST
#define I2C_ERR_AF              I2C_ACK_FAILURE
#define I2C_ERR_OVR             I2C_OVERRUN
#define I2C_ERR_TIMEOUT         I2C_TIMEOUT
#define I2C_ERR_DMA             0x40U
#define I2C_ERR_BUSY            0x80U

/**
 * @brief   Low level fields of the I2C driver structure.
 */
#define I2C_DRIVER_EXT_FIELDS                                                 \
  /* I2C peripheral register base address.*/                                  \
  I2C_TypeDef               *i2c;                                             \
  /* RX DMA stream pointer.*/                                                 \
  const ch32_dma_stream_t   *dmarx;                                           \
  /* TX DMA stream pointer.*/                                                 \
  const ch32_dma_stream_t   *dmatx;                                           \
  /* RX DMA mode value.*/                                                     \
  uint32_t                  rxdmamode;                                        \
  /* TX DMA mode value.*/                                                     \
  uint32_t                  txdmamode;                                        \
  /* Waiting thread reference.*/                                              \
  thread_reference_t        thread;                                           \
  /* Slave address (bit 15=1 for 10-bit mode).*/                              \
  i2caddr_t                 addr;                                             \
  /* Transmit buffer pointer.*/                                               \
  const uint8_t             *txbuf;                                           \
  /* Receive buffer pointer.*/                                                \
  uint8_t                   *rxbuf;                                           \
  /* Transmit bytes remaining.*/                                              \
  size_t                    txbytes;                                          \
  /* Receive bytes remaining.*/                                               \
  size_t                    rxbytes;

/**
 * @brief   Supported duty cycle modes for the I2C bus.
 */
typedef enum {
  STD_DUTY_CYCLE = 1,
  FAST_DUTY_CYCLE_2 = 2,
  FAST_DUTY_CYCLE_16_9 = 3,
} i2cdutycycle_t;

/**
 * @brief   Supported modes for the I2C bus.
 */
typedef enum {
  OPMODE_I2C = 1,
  OPMODE_SMBUS_DEVICE = 2,
  OPMODE_SMBUS_HOST = 3,
} i2copmode_t;
/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    CH32 configuration options
 * @{
 */
/**
 * @brief   I2C1 driver enable switch.
 * @details If set to @p TRUE the support for I2C1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_I2C_USE_I2C1) || defined(__DOXYGEN__)
#define CH32_I2C_USE_I2C1                  FALSE
#endif

/**
 * @brief   I2C2 driver enable switch.
 * @details If set to @p TRUE the support for I2C2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_I2C_USE_I2C2) || defined(__DOXYGEN__)
#define CH32_I2C_USE_I2C2                  FALSE
#endif

/**
 * @brief   I2C3 driver enable switch.
 * @details If set to @p TRUE the support for I2C3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_I2C_USE_I2C3) || defined(__DOXYGEN__)
#define CH32_I2C_USE_I2C3                  FALSE
#endif

/**
 * @brief   I2C4 driver enable switch.
 * @details If set to @p TRUE the support for I2C4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_I2C_USE_I2C4) || defined(__DOXYGEN__)
#define CH32_I2C_USE_I2C4                  FALSE
#endif

/**
 * @brief   I2C1 DMA priority.
 * @note    The default is @p 1.
 */
#if !defined(CH32_I2C_I2C1_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_I2C_I2C1_DMA_PRIORITY         1
#endif

/**
 * @brief   I2C2 DMA priority.
 * @note    The default is @p 1.
 */
#if !defined(CH32_I2C_I2C2_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_I2C_I2C2_DMA_PRIORITY         1
#endif

/**
 * @brief   I2C3 DMA priority.
 * @note    The default is @p 1.
 */
#if !defined(CH32_I2C_I2C3_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_I2C_I2C3_DMA_PRIORITY         1
#endif

/**
 * @brief   I2C4 DMA priority.
 * @note    The default is @p 1.
 */
#if !defined(CH32_I2C_I2C4_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_I2C_I2C4_DMA_PRIORITY         1
#endif

/**
 * @brief   I2C1 interrupt priority.
 */
#if !defined(CH32_I2C_I2C1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_I2C_I2C1_IRQ_PRIORITY         10
#endif

/**
 * @brief   I2C2 interrupt priority.
 */
#if !defined(CH32_I2C_I2C2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_I2C_I2C2_IRQ_PRIORITY         10
#endif

/**
 * @brief   I2C3 interrupt priority.
 */
#if !defined(CH32_I2C_I2C3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_I2C_I2C3_IRQ_PRIORITY         10
#endif

/**
 * @brief   I2C4 interrupt priority.
 */
#if !defined(CH32_I2C_I2C4_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_I2C_I2C4_IRQ_PRIORITY         10
#endif

/**
 * @brief   I2C1 RX DMA stream ID.
 */
#if !defined(CH32_I2C_I2C1_RX_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_I2C_I2C1_RX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   I2C1 TX DMA stream ID.
 */
#if !defined(CH32_I2C_I2C1_TX_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_I2C_I2C1_TX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   I2C2 RX DMA stream ID.
 */
#if !defined(CH32_I2C_I2C2_RX_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_I2C_I2C2_RX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   I2C2 TX DMA stream ID.
 */
#if !defined(CH32_I2C_I2C2_TX_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_I2C_I2C2_TX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   I2C3 RX DMA stream ID.
 */
#if !defined(CH32_I2C_I2C3_RX_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_I2C_I2C3_RX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   I2C3 TX DMA stream ID.
 */
#if !defined(CH32_I2C_I2C3_TX_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_I2C_I2C3_TX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   I2C4 RX DMA stream ID.
 */
#if !defined(CH32_I2C_I2C4_RX_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_I2C_I2C4_RX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   I2C4 TX DMA stream ID.
 */
#if !defined(CH32_I2C_I2C4_TX_DMA_STREAM) || defined(__DOXYGEN__)
#define CH32_I2C_I2C4_TX_DMA_STREAM        CH32_DMA_STREAM_ID_ANY
#endif

/**
 * @brief   Bus busy timeout in milliseconds.
 */
#if !defined(CH32_I2C_BUSY_TIMEOUT) || defined(__DOXYGEN__)
#define CH32_I2C_BUSY_TIMEOUT              500
#endif

/**
 * @brief   DMA error hook.
 */
#if !defined(CH32_I2C_DMA_ERROR_HOOK) || defined(__DOXYGEN__)
#define CH32_I2C_DMA_ERROR_HOOK(i2cp)                                         \
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

/**
 * @brief   Type representing an I2C address.
 */
typedef uint16_t i2caddr_t;

/**
 * @brief   Type of I2C Driver condition flags.
 */
typedef uint32_t i2cflags_t;

/**
 * @brief   I2C driver configuration structure.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
struct hal_i2c_config {
  /* End of the mandatory fields.*/
  uint32_t                  clock_speed;
  uint32_t                  duty_cycle;
  uint32_t                  op_mode;
};

/**
 * @brief   Type of a structure representing an I2C configuration.
 */
typedef struct hal_i2c_config I2CConfig;

/**
 * @brief   Type of a structure representing an I2C driver.
 */
typedef struct hal_i2c_driver I2CDriver;

/**
 * @brief   Structure representing an I2C driver.
 */
struct hal_i2c_driver {
  /**
   * @brief   Driver state.
   */
  i2cstate_t                state;
  /**
   * @brief   Current configuration data.
   */
  const I2CConfig           *config;
  /**
   * @brief   Error flags.
   */
  i2cflags_t                errors;
#if (I2C_USE_MUTUAL_EXCLUSION == TRUE) || defined(__DOXYGEN__)
  mutex_t                   mutex;
#endif
#if defined(I2C_DRIVER_EXT_FIELDS)
  I2C_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Get errors from I2C driver.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
#define i2c_lld_get_errors(i2cp) ((i2cp)->errors)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (CH32_I2C_USE_I2C1 == TRUE) && !defined(__DOXYGEN__)
extern I2CDriver I2CD1;
#endif

#if (CH32_I2C_USE_I2C2 == TRUE) && !defined(__DOXYGEN__)
extern I2CDriver I2CD2;
#endif

#if (CH32_I2C_USE_I2C3 == TRUE) && !defined(__DOXYGEN__)
extern I2CDriver I2CD3;
#endif

#if (CH32_I2C_USE_I2C4 == TRUE) && !defined(__DOXYGEN__)
extern I2CDriver I2CD4;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void i2c_lld_init(void);
  void i2c_lld_start(I2CDriver *i2cp);
  void i2c_lld_stop(I2CDriver *i2cp);
  msg_t i2c_lld_master_transmit_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                        const uint8_t *txbuf, size_t txbytes,
                                        uint8_t *rxbuf, size_t rxbytes,
                                        sysinterval_t timeout);
  msg_t i2c_lld_master_receive_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                       uint8_t *rxbuf, size_t rxbytes,
                                       sysinterval_t timeout);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_I2C == TRUE */

#endif /* HAL_I2C_LLD_H */

/** @} */
