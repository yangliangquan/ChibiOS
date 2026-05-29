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
 * @file    USARTv1/hal_sio_lld.h
 * @brief   CH32 SIO subsystem low level driver header.
 *
 * @addtogroup SIO
 * @{
 */

#ifndef HAL_SIO_LLD_H
#define HAL_SIO_LLD_H

#if (HAL_USE_SIO == TRUE) || defined(__DOXYGEN__)

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

/**
 * @name    STATR bit position definitions
 * @details The CH32 CMSIS header does not provide _Pos macros, so they
 *          are defined here for use with __sio_reloc_field().
 * @{
 */
#define USART_STATR_PE_Pos                    (0U)
#define USART_STATR_FE_Pos                    (1U)
#define USART_STATR_NE_Pos                    (2U)
#define USART_STATR_ORE_Pos                   (3U)
#define USART_STATR_IDLE_Pos                  (4U)
#define USART_STATR_RXNE_Pos                  (5U)
#define USART_STATR_TC_Pos                    (6U)
#define USART_STATR_TXE_Pos                   (7U)
#define USART_STATR_LBD_Pos                   (8U)
/** @} */

/**
 * @name    CTLR1 bit position definitions
 * @{
 */
#define USART_CTLR1_TXEIE_Pos                 (7U)
#define USART_CTLR1_TCIE_Pos                  (6U)
#define USART_CTLR1_RXNEIE_Pos                (5U)
#define USART_CTLR1_IDLEIE_Pos                (4U)
#define USART_CTLR1_PEIE_Pos                  (8U)
/** @} */

/**
 * @name    CTLR2 bit position definitions
 * @{
 */
#define USART_CTLR2_LBDIE_Pos                 (6U)
/** @} */

/**
 * @name    Mask macros for STATR (needed because CH32 has no _Msk)
 * @{
 */
#define USART_STATR_PE_Msk                    ((uint16_t)USART_STATR_PE)
#define USART_STATR_FE_Msk                    ((uint16_t)USART_STATR_FE)
#define USART_STATR_NE_Msk                    ((uint16_t)USART_STATR_NE)
#define USART_STATR_ORE_Msk                   ((uint16_t)USART_STATR_ORE)
#define USART_STATR_IDLE_Msk                  ((uint16_t)USART_STATR_IDLE)
#define USART_STATR_RXNE_Msk                  ((uint16_t)USART_STATR_RXNE)
#define USART_STATR_TC_Msk                    ((uint16_t)USART_STATR_TC)
#define USART_STATR_TXE_Msk                   ((uint16_t)USART_STATR_TXE)
#define USART_STATR_LBD_Msk                   ((uint16_t)USART_STATR_LBD)
/** @} */

/**
 * @brief   Mask of RX-related errors in the STATR register.
 */
#define SIO_LLD_ISR_RX_ERRORS               (USART_STATR_NE   | USART_STATR_FE   |  \
                                             USART_STATR_PE   | USART_STATR_ORE  |  \
                                             USART_STATR_LBD)

/* Mask of error bits only (excluding IDLE and LBD).*/
#define USART_STATR_ONFP_Pos                  USART_STATR_PE_Pos
#define USART_STATR_ONFP_Msk                  (0x0FUL << USART_STATR_ONFP_Pos)

/* Mask of error + IDLE bits (ONFP = ORE, NE, FE, PE plus IDLE).*/
#define USART_STATR_IONFP_Pos                 USART_STATR_PE_Pos
#define USART_STATR_IONFP_Msk                 (0x1FUL << USART_STATR_IONFP_Pos)

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    CH32 configuration options
 * @{
 */
/**
 * @brief   SIO driver 1 enable switch.
 * @details If set to @p TRUE the support for USART1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SIO_USE_USART1) || defined(__DOXYGEN__)
#define CH32_SIO_USE_USART1                   FALSE
#endif

/**
 * @brief   SIO driver 2 enable switch.
 * @details If set to @p TRUE the support for USART2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SIO_USE_USART2) || defined(__DOXYGEN__)
#define CH32_SIO_USE_USART2                   FALSE
#endif

/**
 * @brief   SIO driver 3 enable switch.
 * @details If set to @p TRUE the support for USART3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SIO_USE_USART3) || defined(__DOXYGEN__)
#define CH32_SIO_USE_USART3                   FALSE
#endif

/**
 * @brief   SIO driver 4 enable switch.
 * @details If set to @p TRUE the support for USART4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SIO_USE_USART4) || defined(__DOXYGEN__)
#define CH32_SIO_USE_USART4                   FALSE
#endif

/**
 * @brief   SIO driver 5 enable switch.
 * @details If set to @p TRUE the support for USART5 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SIO_USE_USART5) || defined(__DOXYGEN__)
#define CH32_SIO_USE_USART5                   FALSE
#endif

/**
 * @brief   SIO driver 6 enable switch.
 * @details If set to @p TRUE the support for USART6 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SIO_USE_USART6) || defined(__DOXYGEN__)
#define CH32_SIO_USE_USART6                   FALSE
#endif

/**
 * @brief   SIO driver 7 enable switch.
 * @details If set to @p TRUE the support for USART7 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SIO_USE_USART7) || defined(__DOXYGEN__)
#define CH32_SIO_USE_USART7                   FALSE
#endif

/**
 * @brief   SIO driver 8 enable switch.
 * @details If set to @p TRUE the support for USART8 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_SIO_USE_USART8) || defined(__DOXYGEN__)
#define CH32_SIO_USE_USART8                   FALSE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !CH32_SIO_USE_USART1 && !CH32_SIO_USE_USART2 &&                           \
    !CH32_SIO_USE_USART3 && !CH32_SIO_USE_USART4  &&                           \
    !CH32_SIO_USE_USART5 && !CH32_SIO_USE_USART6  &&                           \
    !CH32_SIO_USE_USART7 && !CH32_SIO_USE_USART8
#error "SIO driver activated but no USART peripheral assigned"
#endif

/* Check for exclusive USART allocation.*/
#if CH32_SIO_USE_USART1
#if defined(CH32_USART1_IS_USED)
#error "SIOD1 requires USART1 but it is already used"
#else
#define CH32_USART1_IS_USED
#endif
#endif

#if CH32_SIO_USE_USART2
#if defined(CH32_USART2_IS_USED)
#error "SIOD2 requires USART2 but it is already used"
#else
#define CH32_USART2_IS_USED
#endif
#endif

#if CH32_SIO_USE_USART3
#if defined(CH32_USART3_IS_USED)
#error "SIOD3 requires USART3 but it is already used"
#else
#define CH32_USART3_IS_USED
#endif
#endif

#if CH32_SIO_USE_USART4
#if defined(CH32_USART4_IS_USED)
#error "SIOD4 requires USART4 but it is already used"
#else
#define CH32_USART4_IS_USED
#endif
#endif

#if CH32_SIO_USE_USART5
#if defined(CH32_USART5_IS_USED)
#error "SIOD5 requires USART5 but it is already used"
#else
#define CH32_USART5_IS_USED
#endif
#endif

#if CH32_SIO_USE_USART6
#if defined(CH32_USART6_IS_USED)
#error "SIOD6 requires USART6 but it is already used"
#else
#define CH32_USART6_IS_USED
#endif
#endif

#if CH32_SIO_USE_USART7
#if defined(CH32_USART7_IS_USED)
#error "SIOD7 requires USART7 but it is already used"
#else
#define CH32_USART7_IS_USED
#endif
#endif

#if CH32_SIO_USE_USART8
#if defined(CH32_USART8_IS_USED)
#error "SIOD8 requires USART8 but it is already used"
#else
#define CH32_USART8_IS_USED
#endif
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Low level fields of the SIO driver structure.
 */
#define sio_lld_driver_fields                                               \
  /* Pointer to the USARTx registers block.*/                               \
  USART_TypeDef             *usart;                                         \
  /* Clock frequency for the associated USART.*/                            \
  uint32_t                  clock;

/**
 * @brief   Low level fields of the SIO configuration structure.
 */
#define sio_lld_config_fields                                               \
  /* Desired baud rate.*/                                                   \
  uint32_t                  baud;                                           \
  /* USART CTLR1 register initialization data.*/                            \
  uint32_t                  cr1;                                            \
  /* USART CTLR2 register initialization data.*/                            \
  uint32_t                  cr2;                                            \
  /* USART CTLR3 register initialization data.*/                            \
  uint32_t                  cr3;

/**
 * @brief   Determines the state of the RX FIFO.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @return              The RX FIFO state.
 * @retval false        if RX FIFO is not empty
 * @retval true         if RX FIFO is empty
 *
 * @notapi
 */
#define sio_lld_is_rx_empty(siop)                                           \
  (bool)(((siop)->usart->STATR & USART_STATR_RXNE) == 0U)

/**
 * @brief   Determines the activity state of the receiver.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @return              The RX activity state.
 * @retval false        if RX is in active state.
 * @retval true         if RX is in idle state.
 *
 * @notapi
 */
#define sio_lld_is_rx_idle(siop)                                            \
  (bool)(((siop)->usart->STATR & USART_STATR_IDLE) != 0U)

/**
 * @brief   Determines if RX has pending error events to be read and cleared.
 * @note    Only error and protocol errors are handled, data events are not
 *          considered.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @return              The RX error events.
 * @retval false        if RX has no pending events
 * @retval true         if RX has pending events
 *
 * @notapi
 */
#define sio_lld_has_rx_errors(siop)                                         \
  (bool)(((siop)->usart->STATR & SIO_LLD_ISR_RX_ERRORS) != 0U)

/**
 * @brief   Determines the state of the TX FIFO.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @return              The TX FIFO state.
 * @retval false        if TX FIFO is not full
 * @retval true         if TX FIFO is full
 *
 * @notapi
 */
#define sio_lld_is_tx_full(siop)                                            \
  (bool)(((siop)->usart->STATR & USART_STATR_TXE) == 0U)

/**
 * @brief   Determines the transmission state.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @return              The TX FIFO state.
 * @retval false        if transmission is idle
 * @retval true         if transmission is ongoing
 *
 * @notapi
 */
#define sio_lld_is_tx_ongoing(siop)                                         \
  (bool)(((siop)->usart->STATR & USART_STATR_TC) == 0U)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (CH32_SIO_USE_USART1 == TRUE) && !defined(__DOXYGEN__)
extern SIODriver SIOD1;
#endif

#if (CH32_SIO_USE_USART2 == TRUE) && !defined(__DOXYGEN__)
extern SIODriver SIOD2;
#endif

#if (CH32_SIO_USE_USART3 == TRUE) && !defined(__DOXYGEN__)
extern SIODriver SIOD3;
#endif

#if (CH32_SIO_USE_USART4 == TRUE) && !defined(__DOXYGEN__)
extern SIODriver SIOD4;
#endif

#if (CH32_SIO_USE_USART5 == TRUE) && !defined(__DOXYGEN__)
extern SIODriver SIOD5;
#endif

#if (CH32_SIO_USE_USART6 == TRUE) && !defined(__DOXYGEN__)
extern SIODriver SIOD6;
#endif

#if (CH32_SIO_USE_USART7 == TRUE) && !defined(__DOXYGEN__)
extern SIODriver SIOD7;
#endif

#if (CH32_SIO_USE_USART8 == TRUE) && !defined(__DOXYGEN__)
extern SIODriver SIOD8;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void sio_lld_init(void);
  msg_t  sio_lld_start(SIODriver *siop);
  void sio_lld_stop(SIODriver *siop);
  void sio_lld_update_enable_flags(SIODriver *siop);
  sioevents_t sio_lld_get_and_clear_errors(SIODriver *siop);
  sioevents_t sio_lld_get_and_clear_events(SIODriver *siop);
  sioevents_t sio_lld_get_events(SIODriver *siop);
  size_t sio_lld_read(SIODriver *siop, uint8_t *buffer, size_t n);
  size_t sio_lld_write(SIODriver *siop, const uint8_t *buffer, size_t n);
  msg_t sio_lld_get(SIODriver *siop);
  void sio_lld_put(SIODriver *siop, uint_fast16_t data);
  msg_t sio_lld_control(SIODriver *siop, unsigned int operation, void *arg);
  void sio_lld_serve_interrupt(SIODriver *siop);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SIO == TRUE */

#endif /* HAL_SIO_LLD_H */

/** @} */
