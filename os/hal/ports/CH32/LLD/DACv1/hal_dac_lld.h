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
 * @file    hal_dac_lld.h
 * @brief   CH32 DAC subsystem low level driver header.
 *
 * @addtogroup DAC
 * @{
 */

#ifndef HAL_DAC_LLD_H
#define HAL_DAC_LLD_H

#if (HAL_USE_DAC == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Maximum number of DAC channels per unit.
 */
#define DAC_MAX_CHANNELS                    2

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   DAC1 CH1 driver enable switch.
 * @details If set to @p TRUE the support for DAC1 channel 1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_DAC_USE_DAC1_CH1) || defined(__DOXYGEN__)
#define CH32_DAC_USE_DAC1_CH1               FALSE
#endif

/**
 * @brief   DAC1 CH2 driver enable switch.
 * @details If set to @p TRUE the support for DAC1 channel 2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_DAC_USE_DAC1_CH2) || defined(__DOXYGEN__)
#define CH32_DAC_USE_DAC1_CH2               FALSE
#endif

/**
 * @brief   DAC1 CH1 DMA priority (0..3|lowest..highest).
 */
#if !defined(CH32_DAC_DAC1_CH1_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_DAC_DAC1_CH1_DMA_PRIORITY      2
#endif

/**
 * @brief   DAC1 CH2 DMA priority (0..3|lowest..highest).
 */
#if !defined(CH32_DAC_DAC1_CH2_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_DAC_DAC1_CH2_DMA_PRIORITY      2
#endif

/**
 * @brief   DAC1 CH1 DMA interrupt priority level setting.
 */
#if !defined(CH32_DAC_DAC1_CH1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_DAC_DAC1_CH1_IRQ_PRIORITY      5
#endif

/**
 * @brief   DAC1 CH2 DMA interrupt priority level setting.
 */
#if !defined(CH32_DAC_DAC1_CH2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_DAC_DAC1_CH2_IRQ_PRIORITY      5
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/* Registry checks.*/
#if !defined(CH32_HAS_DAC1_CH1)
#error "CH32_HAS_DAC1_CH1 not defined in registry"
#endif

#if !defined(CH32_HAS_DAC1_CH2)
#error "CH32_HAS_DAC1_CH2 not defined in registry"
#endif

#if (CH32_DAC_USE_DAC1_CH1 == TRUE) && (CH32_HAS_DAC1_CH1 == FALSE)
#error "DAC1 CH1 not present in the selected device"
#endif

#if (CH32_DAC_USE_DAC1_CH2 == TRUE) && (CH32_HAS_DAC1_CH2 == FALSE)
#error "DAC1 CH2 not present in the selected device"
#endif

#if !CH32_DAC_USE_DAC1_CH1 && !CH32_DAC_USE_DAC1_CH2
#error "DAC driver activated but no DAC peripheral assigned"
#endif

#if CH32_DAC_USE_DAC1_CH1 &&                                              \
    !OSAL_IRQ_IS_VALID_PRIORITY(CH32_DAC_DAC1_CH1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to DAC1 CH1"
#endif

#if CH32_DAC_USE_DAC1_CH2 &&                                              \
    !OSAL_IRQ_IS_VALID_PRIORITY(CH32_DAC_DAC1_CH2_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to DAC1 CH2"
#endif

#if CH32_DAC_USE_DAC1_CH1 &&                                              \
    !CH32_DMA_IS_VALID_PRIORITY(CH32_DAC_DAC1_CH1_DMA_PRIORITY)
#error "Invalid DMA priority assigned to DAC1 CH1"
#endif

#if CH32_DAC_USE_DAC1_CH2 &&                                              \
    !CH32_DMA_IS_VALID_PRIORITY(CH32_DAC_DAC1_CH2_DMA_PRIORITY)
#error "Invalid DMA priority assigned to DAC1 CH2"
#endif

/* DMA is needed if any DAC channel is used.*/
#if (CH32_DAC_USE_DAC1_CH1 == TRUE) || (CH32_DAC_USE_DAC1_CH2 == TRUE)
#define CH32_DMA_REQUIRED                  TRUE
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a DAC channel index.
 */
typedef uint32_t dacchannel_t;

/**
 * @brief   Type representing a DAC sample.
 */
typedef uint16_t dacsample_t;

/**
 * @brief   DAC channel parameters type.
 */
typedef struct {
  /**
   * @brief   DAC channel number (0 for CH1, 1 for CH2).
   */
  uint32_t                  channel;
  /**
   * @brief   Associated DMA stream.
   */
  uint32_t                  dmastream;
  /**
   * @brief   DMAMUX peripheral selector.
   */
  uint32_t                  dmamux;
  /**
   * @brief   DMA mode bits.
   */
  uint32_t                  dmamode;
  /**
   * @brief   DMA channel IRQ priority.
   */
  uint32_t                  dmairqprio;
} dacparams_t;

/**
 * @brief   Possible DAC failure causes.
 * @note    Error codes are architecture dependent and should not relied
 *          upon.
 */
typedef enum {
  DAC_ERR_DMAFAILURE = 0,                   /**< DMA operations failure.    */
  DAC_ERR_UNDERFLOW = 1                     /**< DAC overflow condition.    */
} dacerror_t;

/**
 * @brief   Samples alignment and size mode.
 */
typedef enum {
  DAC_DHRM_12BIT_RIGHT = 0,
  DAC_DHRM_12BIT_LEFT  = 1,
  DAC_DHRM_8BIT_RIGHT  = 2
} dacdhrmode_t;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Low level fields of the DAC driver structure.
 */
#define dac_lld_driver_fields                                               \
  /* DAC channel parameters.*/                                              \
  const dacparams_t         *params;                                        \
  /* Associated DMA stream.*/                                                \
  const ch32_dma_stream_t   *dma;

/**
 * @brief   Low level fields of the DAC configuration structure.
 */
#define dac_lld_config_fields                                               \
  /* Initial output value on DAC channel.*/                                 \
  uint32_t                  init;                                           \
  /* DAC data holding register mode.*/                                      \
  dacdhrmode_t              datamode;                                       \
  /* DAC control register.*/                                                \
  uint32_t                  cr;

/**
 * @brief   Low level fields of the DAC group configuration structure.
 */
#define dac_lld_conversion_group_fields                                     \
  /* Dummy configuration, it is not needed.*/                               \
  uint32_t                  dummy;

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (CH32_DAC_USE_DAC1_CH1 == TRUE) && !defined(__DOXYGEN__)
extern DACDriver DACD1;
#endif

#if (CH32_DAC_USE_DAC1_CH2 == TRUE) && !defined(__DOXYGEN__)
extern DACDriver DACD2;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void dac_lld_init(void);
  void dac_lld_start(DACDriver *dacp);
  void dac_lld_stop(DACDriver *dacp);
  void dac_lld_put_channel(DACDriver *dacp,
                           dacchannel_t channel,
                           dacsample_t sample);
  void dac_lld_start_conversion(DACDriver *dacp);
  void dac_lld_stop_conversion(DACDriver *dacp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_DAC == TRUE */

#endif /* HAL_DAC_LLD_H */

/** @} */
