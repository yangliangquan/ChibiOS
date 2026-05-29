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
 * @file    USARTv1/hal_uart_lld.h
 * @brief   CH32 UART subsystem low level driver header.
 *
 * @addtogroup UART
 * @{
 */

#ifndef HAL_UART_LLD_H
#define HAL_UART_LLD_H

#if (HAL_USE_UART == TRUE) || defined(__DOXYGEN__)

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

/**
 * @name    GPR register helper macros
 * @{
 */
#define USART_GPR_PSC_N(n)                    ((uint16_t)(n))
#define USART_GPR_PSC_DIV1                    USART_GPR_PSC_N(0U)
/** @} */

#define UART_DRIVER_EXT_FIELDS                                                                                         \
    USART_TypeDef *usart;                                                                                              \
    uint32_t clock;                                                                                                    \
    uint32_t dmarxmode;                                                                                                \
    uint32_t dmatxmode;                                                                                                \
    const ch32_dma_stream_t *dmarx;                                                                                    \
    const ch32_dma_stream_t *dmatx;                                                                                    \
    volatile uint16_t rxbuf;

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    CH32 configuration options
 * @{
 */
/**
 * @brief   UART driver 1 enable switch.
 * @details If set to @p TRUE the support for USART1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_UART_USE_USART1) || defined(__DOXYGEN__)
#define CH32_UART_USE_USART1                  FALSE
#endif

/**
 * @brief   UART driver 2 enable switch.
 * @details If set to @p TRUE the support for USART2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_UART_USE_USART2) || defined(__DOXYGEN__)
#define CH32_UART_USE_USART2                  FALSE
#endif

/**
 * @brief   UART driver 3 enable switch.
 * @details If set to @p TRUE the support for USART3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_UART_USE_USART3) || defined(__DOXYGEN__)
#define CH32_UART_USE_USART3                  FALSE
#endif

/**
 * @brief   UART driver 4 enable switch.
 * @details If set to @p TRUE the support for USART4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_UART_USE_USART4) || defined(__DOXYGEN__)
#define CH32_UART_USE_USART4                  FALSE
#endif

/**
 * @brief   UART driver 5 enable switch.
 * @details If set to @p TRUE the support for USART5 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_UART_USE_USART5) || defined(__DOXYGEN__)
#define CH32_UART_USE_USART5                  FALSE
#endif

/**
 * @brief   UART driver 6 enable switch.
 * @details If set to @p TRUE the support for USART6 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_UART_USE_USART6) || defined(__DOXYGEN__)
#define CH32_UART_USE_USART6                  FALSE
#endif

/**
 * @brief   UART driver 7 enable switch.
 * @details If set to @p TRUE the support for USART7 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_UART_USE_USART7) || defined(__DOXYGEN__)
#define CH32_UART_USE_USART7                  FALSE
#endif

/**
 * @brief   UART driver 8 enable switch.
 * @details If set to @p TRUE the support for USART8 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_UART_USE_USART8) || defined(__DOXYGEN__)
#define CH32_UART_USE_USART8                  FALSE
#endif

/**
 * @brief   USART1 interrupt priority level setting.
 */
#if !defined(CH32_UART_USART1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_UART_USART1_IRQ_PRIORITY       12
#endif

/**
 * @brief   USART2 interrupt priority level setting.
 */
#if !defined(CH32_UART_USART2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_UART_USART2_IRQ_PRIORITY       12
#endif

/**
 * @brief   USART3 interrupt priority level setting.
 */
#if !defined(CH32_UART_USART3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_UART_USART3_IRQ_PRIORITY       12
#endif

/**
 * @brief   USART4 interrupt priority level setting.
 */
#if !defined(CH32_UART_USART4_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_UART_USART4_IRQ_PRIORITY       12
#endif

/**
 * @brief   USART5 interrupt priority level setting.
 */
#if !defined(CH32_UART_USART5_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_UART_USART5_IRQ_PRIORITY       12
#endif

/**
 * @brief   USART6 interrupt priority level setting.
 */
#if !defined(CH32_UART_USART6_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_UART_USART6_IRQ_PRIORITY       12
#endif

/**
 * @brief   USART7 interrupt priority level setting.
 */
#if !defined(CH32_UART_USART7_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_UART_USART7_IRQ_PRIORITY       12
#endif

/**
 * @brief   USART8 interrupt priority level setting.
 */
#if !defined(CH32_UART_USART8_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_UART_USART8_IRQ_PRIORITY       12
#endif

/**
 * @brief   USART1 DMA priority (0..3|lowest..highest).
 */
#if !defined(CH32_UART_USART1_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_UART_USART1_DMA_PRIORITY       0
#endif

/**
 * @brief   USART2 DMA priority (0..3|lowest..highest).
 */
#if !defined(CH32_UART_USART2_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_UART_USART2_DMA_PRIORITY       0
#endif

/**
 * @brief   USART3 DMA priority (0..3|lowest..highest).
 */
#if !defined(CH32_UART_USART3_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_UART_USART3_DMA_PRIORITY       0
#endif

/**
 * @brief   USART4 DMA priority (0..3|lowest..highest).
 */
#if !defined(CH32_UART_USART4_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_UART_USART4_DMA_PRIORITY       0
#endif

/**
 * @brief   USART5 DMA priority (0..3|lowest..highest).
 */
#if !defined(CH32_UART_USART5_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_UART_USART5_DMA_PRIORITY       0
#endif

/**
 * @brief   USART6 DMA priority (0..3|lowest..highest).
 */
#if !defined(CH32_UART_USART6_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_UART_USART6_DMA_PRIORITY       0
#endif

/**
 * @brief   USART7 DMA priority (0..3|lowest..highest).
 */
#if !defined(CH32_UART_USART7_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_UART_USART7_DMA_PRIORITY       0
#endif

/**
 * @brief   USART8 DMA priority (0..3|lowest..highest).
 */
#if !defined(CH32_UART_USART8_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_UART_USART8_DMA_PRIORITY       0
#endif

/**
 * @brief   UART DMA error hook.
 * @note    The default action for DMA errors is a system halt because DMA
 *          error can only happen because programming errors.
 */
#if !defined(CH32_UART_DMA_ERROR_HOOK) || defined(__DOXYGEN__)
#define CH32_UART_DMA_ERROR_HOOK(uartp)    osalSysHalt("DMA failure")
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if CH32_UART_USE_USART1 && !defined(USART1)
#error "USART1 not present in the selected device"
#endif

#if CH32_UART_USE_USART2 && !defined(USART2)
#error "USART2 not present in the selected device"
#endif

#if CH32_UART_USE_USART3 && !defined(USART3)
#error "USART3 not present in the selected device"
#endif

#if CH32_UART_USE_USART4 && !defined(USART4)
#error "USART4 not present in the selected device"
#endif

#if CH32_UART_USE_USART5 && !defined(USART5)
#error "USART5 not present in the selected device"
#endif

#if CH32_UART_USE_USART6 && !defined(USART6)
#error "USART6 not present in the selected device"
#endif

#if CH32_UART_USE_USART7 && !defined(USART7)
#error "USART7 not present in the selected device"
#endif

#if CH32_UART_USE_USART8 && !defined(USART8)
#error "USART8 not present in the selected device"
#endif

#if !CH32_UART_USE_USART1 && !CH32_UART_USE_USART2 &&                           \
    !CH32_UART_USE_USART3 && !CH32_UART_USE_USART4  &&                           \
    !CH32_UART_USE_USART5 && !CH32_UART_USE_USART6  &&                           \
    !CH32_UART_USE_USART7 && !CH32_UART_USE_USART8
#error "UART driver activated but no USART/UART peripheral assigned"
#endif

/* Check for exclusive USART allocation.*/
#if CH32_UART_USE_USART1
#if defined(CH32_USART1_IS_USED)
#error "UARTD1 requires USART1 but it is already used"
#else
#define CH32_USART1_IS_USED
#endif
#endif

#if CH32_UART_USE_USART2
#if defined(CH32_USART2_IS_USED)
#error "UARTD2 requires USART2 but it is already used"
#else
#define CH32_USART2_IS_USED
#endif
#endif

#if CH32_UART_USE_USART3
#if defined(CH32_USART3_IS_USED)
#error "UARTD3 requires USART3 but it is already used"
#else
#define CH32_USART3_IS_USED
#endif
#endif

#if CH32_UART_USE_USART4
#if defined(CH32_USART4_IS_USED)
#error "UARTD4 requires USART4 but it is already used"
#else
#define CH32_USART4_IS_USED
#endif
#endif

#if CH32_UART_USE_USART5
#if defined(CH32_USART5_IS_USED)
#error "UARTD5 requires USART5 but it is already used"
#else
#define CH32_USART5_IS_USED
#endif
#endif

#if CH32_UART_USE_USART6
#if defined(CH32_USART6_IS_USED)
#error "UARTD6 requires USART6 but it is already used"
#else
#define CH32_USART6_IS_USED
#endif
#endif

#if CH32_UART_USE_USART7
#if defined(CH32_USART7_IS_USED)
#error "UARTD7 requires USART7 but it is already used"
#else
#define CH32_USART7_IS_USED
#endif
#endif

#if CH32_UART_USE_USART8
#if defined(CH32_USART8_IS_USED)
#error "UARTD8 requires USART8 but it is already used"
#else
#define CH32_USART8_IS_USED
#endif
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   UART driver condition flags type.
 */
typedef uint32_t uartflags_t;

/**
 * @brief   Type of an UART driver.
 */
typedef struct hal_uart_driver UARTDriver;

/**
 * @brief   Generic UART notification callback type.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 */
typedef void (*uartcb_t)(UARTDriver *uartp);

/**
 * @brief   Character received UART notification callback type.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 * @param[in] c         received character
 */
typedef void (*uartccb_t)(UARTDriver *uartp, uint16_t c);

/**
 * @brief   Receive error UART notification callback type.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 * @param[in] e         receive error mask
 */
typedef void (*uartecb_t)(UARTDriver *uartp, uartflags_t e);

/**
 * @brief   Type of an UART configuration structure.
 */
typedef struct hal_uart_config {
  /**
   * @brief End of transmission buffer callback.
   */
  uartcb_t                  txend1_cb;
  /**
   * @brief Physical end of transmission callback.
   */
  uartcb_t                  txend2_cb;
  /**
   * @brief Receive buffer filled callback.
   */
  uartcb_t                  rxend_cb;
  /**
   * @brief Character received while out of the @p UART_RX_ACTIVE state.
   */
  uartccb_t                 rxchar_cb;
  /**
   * @brief Receive error callback.
   */
  uartecb_t                 rxerr_cb;
  /* End of the mandatory fields.*/
  /**
   * @brief   Receiver timeout callback.
   */
  uartcb_t                  timeout_cb;
  /**
   * @brief Bit rate.
   */
  uint32_t                  baud;
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
} UARTConfig;

/**
 * @brief   Structure representing an UART driver.
 */
struct hal_uart_driver {
  /**
   * @brief Driver state.
   */
  uartstate_t               state;
  /**
   * @brief Transmitter state.
   */
  uarttxstate_t             txstate;
  /**
   * @brief Receiver state.
   */
  uartrxstate_t             rxstate;
  /**
   * @brief Current configuration data.
   */
  const UARTConfig          *config;
#if (UART_USE_WAIT == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   Synchronization flag for transmit operations.
   */
  bool                      early;
  /**
   * @brief   Waiting thread on RX.
   */
  thread_reference_t        threadrx;
  /**
   * @brief   Waiting thread on TX.
   */
  thread_reference_t        threadtx;
#endif /* UART_USE_WAIT */
#if (UART_USE_MUTUAL_EXCLUSION == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   Mutex protecting the peripheral.
   */
  mutex_t                   mutex;
#endif /* UART_USE_MUTUAL_EXCLUSION */
#if defined(UART_DRIVER_EXT_FIELDS)
  UART_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (CH32_UART_USE_USART1 == TRUE) && !defined(__DOXYGEN__)
extern UARTDriver UARTD1;
#endif

#if (CH32_UART_USE_USART2 == TRUE) && !defined(__DOXYGEN__)
extern UARTDriver UARTD2;
#endif

#if (CH32_UART_USE_USART3 == TRUE) && !defined(__DOXYGEN__)
extern UARTDriver UARTD3;
#endif

#if (CH32_UART_USE_USART4 == TRUE) && !defined(__DOXYGEN__)
extern UARTDriver UARTD4;
#endif

#if (CH32_UART_USE_USART5 == TRUE) && !defined(__DOXYGEN__)
extern UARTDriver UARTD5;
#endif

#if (CH32_UART_USE_USART6 == TRUE) && !defined(__DOXYGEN__)
extern UARTDriver UARTD6;
#endif

#if (CH32_UART_USE_USART7 == TRUE) && !defined(__DOXYGEN__)
extern UARTDriver UARTD7;
#endif

#if (CH32_UART_USE_USART8 == TRUE) && !defined(__DOXYGEN__)
extern UARTDriver UARTD8;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void uart_lld_init(void);
  void uart_lld_start(UARTDriver *uartp);
  void uart_lld_stop(UARTDriver *uartp);
  void uart_lld_start_send(UARTDriver *uartp, size_t n, const void *txbuf);
  size_t uart_lld_stop_send(UARTDriver *uartp);
  void uart_lld_start_receive(UARTDriver *uartp, size_t n, void *rxbuf);
  size_t uart_lld_stop_receive(UARTDriver *uartp);
  void uart_lld_serve_interrupt(UARTDriver *uartp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_UART == TRUE */

#endif /* HAL_UART_LLD_H */

/** @} */
