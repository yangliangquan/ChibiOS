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
 * @file    DMAv1/ch32_dma.h
 * @brief   DMA helper driver header.
 * @note    This driver uses the new naming convention used for the CH32F2xx
 *          so the "DMA channels" are referred as "DMA streams".
 *
 * @addtogroup CH32_DMA
 * @{
 */

#ifndef CH32_DMA_H
#define CH32_DMA_H

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   DMA capability.
 * @details if @p TRUE then the DMA is able of burst transfers, FIFOs,
 *          scatter gather and other advanced features.
 */
#define CH32_DMA_ADVANCED          FALSE

/**
 * @brief   Maximum number of transfers in a single operation.
 */
#define CH32_DMA_MAX_TRANSFER      65535

/**
 * @brief   Total number of DMA streams.
 * @details This is the total number of streams among all the DMA units.
 */
#define CH32_DMA_STREAMS           (CH32_DMA1_NUM_CHANNELS +              \
                                     CH32_DMA2_NUM_CHANNELS)

/**
 * @brief   Mask of the ISR bits passed to the DMA callback functions.
 */
#define CH32_DMA_ISR_MASK          0x0F

/**
 * @brief   Returns the request line associated to the specified stream.
 * @note    In some CH32 manuals the request line is named confusingly
 *          channel.
 *
 * @param[in] id        the unique numeric stream identifier
 * @param[in] c         a stream/request association word, one request per
 *                      nibble
 * @return              Returns the request associated to the stream.
 */
#define CH32_DMA_GETCHANNEL(id, c)                                         \
  (((uint32_t)(c) >> (((uint32_t)(id) % (uint32_t)CH32_DMA1_NUM_CHANNELS) * 4U)) & 15U)

/**
 * @brief   Checks if a DMA priority is within the valid range.
 * @param[in] prio      DMA priority
 *
 * @retval              The check result.
 * @retval false        invalid DMA priority.
 * @retval true         correct DMA priority.
 */
#define CH32_DMA_IS_VALID_PRIORITY(prio) (((prio) >= 0U) && ((prio) <= 3U))

#if (CH32_DMA_SUPPORTS_DMAMUX == FALSE) || defined(_DOXYGEN__)
/**
 * @brief   Checks if a DMA stream id is within the valid range.
 *
 * @param[in] id        DMA stream id
 * @retval              The check result.
 * @retval false        invalid DMA channel.
 * @retval true         correct DMA channel.
 */
#define CH32_DMA_IS_VALID_STREAM(id) (((id) >= 0U) &&                      \
                                       ((id) < CH32_DMA_STREAMS))
#else /* CH32_DMA_SUPPORTS_DMAMUX == FALSE */
#if CH32_DMA2_NUM_CHANNELS > 0
#define CH32_DMA_IS_VALID_STREAM(id) (((id) >= 0U) &&                      \
                                       ((id) <= (CH32_DMA_STREAMS + 2)))
#else
#define CH32_DMA_IS_VALID_STREAM(id) (((id) >= 0U) &&                      \
                                       ((id) <= (CH32_DMA_STREAMS + 1)))
#endif
#endif /* CH32_DMA_SUPPORTS_DMAMUX == FALSE */

/**
 * @brief   Returns an unique numeric identifier for a DMA stream.
 *
 * @param[in] dma       the DMA unit number
 * @param[in] stream    the stream number
 * @return              An unique numeric stream identifier.
 */
#define CH32_DMA_STREAM_ID(dma, stream)                                    \
  ((((dma) - 1) * CH32_DMA1_NUM_CHANNELS) + ((stream) - 1))

/**
 * @brief   Returns a DMA stream identifier mask.
 *
 *
 * @param[in] dma       the DMA unit number
 * @param[in] stream    the stream number
 * @return              A DMA stream identifier mask.
 */
#define CH32_DMA_STREAM_ID_MSK(dma, stream)                                \
  (1U << CH32_DMA_STREAM_ID(dma, stream))

/**
 * @brief   Checks if a DMA stream unique identifier belongs to a mask.
 *
 * @param[in] id        the stream numeric identifier
 * @param[in] mask      the stream numeric identifiers mask
 *
 * @retval              The check result.
 * @retval false        id does not belong to the mask.
 * @retval true         id belongs to the mask.
 */
#define CH32_DMA_IS_VALID_ID(id, mask) (((1U << (id)) & (mask)))

#if (CH32_DMA_SUPPORTS_DMAMUX == TRUE) || defined(_DOXYGEN__)
/**
 * @name    Special stream identifiers
 * @{
 */
#define CH32_DMA_STREAM_ID_ANY         CH32_DMA_STREAMS
#define CH32_DMA_STREAM_ID_ANY_DMA1    (CH32_DMA_STREAM_ID_ANY + 1)
#if CH32_DMA2_NUM_CHANNELS > 0
#define CH32_DMA_STREAM_ID_ANY_DMA2    (CH32_DMA_STREAM_ID_ANY_DMA1 + 1)
#endif
/** @} */
#endif

/**
 * @name    DMA streams identifiers
 * @{
 */
/**
 * @brief   Returns a pointer to a ch32_dma_stream_t structure.
 *
 * @param[in] id        the stream numeric identifier
 * @return              A pointer to the ch32_dma_stream_t constant structure
 *                      associated to the DMA stream.
 */
#define CH32_DMA_STREAM(id)        (&_ch32_dma_streams[id])

#if CH32_DMA1_NUM_CHANNELS > 0
#define CH32_DMA1_STREAM1          CH32_DMA_STREAM(0)
#endif
#if CH32_DMA1_NUM_CHANNELS > 1
#define CH32_DMA1_STREAM2          CH32_DMA_STREAM(1)
#endif
#if CH32_DMA1_NUM_CHANNELS > 2
#define CH32_DMA1_STREAM3          CH32_DMA_STREAM(2)
#endif
#if CH32_DMA1_NUM_CHANNELS > 3
#define CH32_DMA1_STREAM4          CH32_DMA_STREAM(3)
#endif
#if CH32_DMA1_NUM_CHANNELS > 4
#define CH32_DMA1_STREAM5          CH32_DMA_STREAM(4)
#endif
#if CH32_DMA1_NUM_CHANNELS > 5
#define CH32_DMA1_STREAM6          CH32_DMA_STREAM(5)
#endif
#if CH32_DMA1_NUM_CHANNELS > 6
#define CH32_DMA1_STREAM7          CH32_DMA_STREAM(6)
#endif
#if CH32_DMA1_NUM_CHANNELS > 7
#define CH32_DMA1_STREAM8          CH32_DMA_STREAM(7)
#endif
#if CH32_DMA2_NUM_CHANNELS > 0
#define CH32_DMA2_STREAM1          CH32_DMA_STREAM(CH32_DMA1_NUM_CHANNELS + 0)
#endif
#if CH32_DMA2_NUM_CHANNELS > 1
#define CH32_DMA2_STREAM2          CH32_DMA_STREAM(CH32_DMA1_NUM_CHANNELS + 1)
#endif
#if CH32_DMA2_NUM_CHANNELS > 2
#define CH32_DMA2_STREAM3          CH32_DMA_STREAM(CH32_DMA1_NUM_CHANNELS + 2)
#endif
#if CH32_DMA2_NUM_CHANNELS > 3
#define CH32_DMA2_STREAM4          CH32_DMA_STREAM(CH32_DMA1_NUM_CHANNELS + 3)
#endif
#if CH32_DMA2_NUM_CHANNELS > 4
#define CH32_DMA2_STREAM5          CH32_DMA_STREAM(CH32_DMA1_NUM_CHANNELS + 4)
#endif
#if CH32_DMA2_NUM_CHANNELS > 5
#define CH32_DMA2_STREAM6          CH32_DMA_STREAM(CH32_DMA1_NUM_CHANNELS + 5)
#endif
#if CH32_DMA2_NUM_CHANNELS > 6
#define CH32_DMA2_STREAM7          CH32_DMA_STREAM(CH32_DMA1_NUM_CHANNELS + 6)
#endif
#if CH32_DMA2_NUM_CHANNELS > 7
#define CH32_DMA2_STREAM8          CH32_DMA_STREAM(CH32_DMA1_NUM_CHANNELS + 7)
#endif
/** @} */

/* DMA_data_transfer_direction */
#define DMA_DIR_PeripheralDST              ((uint32_t)0x00000010)
#define DMA_DIR_PeripheralSRC              ((uint32_t)0x00000000)

/* DMA_peripheral_incremented_mode */
#define DMA_PeripheralInc_Enable           ((uint32_t)0x00000040)
#define DMA_PeripheralInc_Disable          ((uint32_t)0x00000000)
											
/* DMA_memory_incremented_mode */
#define DMA_MemoryInc_Enable               ((uint32_t)0x00000080)
#define DMA_MemoryInc_Disable              ((uint32_t)0x00000000)
										
/* DMA_peripheral_data_size */
#define DMA_PeripheralDataSize_Byte        ((uint32_t)0x00000000)
#define DMA_PeripheralDataSize_HalfWord    ((uint32_t)0x00000100)
#define DMA_PeripheralDataSize_Word        ((uint32_t)0x00000200)
#define DMA_PeripheralDataSize_256         ((uint32_t)0x00000300)

/* DMA_memory_data_size */
#define DMA_MemoryDataSize_Byte            ((uint32_t)0x00000000)
#define DMA_MemoryDataSize_HalfWord        ((uint32_t)0x00000400)
#define DMA_MemoryDataSize_Word            ((uint32_t)0x00000800)
#define DMA_MemoryDataSize_256             ((uint32_t)0x00000C00)

/* DMA_circular_normal_mode */
#define DMA_Mode_Circular                  ((uint32_t)0x00000020)
#define DMA_Mode_Normal                    ((uint32_t)0x00000000)

/* DMA_priority_level */
#define DMA_Priority_VeryHigh              ((uint32_t)0x00003000)
#define DMA_Priority_High                  ((uint32_t)0x00002000)
#define DMA_Priority_Medium                ((uint32_t)0x00001000)
#define DMA_Priority_Low                   ((uint32_t)0x00000000)

/* DMA_memory_to_memory */
#define DMA_M2M_Enable                     ((uint32_t)0x00004000)
#define DMA_M2M_Disable                    ((uint32_t)0x00000000)

/* DMA_interrupts_definition */
#define DMA_IT_TC                          ((uint32_t)0x00000002)
#define DMA_IT_HT                          ((uint32_t)0x00000004)
#define DMA_IT_TE                          ((uint32_t)0x00000008)

#define DMA1_IT_GL1                        ((uint32_t)0x00000001)
#define DMA1_IT_TC1                        ((uint32_t)0x00000002)
#define DMA1_IT_HT1                        ((uint32_t)0x00000004)
#define DMA1_IT_TE1                        ((uint32_t)0x00000008)
#define DMA1_IT_GL2                        ((uint32_t)0x00000010)
#define DMA1_IT_TC2                        ((uint32_t)0x00000020)
#define DMA1_IT_HT2                        ((uint32_t)0x00000040)
#define DMA1_IT_TE2                        ((uint32_t)0x00000080)
#define DMA1_IT_GL3                        ((uint32_t)0x00000100)
#define DMA1_IT_TC3                        ((uint32_t)0x00000200)
#define DMA1_IT_HT3                        ((uint32_t)0x00000400)
#define DMA1_IT_TE3                        ((uint32_t)0x00000800)
#define DMA1_IT_GL4                        ((uint32_t)0x00001000)
#define DMA1_IT_TC4                        ((uint32_t)0x00002000)
#define DMA1_IT_HT4                        ((uint32_t)0x00004000)
#define DMA1_IT_TE4                        ((uint32_t)0x00008000)
#define DMA1_IT_GL5                        ((uint32_t)0x00010000)
#define DMA1_IT_TC5                        ((uint32_t)0x00020000)
#define DMA1_IT_HT5                        ((uint32_t)0x00040000)
#define DMA1_IT_TE5                        ((uint32_t)0x00080000)
#define DMA1_IT_GL6                        ((uint32_t)0x00100000)
#define DMA1_IT_TC6                        ((uint32_t)0x00200000)
#define DMA1_IT_HT6                        ((uint32_t)0x00400000)
#define DMA1_IT_TE6                        ((uint32_t)0x00800000)
#define DMA1_IT_GL7                        ((uint32_t)0x01000000)
#define DMA1_IT_TC7                        ((uint32_t)0x02000000)
#define DMA1_IT_HT7                        ((uint32_t)0x04000000)
#define DMA1_IT_TE7                        ((uint32_t)0x08000000)
#define DMA1_IT_GL8                        ((uint32_t)0x10000000)
#define DMA1_IT_TC8                        ((uint32_t)0x20000000)
#define DMA1_IT_HT8                        ((uint32_t)0x40000000)
#define DMA1_IT_TE8                        ((uint32_t)0x80000000)

#define DMA2_IT_GL1                        ((uint32_t)0x00000001)
#define DMA2_IT_TC1                        ((uint32_t)0x00000002)
#define DMA2_IT_HT1                        ((uint32_t)0x00000004)
#define DMA2_IT_TE1                        ((uint32_t)0x00000008)
#define DMA2_IT_GL2                        ((uint32_t)0x00000010)
#define DMA2_IT_TC2                        ((uint32_t)0x00000020)
#define DMA2_IT_HT2                        ((uint32_t)0x00000040)
#define DMA2_IT_TE2                        ((uint32_t)0x00000080)
#define DMA2_IT_GL3                        ((uint32_t)0x00000100)
#define DMA2_IT_TC3                        ((uint32_t)0x00000200)
#define DMA2_IT_HT3                        ((uint32_t)0x00000400)
#define DMA2_IT_TE3                        ((uint32_t)0x00000800)
#define DMA2_IT_GL4                        ((uint32_t)0x00001000)
#define DMA2_IT_TC4                        ((uint32_t)0x00002000)
#define DMA2_IT_HT4                        ((uint32_t)0x00004000)
#define DMA2_IT_TE4                        ((uint32_t)0x00008000)
#define DMA2_IT_GL5                        ((uint32_t)0x00010000)
#define DMA2_IT_TC5                        ((uint32_t)0x00020000)
#define DMA2_IT_HT5                        ((uint32_t)0x00040000)
#define DMA2_IT_TE5                        ((uint32_t)0x00080000)
#define DMA2_IT_GL6                        ((uint32_t)0x00100000)
#define DMA2_IT_TC6                        ((uint32_t)0x00200000)
#define DMA2_IT_HT6                        ((uint32_t)0x00400000)
#define DMA2_IT_TE6                        ((uint32_t)0x00800000)
#define DMA2_IT_GL7                        ((uint32_t)0x01000000)
#define DMA2_IT_TC7                        ((uint32_t)0x02000000)
#define DMA2_IT_HT7                        ((uint32_t)0x04000000)
#define DMA2_IT_TE7                        ((uint32_t)0x08000000)
#define DMA2_IT_GL8                        ((uint32_t)0x10000000)
#define DMA2_IT_TC8                        ((uint32_t)0x20000000)
#define DMA2_IT_HT8                        ((uint32_t)0x40000000)
#define DMA2_IT_TE8                        ((uint32_t)0x80000000)

/* DMA_flags_definition */
#define DMA1_FLAG_GL1                      ((uint32_t)0x00000001)
#define DMA1_FLAG_TC1                      ((uint32_t)0x00000002)
#define DMA1_FLAG_HT1                      ((uint32_t)0x00000004)
#define DMA1_FLAG_TE1                      ((uint32_t)0x00000008)
#define DMA1_FLAG_GL2                      ((uint32_t)0x00000010)
#define DMA1_FLAG_TC2                      ((uint32_t)0x00000020)
#define DMA1_FLAG_HT2                      ((uint32_t)0x00000040)
#define DMA1_FLAG_TE2                      ((uint32_t)0x00000080)
#define DMA1_FLAG_GL3                      ((uint32_t)0x00000100)
#define DMA1_FLAG_TC3                      ((uint32_t)0x00000200)
#define DMA1_FLAG_HT3                      ((uint32_t)0x00000400)
#define DMA1_FLAG_TE3                      ((uint32_t)0x00000800)
#define DMA1_FLAG_GL4                      ((uint32_t)0x00001000)
#define DMA1_FLAG_TC4                      ((uint32_t)0x00002000)
#define DMA1_FLAG_HT4                      ((uint32_t)0x00004000)
#define DMA1_FLAG_TE4                      ((uint32_t)0x00008000)
#define DMA1_FLAG_GL5                      ((uint32_t)0x00010000)
#define DMA1_FLAG_TC5                      ((uint32_t)0x00020000)
#define DMA1_FLAG_HT5                      ((uint32_t)0x00040000)
#define DMA1_FLAG_TE5                      ((uint32_t)0x00080000)
#define DMA1_FLAG_GL6                      ((uint32_t)0x00100000)
#define DMA1_FLAG_TC6                      ((uint32_t)0x00200000)
#define DMA1_FLAG_HT6                      ((uint32_t)0x00400000)
#define DMA1_FLAG_TE6                      ((uint32_t)0x00800000)
#define DMA1_FLAG_GL7                      ((uint32_t)0x01000000)
#define DMA1_FLAG_TC7                      ((uint32_t)0x02000000)
#define DMA1_FLAG_HT7                      ((uint32_t)0x04000000)
#define DMA1_FLAG_TE7                      ((uint32_t)0x08000000)
#define DMA1_FLAG_GL8                      ((uint32_t)0x10000000)
#define DMA1_FLAG_TC8                      ((uint32_t)0x20000000)
#define DMA1_FLAG_HT8                      ((uint32_t)0x40000000)
#define DMA1_FLAG_TE8                      ((uint32_t)0x80000000)

#define DMA2_FLAG_GL1                      ((uint32_t)0x00000001)
#define DMA2_FLAG_TC1                      ((uint32_t)0x00000002)
#define DMA2_FLAG_HT1                      ((uint32_t)0x00000004)
#define DMA2_FLAG_TE1                      ((uint32_t)0x00000008)
#define DMA2_FLAG_GL2                      ((uint32_t)0x00000010)
#define DMA2_FLAG_TC2                      ((uint32_t)0x00000020)
#define DMA2_FLAG_HT2                      ((uint32_t)0x00000040)
#define DMA2_FLAG_TE2                      ((uint32_t)0x00000080)
#define DMA2_FLAG_GL3                      ((uint32_t)0x00000100)
#define DMA2_FLAG_TC3                      ((uint32_t)0x00000200)
#define DMA2_FLAG_HT3                      ((uint32_t)0x00000400)
#define DMA2_FLAG_TE3                      ((uint32_t)0x00000800)
#define DMA2_FLAG_GL4                      ((uint32_t)0x00001000)
#define DMA2_FLAG_TC4                      ((uint32_t)0x00002000)
#define DMA2_FLAG_HT4                      ((uint32_t)0x00004000)
#define DMA2_FLAG_TE4                      ((uint32_t)0x00008000)
#define DMA2_FLAG_GL5                      ((uint32_t)0x00010000)
#define DMA2_FLAG_TC5                      ((uint32_t)0x00020000)
#define DMA2_FLAG_HT5                      ((uint32_t)0x00040000)
#define DMA2_FLAG_TE5                      ((uint32_t)0x00080000)
#define DMA2_FLAG_GL6                      ((uint32_t)0x00100000)
#define DMA2_FLAG_TC6                      ((uint32_t)0x00200000)
#define DMA2_FLAG_HT6                      ((uint32_t)0x00400000)
#define DMA2_FLAG_TE6                      ((uint32_t)0x00800000)
#define DMA2_FLAG_GL7                      ((uint32_t)0x01000000)
#define DMA2_FLAG_TC7                      ((uint32_t)0x02000000)
#define DMA2_FLAG_HT7                      ((uint32_t)0x04000000)
#define DMA2_FLAG_TE7                      ((uint32_t)0x08000000)
#define DMA2_FLAG_GL8                      ((uint32_t)0x10000000)
#define DMA2_FLAG_TC8                      ((uint32_t)0x20000000)
#define DMA2_FLAG_HT8                      ((uint32_t)0x40000000)
#define DMA2_FLAG_TE8                      ((uint32_t)0x80000000)

/* DMA_MuxChannel_definition */
#define DMA_MuxChannel1                    ((uint8_t)0x00)
#define DMA_MuxChannel2                    ((uint8_t)0x01)
#define DMA_MuxChannel3                    ((uint8_t)0x02)
#define DMA_MuxChannel4                    ((uint8_t)0x03)
#define DMA_MuxChannel5                    ((uint8_t)0x04)
#define DMA_MuxChannel6                    ((uint8_t)0x05)
#define DMA_MuxChannel7                    ((uint8_t)0x06)
#define DMA_MuxChannel8                    ((uint8_t)0x07)
#define DMA_MuxChannel9                    ((uint8_t)0x08)
#define DMA_MuxChannel10                   ((uint8_t)0x09)
#define DMA_MuxChannel11                   ((uint8_t)0x0A)
#define DMA_MuxChannel12                   ((uint8_t)0x0B)
#define DMA_MuxChannel13                   ((uint8_t)0x0C)
#define DMA_MuxChannel14                   ((uint8_t)0x0D)
#define DMA_MuxChannel15                   ((uint8_t)0x0E)
#define DMA_MuxChannel16                   ((uint8_t)0x0F)



/* DMA_BufferMode_Memory */
#define DMA_SingleBufferMode               ((uint32_t)0x00000000)
#define DMA_DoubleBufferMode               ((uint32_t)0x00008000)

/* DMA_DoubleBufferMode_Memory */
#define DMA_DoubleBufferMode_Memory_0      ((uint32_t)0x00000000)
#define DMA_DoubleBufferMode_Memory_1      ((uint32_t)0x00010000)

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !defined(CH32_DMA_SUPPORTS_DMAMUX)
#error "CH32_DMA_SUPPORTS_DMAMUX not defined in registry"
#endif

#if !defined(CH32_DMA_SUPPORTS_CSELR)
#error "CH32_DMA_SUPPORTS_CSELR not defined in registry"
#endif

#if CH32_DMA_SUPPORTS_DMAMUX && CH32_DMA_SUPPORTS_CSELR
#error "CH32_DMA_SUPPORTS_DMAMUX and CH32_DMA_SUPPORTS_CSELR both TRUE"
#endif

#if !defined(CH32_DMA1_NUM_CHANNELS)
#error "CH32_DMA1_NUM_CHANNELS not defined in registry"
#endif

#if !defined(CH32_DMA2_NUM_CHANNELS)
#error "CH32_DMA2_NUM_CHANNELS not defined in registry"
#endif

#if (CH32_DMA1_NUM_CHANNELS < 0) || (CH32_DMA1_NUM_CHANNELS > 8)
#error "unsupported channels configuration"
#endif

#if (CH32_DMA2_NUM_CHANNELS < 0) || (CH32_DMA2_NUM_CHANNELS > 8)
#error "unsupported channels configuration"
#endif

#if (CH32_DMA_SUPPORTS_DMAMUX == TRUE) || defined(__DOXYGEN__)
#include "ch32_dmamux.h"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a DMA callback.
 *
 * @param[in] p         parameter for the registered function
 * @param[in] flags     pre-shifted content of the ISR register, the bits
 *                      are aligned to bit zero
 */
typedef void (*ch32_dmaisr_t)(void *p, uint32_t flags);

/**
 * @brief   CH32 DMA stream descriptor structure.
 */
typedef struct {
  DMA_TypeDef           *dma;           /**< @brief Associated DMA.         */
  DMA_Channel_TypeDef   *channel;       /**< @brief Associated DMA channel. */
  uint32_t              cmask;          /**< @brief Mask of streams sharing
                                             the same ISR.                  */
#if (CH32_DMA_SUPPORTS_CSELR == TRUE) || defined(__DOXYGEN__)
  volatile uint32_t     *cselr;         /**< @brief Associated CSELR reg.   */
#elif CH32_DMA_SUPPORTS_DMAMUX == TRUE
  DMAMUX_TypeDef        *mux;           /**< @brief Associated DMA mux.     */
#else
  uint8_t               dummy;          /**< @brief Filler.                 */
#endif
  uint8_t               shift;          /**< @brief Bit offset in ISR, IFCR
                                             and CSELR registers.           */
  uint8_t               selfindex;      /**< @brief Index to self in array. */
  uint8_t               vector;         /**< @brief Associated IRQ vector.  */
} ch32_dma_stream_t;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @name    Macro Functions
 * @{
 */
/**
 * @brief   Associates a peripheral data register to a DMA stream.
 * @note    This function can be invoked in both ISR or thread context.
 * @pre     The stream must have been allocated using @p dmaStreamAlloc().
 * @post    After use the stream can be released using @p dmaStreamRelease().
 *
 * @param[in] dmastp    pointer to a ch32_dma_stream_t structure
 * @param[in] addr      value to be written in the CPAR register
 *
 * @special
 */
#define dmaStreamSetPeripheral(dmastp, addr) {                              \
  (dmastp)->channel->PADDR = (uint32_t)(addr);                               \
}

/**
 * @brief   Associates a memory destination to a DMA stream.
 * @note    This function can be invoked in both ISR or thread context.
 * @pre     The stream must have been allocated using @p dmaStreamAlloc().
 * @post    After use the stream can be released using @p dmaStreamRelease().
 *
 * @param[in] dmastp    pointer to a ch32_dma_stream_t structure
 * @param[in] addr      value to be written in the CMAR register
 *
 * @special
 */
#define dmaStreamSetMemory0(dmastp, addr) {                                 \
  (dmastp)->channel->MADDR = (uint32_t)(addr);                               \
}

/**
 * @brief   Sets the number of transfers to be performed.
 * @note    This function can be invoked in both ISR or thread context.
 * @pre     The stream must have been allocated using @p dmaStreamAlloc().
 * @post    After use the stream can be released using @p dmaStreamRelease().
 *
 * @param[in] dmastp    pointer to a ch32_dma_stream_t structure
 * @param[in] size      value to be written in the CNDTR register
 *
 * @special
 */
#define dmaStreamSetTransactionSize(dmastp, size) {                         \
  (dmastp)->channel->CNTR = (uint32_t)(size);                              \
}

/**
 * @brief   Returns the number of transfers to be performed.
 * @note    This function can be invoked in both ISR or thread context.
 * @pre     The stream must have been allocated using @p dmaStreamAlloc().
 * @post    After use the stream can be released using @p dmaStreamRelease().
 *
 * @param[in] dmastp    pointer to a ch32_dma_stream_t structure
 * @return              The number of transfers to be performed.
 *
 * @special
 */
#define dmaStreamGetTransactionSize(dmastp) ((size_t)((dmastp)->channel->CNTR))

/**
 * @brief   Programs the stream mode settings.
 * @note    This function can be invoked in both ISR or thread context.
 * @pre     The stream must have been allocated using @p dmaStreamAlloc().
 * @post    After use the stream can be released using @p dmaStreamRelease().
 *
 * @param[in] dmastp    pointer to a ch32_dma_stream_t structure
 * @param[in] mode      value to be written in the CCR register
 *
 * @special
 */
#if CH32_DMA_SUPPORTS_CSELR || defined(__DOXYGEN__)
#define dmaStreamSetMode(dmastp, mode) {                                    \
  uint32_t cselr = *(dmastp)->cselr;                                        \
  cselr &= ~(0x0000000FU << (dmastp)->shift);                               \
  cselr |=  (((uint32_t)(mode) >> 16U) << (dmastp)->shift);                 \
  *(dmastp)->cselr = cselr;                                                 \
  (dmastp)->channel->CCR  = (uint32_t)(mode);                               \
}
#else
#define dmaStreamSetMode(dmastp, mode)                                                                                 \
    {                                                                                                                  \
        (dmastp)->channel->CFGR =                                                                                      \
            (uint32_t)(mode);                                                                                          \
    }
#endif

/**
 * @brief   DMA stream enable.
 * @note    This function can be invoked in both ISR or thread context.
 * @pre     The stream must have been allocated using @p dmaStreamAlloc().
 * @post    After use the stream can be released using @p dmaStreamRelease().
 *
 * @param[in] dmastp    pointer to a ch32_dma_stream_t structure
 *
 * @special
 */
#define dmaStreamEnable(dmastp) {                                           \
  (dmastp)->channel->CFGR |= DMA_CFGR1_EN;                                \
}

/**
 * @brief   DMA stream disable.
 * @details The function disables the specified stream and then clears any
 *          pending interrupt.
 * @note    This function can be invoked in both ISR or thread context.
 * @note    Interrupts enabling flags are set to zero after this call, see
 *          bug 3607518.
 * @pre     The stream must have been allocated using @p dmaStreamAlloc().
 * @post    After use the stream can be released using @p dmaStreamRelease().
 *
 * @param[in] dmastp    pointer to a ch32_dma_stream_t structure
 *
 * @special
 */
#define dmaStreamDisable(dmastp) {                                          \
  (dmastp)->channel->CFGR &= ~(DMA_CFGR1_TCIE | DMA_CFGR1_HTIE |       \
                               DMA_CFGR1_TEIE | DMA_CFGR1_EN);         \
  dmaStreamClearInterrupt(dmastp);                                          \
}

/**
 * @brief   DMA stream interrupt sources clear.
 * @note    This function can be invoked in both ISR or thread context.
 * @pre     The stream must have been allocated using @p dmaStreamAlloc().
 * @post    After use the stream can be released using @p dmaStreamRelease().
 *
 * @param[in] dmastp    pointer to a ch32_dma_stream_t structure
 *
 * @special
 */
#define dmaStreamClearInterrupt(dmastp) {                                   \
  (dmastp)->dma->INTFCR = CH32_DMA_ISR_MASK << (dmastp)->shift;              \
}

/**
 * @brief   Starts a memory to memory operation using the specified stream.
 * @note    The default transfer data mode is "byte to byte" but it can be
 *          changed by specifying extra options in the @p mode parameter.
 * @pre     The stream must have been allocated using @p dmaStreamAlloc().
 * @post    After use the stream can be released using @p dmaStreamRelease().
 *
 * @param[in] dmastp    pointer to a ch32_dma_stream_t structure
 * @param[in] mode      value to be written in the CCR register, this value
 *                      is implicitly ORed with:
 *                      - @p DMA_CFGR1_MINC
 *                      - @p DMA_CFGR1_PINC
 *                      - @p DMA_CFGR1_DIR_M2M
 *                      - @p DMA_CFGR1_EN
 *                      .
 * @param[in] src       source address
 * @param[in] dst       destination address
 * @param[in] n         number of data units to copy
 */
#define dmaStartMemCopy(dmastp, mode, src, dst, n) {                        \
  dmaStreamSetPeripheral(dmastp, src);                                      \
  dmaStreamSetMemory0(dmastp, dst);                                         \
  dmaStreamSetTransactionSize(dmastp, n);                                   \
  dmaStreamSetMode(dmastp, (mode) |                                         \
                           DMA_CFGR1_MINC | DMA_CFGR1_PINC |          \
                           DMA_CFGR1_DIR_M2M | DMA_CFGR1_EN);         \
}

/**
 * @brief   Polled wait for DMA transfer end.
 * @pre     The stream must have been allocated using @p dmaStreamAlloc().
 * @post    After use the stream can be released using @p dmaStreamRelease().
 *
 * @param[in] dmastp    pointer to a ch32_dma_stream_t structure
 */
#define dmaWaitCompletion(dmastp) {                                         \
  while ((dmastp)->channel->CNTR > 0U)                                     \
    ;                                                                       \
  dmaStreamDisable(dmastp);                                                 \
}
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(__DOXYGEN__)
extern const ch32_dma_stream_t _ch32_dma_streams[CH32_DMA_STREAMS];
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void dmaInit(void);
  const ch32_dma_stream_t *dmaStreamAllocI(uint32_t id,
                                            uint32_t priority,
                                            ch32_dmaisr_t func,
                                            void *param);
  const ch32_dma_stream_t *dmaStreamAlloc(uint32_t id,
                                           uint32_t priority,
                                           ch32_dmaisr_t func,
                                           void *param);
  void dmaStreamFreeI(const ch32_dma_stream_t *dmastp);
  void dmaStreamFree(const ch32_dma_stream_t *dmastp);
  void dmaServeInterrupt(const ch32_dma_stream_t *dmastp);
#if CH32_DMA_SUPPORTS_DMAMUX == TRUE
  void dmaSetRequestSource(const ch32_dma_stream_t *dmastp, uint32_t per);
#endif
#ifdef __cplusplus
}
#endif

#endif /* CH32_DMA_H */

/** @} */
