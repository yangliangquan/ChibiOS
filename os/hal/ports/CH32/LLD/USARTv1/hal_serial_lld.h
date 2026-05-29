/*
    ChibiOS - Copyright (C) 2006-2026 Giovanni Di Sirio.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at:

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    USARTv1/hal_serial_lld.h
 * @brief   CH32 serial subsystem low level driver header.
 *
 * @addtogroup SERIAL
 * @{
 */

#ifndef HAL_SERIAL_LLD_H
#define HAL_SERIAL_LLD_H

#if (HAL_USE_SERIAL == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    CTLR1 register helper macros
 * @{
 */
#define USART_CTLR1_DATA7                     (USART_CTLR1_M_EXT_0)
#define USART_CTLR1_DATA8                     (0U)
#define USART_CTLR1_DATA9                     (USART_CTLR1_M)
/** @} */

/**
 * @name    CTLR2 register helper macros
 * @{
 */
#define USART_CTLR2_STOP1_BITS                (0U << 12)
#define USART_CTLR2_STOP0P5_BITS              (1U << 12)
#define USART_CTLR2_STOP2_BITS                (2U << 12)
#define USART_CTLR2_STOP1P5_BITS              (3U << 12)
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    CH32 configuration options
 * @{
 */
/**
 * @brief   Serial driver 1 enable switch.
 * @details If set to @p TRUE the support for USART1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SERIAL_USE_USART1) || defined(__DOXYGEN__)
#define CH32_SERIAL_USE_USART1             FALSE
#endif

/**
 * @brief   Serial driver 2 enable switch.
 * @details If set to @p TRUE the support for USART2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SERIAL_USE_USART2) || defined(__DOXYGEN__)
#define CH32_SERIAL_USE_USART2             FALSE
#endif

/**
 * @brief   Serial driver 3 enable switch.
 * @details If set to @p TRUE the support for USART3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SERIAL_USE_USART3) || defined(__DOXYGEN__)
#define CH32_SERIAL_USE_USART3             FALSE
#endif

/**
 * @brief   Serial driver 4 enable switch.
 * @details If set to @p TRUE the support for USART4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SERIAL_USE_USART4) || defined(__DOXYGEN__)
#define CH32_SERIAL_USE_USART4             FALSE
#endif

/**
 * @brief   Serial driver 5 enable switch.
 * @details If set to @p TRUE the support for USART5 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SERIAL_USE_USART5) || defined(__DOXYGEN__)
#define CH32_SERIAL_USE_USART5             FALSE
#endif

/**
 * @brief   Serial driver 6 enable switch.
 * @details If set to @p TRUE the support for USART6 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SERIAL_USE_USART6) || defined(__DOXYGEN__)
#define CH32_SERIAL_USE_USART6             FALSE
#endif

/**
 * @brief   Serial driver 7 enable switch.
 * @details If set to @p TRUE the support for USART7 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SERIAL_USE_USART7) || defined(__DOXYGEN__)
#define CH32_SERIAL_USE_USART7             FALSE
#endif

/**
 * @brief   Serial driver 8 enable switch.
 * @details If set to @p TRUE the support for USART8 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SERIAL_USE_USART8) || defined(__DOXYGEN__)
#define CH32_SERIAL_USE_USART8             FALSE
#endif
/** @} */

/**
 * @name    CH32 IRQ priority settings
 * @{
 */
/**
 * @brief   USART1 interrupt priority level setting.
 */
#if !defined(CH32_SERIAL_USART1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_SERIAL_USART1_IRQ_PRIORITY    12
#endif

/**
 * @brief   USART2 interrupt priority level setting.
 */
#if !defined(CH32_SERIAL_USART2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_SERIAL_USART2_IRQ_PRIORITY    12
#endif

/**
 * @brief   USART3 interrupt priority level setting.
 */
#if !defined(CH32_SERIAL_USART3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_SERIAL_USART3_IRQ_PRIORITY    12
#endif

/**
 * @brief   USART4 interrupt priority level setting.
 */
#if !defined(CH32_SERIAL_USART4_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_SERIAL_USART4_IRQ_PRIORITY    12
#endif

/**
 * @brief   USART5 interrupt priority level setting.
 */
#if !defined(CH32_SERIAL_USART5_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_SERIAL_USART5_IRQ_PRIORITY    12
#endif

/**
 * @brief   USART6 interrupt priority level setting.
 */
#if !defined(CH32_SERIAL_USART6_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_SERIAL_USART6_IRQ_PRIORITY    12
#endif

/**
 * @brief   USART7 interrupt priority level setting.
 */
#if !defined(CH32_SERIAL_USART7_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_SERIAL_USART7_IRQ_PRIORITY    12
#endif

/**
 * @brief   USART8 interrupt priority level setting.
 */
#if !defined(CH32_SERIAL_USART8_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_SERIAL_USART8_IRQ_PRIORITY    12
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !CH32_SERIAL_USE_USART1 && !CH32_SERIAL_USE_USART2 &&         \
    !CH32_SERIAL_USE_USART3 && !CH32_SERIAL_USE_USART4  &&         \
    !CH32_SERIAL_USE_USART5 && !CH32_SERIAL_USE_USART6  &&         \
    !CH32_SERIAL_USE_USART7 && !CH32_SERIAL_USE_USART8
#error "SERIAL driver activated but no USART peripheral assigned"
#endif

/* Check for exclusive USART allocation.*/
#if CH32_SERIAL_USE_USART1
#if defined(CH32_USART1_IS_USED)
#error "SD1 requires USART1 but it is already used"
#else
#define CH32_USART1_IS_USED
#endif
#endif

#if CH32_SERIAL_USE_USART2
#if defined(CH32_USART2_IS_USED)
#error "SD2 requires USART2 but it is already used"
#else
#define CH32_USART2_IS_USED
#endif
#endif

#if CH32_SERIAL_USE_USART3
#if defined(CH32_USART3_IS_USED)
#error "SD3 requires USART3 but it is already used"
#else
#define CH32_USART3_IS_USED
#endif
#endif

#if CH32_SERIAL_USE_USART4
#if defined(CH32_USART4_IS_USED)
#error "SD4 requires USART4 but it is already used"
#else
#define CH32_USART4_IS_USED
#endif
#endif

#if CH32_SERIAL_USE_USART5
#if defined(CH32_USART5_IS_USED)
#error "SD5 requires USART5 but it is already used"
#else
#define CH32_USART5_IS_USED
#endif
#endif

#if CH32_SERIAL_USE_USART6
#if defined(CH32_USART6_IS_USED)
#error "SD6 requires USART6 but it is already used"
#else
#define CH32_USART6_IS_USED
#endif
#endif

#if CH32_SERIAL_USE_USART7
#if defined(CH32_USART7_IS_USED)
#error "SD7 requires USART7 but it is already used"
#else
#define CH32_USART7_IS_USED
#endif
#endif

#if CH32_SERIAL_USE_USART8
#if defined(CH32_USART8_IS_USED)
#error "SD8 requires USART8 but it is already used"
#else
#define CH32_USART8_IS_USED
#endif
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   CH32 Serial Driver configuration structure.
 * @details An instance of this structure must be passed to @p sdStart()
 *          in order to configure and start a serial driver operations.
 * @note    This structure content is architecture dependent, each driver
 *          implementation defines its own version and the custom static
 *          initializers.
 */
typedef struct hal_serial_config {
  /**
   * @brief Bit rate.
   */
  uint32_t                  baud;
  /* End of the mandatory fields.*/
  /**
   * @brief Initialization value for the CTLR1 register.
   */
  uint32_t                  cr1;
  /**
   * @brief Initialization value for the CTLR2 register.
   */
  uint32_t                  cr2;
  /**
   * @brief Initialization value for the CTLR3 register.
   */
  uint32_t                  cr3;
} SerialConfig;

/**
 * @brief   @p SerialDriver specific data.
 */
#define _serial_driver_data                                                 \
  _base_asynchronous_channel_data                                           \
  sdstate_t                 state;                                          \
  input_queue_t             iqueue;                                         \
  output_queue_t            oqueue;                                         \
  uint8_t                   ib[SERIAL_BUFFERS_SIZE];                        \
  uint8_t                   ob[SERIAL_BUFFERS_SIZE];                        \
  /* End of the mandatory fields.*/                                         \
  USART_TypeDef             *usart;                                         \
  uint32_t                  clock;                                          \
  uint8_t                   rxmask;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (CH32_SERIAL_USE_USART1 == TRUE) && !defined(__DOXYGEN__)
extern SerialDriver SD1;
#endif

#if (CH32_SERIAL_USE_USART2 == TRUE) && !defined(__DOXYGEN__)
extern SerialDriver SD2;
#endif

#if (CH32_SERIAL_USE_USART3 == TRUE) && !defined(__DOXYGEN__)
extern SerialDriver SD3;
#endif

#if (CH32_SERIAL_USE_USART4 == TRUE) && !defined(__DOXYGEN__)
extern SerialDriver SD4;
#endif

#if (CH32_SERIAL_USE_USART5 == TRUE) && !defined(__DOXYGEN__)
extern SerialDriver SD5;
#endif

#if (CH32_SERIAL_USE_USART6 == TRUE) && !defined(__DOXYGEN__)
extern SerialDriver SD6;
#endif

#if (CH32_SERIAL_USE_USART7 == TRUE) && !defined(__DOXYGEN__)
extern SerialDriver SD7;
#endif

#if (CH32_SERIAL_USE_USART8 == TRUE) && !defined(__DOXYGEN__)
extern SerialDriver SD8;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void sd_lld_init(void);
  void sd_lld_start(SerialDriver *sdp, const SerialConfig *config);
  void sd_lld_stop(SerialDriver *sdp);
  void sd_lld_serve_interrupt(SerialDriver *sdp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SERIAL == TRUE */

#endif /* HAL_SERIAL_LLD_H */

/** @} */
