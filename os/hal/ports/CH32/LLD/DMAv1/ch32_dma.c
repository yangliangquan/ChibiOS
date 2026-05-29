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
 * @file    DMAv1/ch32_dma.c
 * @brief   DMA helper driver code.
 *
 * @addtogroup CH32_DMA
 * @details DMA sharing helper driver. In the CH32 the DMA streams are a
 *          shared resource, this driver allows to allocate and free DMA
 *          streams at runtime in order to allow all the other device
 *          drivers to coordinate the access to the resource.
 * @note    The DMA ISR handlers are all declared into this module because
 *          sharing, the various device drivers can associate a callback to
 *          ISRs when allocating streams.
 * @{
 */

#include "hal.h"

/* The following macro is only defined if some driver requiring DMA services
   has been enabled.*/
#if defined(CH32_DMA_REQUIRED) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/**
 * @brief   Mask of the DMA1 streams in @p dma_streams_mask.
 */
#define CH32_DMA1_STREAMS_MASK     ((1U << CH32_DMA1_NUM_CHANNELS) - 1U)

/**
 * @brief   Mask of the DMA2 streams in @p dma_streams_mask.
 */
#define CH32_DMA2_STREAMS_MASK     (((1U << CH32_DMA2_NUM_CHANNELS) -     \
                                      1U) << CH32_DMA1_NUM_CHANNELS)

#if CH32_DMA_SUPPORTS_CSELR == TRUE

#if defined(DMA1_CSELR)
#define __DMA1_CSELR                &DMA1_CSELR->CSELR
#else
#define __DMA1_CSELR                &DMA1->CSELR
#endif

#if defined(DMA2_CSELR)
#define __DMA2_CSELR                &DMA2_CSELR->CSELR
#else
#define __DMA2_CSELR                &DMA2->CSELR
#endif

#define DMA1_CH1_VARIANT            __DMA1_CSELR
#define DMA1_CH2_VARIANT            __DMA1_CSELR
#define DMA1_CH3_VARIANT            __DMA1_CSELR
#define DMA1_CH4_VARIANT            __DMA1_CSELR
#define DMA1_CH5_VARIANT            __DMA1_CSELR
#define DMA1_CH6_VARIANT            __DMA1_CSELR
#define DMA1_CH7_VARIANT            __DMA1_CSELR
#define DMA1_CH8_VARIANT            __DMA1_CSELR
#define DMA2_CH1_VARIANT            __DMA2_CSELR
#define DMA2_CH2_VARIANT            __DMA2_CSELR
#define DMA2_CH3_VARIANT            __DMA2_CSELR
#define DMA2_CH4_VARIANT            __DMA2_CSELR
#define DMA2_CH5_VARIANT            __DMA2_CSELR
#define DMA2_CH6_VARIANT            __DMA2_CSELR
#define DMA2_CH7_VARIANT            __DMA2_CSELR
#define DMA2_CH8_VARIANT            __DMA2_CSELR

#elif CH32_DMA_SUPPORTS_DMAMUX == TRUE

#define DMA1_CH1_VARIANT            (DMAMUX)
#define DMA1_CH2_VARIANT            (DMAMUX)
#define DMA1_CH3_VARIANT            (DMAMUX)
#define DMA1_CH4_VARIANT            (DMAMUX)
#define DMA1_CH5_VARIANT            (DMAMUX)
#define DMA1_CH6_VARIANT            (DMAMUX)
#define DMA1_CH7_VARIANT            (DMAMUX)
#define DMA1_CH8_VARIANT            (DMAMUX)
#define DMA2_CH1_VARIANT            (DMAMUX)
#define DMA2_CH2_VARIANT            (DMAMUX)
#define DMA2_CH3_VARIANT            (DMAMUX)
#define DMA2_CH4_VARIANT            (DMAMUX)
#define DMA2_CH5_VARIANT            (DMAMUX)
#define DMA2_CH6_VARIANT            (DMAMUX)
#define DMA2_CH7_VARIANT            (DMAMUX)
#define DMA2_CH8_VARIANT            (DMAMUX)

#else /* !(CH32_DMA_SUPPORTS_DMAMUX == TRUE) */

#define DMA1_CH1_VARIANT            0
#define DMA1_CH2_VARIANT            0
#define DMA1_CH3_VARIANT            0
#define DMA1_CH4_VARIANT            0
#define DMA1_CH5_VARIANT            0
#define DMA1_CH6_VARIANT            0
#define DMA1_CH7_VARIANT            0
#define DMA2_CH1_VARIANT            0
#define DMA2_CH2_VARIANT            0
#define DMA2_CH3_VARIANT            0
#define DMA2_CH4_VARIANT            0
#define DMA2_CH5_VARIANT            0
#define DMA2_CH6_VARIANT            0
#define DMA2_CH7_VARIANT            0

#endif /* !(CH32_DMA_SUPPORTS_DMAMUX == TRUE) */

/*
 * Default ISR collision masks.
 */
#if !defined(CH32_DMA1_CH1_CMASK)
#define CH32_DMA1_CH1_CMASK        (1U << 0U)
#endif

#if !defined(CH32_DMA1_CH2_CMASK)
#define CH32_DMA1_CH2_CMASK        (1U << 1U)
#endif

#if !defined(CH32_DMA1_CH3_CMASK)
#define CH32_DMA1_CH3_CMASK        (1U << 2U)
#endif

#if !defined(CH32_DMA1_CH4_CMASK)
#define CH32_DMA1_CH4_CMASK        (1U << 3U)
#endif

#if !defined(CH32_DMA1_CH5_CMASK)
#define CH32_DMA1_CH5_CMASK        (1U << 4U)
#endif

#if !defined(CH32_DMA1_CH6_CMASK)
#define CH32_DMA1_CH6_CMASK        (1U << 5U)
#endif

#if !defined(CH32_DMA1_CH7_CMASK)
#define CH32_DMA1_CH7_CMASK        (1U << 6U)
#endif

#if !defined(CH32_DMA1_CH8_CMASK)
#define CH32_DMA1_CH8_CMASK        (1U << 7U)
#endif

#if !defined(CH32_DMA2_CH1_CMASK)
#define CH32_DMA2_CH1_CMASK        (1U << (CH32_DMA1_NUM_CHANNELS + 0U))
#endif

#if !defined(CH32_DMA2_CH2_CMASK)
#define CH32_DMA2_CH2_CMASK        (1U << (CH32_DMA1_NUM_CHANNELS + 1U))
#endif

#if !defined(CH32_DMA2_CH3_CMASK)
#define CH32_DMA2_CH3_CMASK        (1U << (CH32_DMA1_NUM_CHANNELS + 2U))
#endif

#if !defined(CH32_DMA2_CH4_CMASK)
#define CH32_DMA2_CH4_CMASK        (1U << (CH32_DMA1_NUM_CHANNELS + 3U))
#endif

#if !defined(CH32_DMA2_CH5_CMASK)
#define CH32_DMA2_CH5_CMASK        (1U << (CH32_DMA1_NUM_CHANNELS + 4U))
#endif

#if !defined(CH32_DMA2_CH6_CMASK)
#define CH32_DMA2_CH6_CMASK        (1U << (CH32_DMA1_NUM_CHANNELS + 5U))
#endif

#if !defined(CH32_DMA2_CH7_CMASK)
#define CH32_DMA2_CH7_CMASK        (1U << (CH32_DMA1_NUM_CHANNELS + 6U))
#endif

#if !defined(CH32_DMA2_CH8_CMASK)
#define CH32_DMA2_CH8_CMASK        (1U << (CH32_DMA1_NUM_CHANNELS + 7U))
#endif

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   DMA streams descriptors.
 * @details This table keeps the association between an unique stream
 *          identifier and the involved physical registers.
 * @note    Don't use this array directly, use the appropriate wrapper macros
 *          instead: @p CH32_DMA1_STREAM1, @p CH32_DMA1_STREAM2 etc.
 */
const ch32_dma_stream_t _ch32_dma_streams[CH32_DMA_STREAMS] = {
#if CH32_DMA1_NUM_CHANNELS > 0
  {DMA1, DMA1_Channel1, CH32_DMA1_CH1_CMASK, DMA1_CH1_VARIANT,  0, 0, CH32_DMA1_CH1_NUMBER},
#endif
#if CH32_DMA1_NUM_CHANNELS > 1
  {DMA1, DMA1_Channel2, CH32_DMA1_CH2_CMASK, DMA1_CH2_VARIANT,  4, 1, CH32_DMA1_CH2_NUMBER},
#endif
#if CH32_DMA1_NUM_CHANNELS > 2
  {DMA1, DMA1_Channel3, CH32_DMA1_CH3_CMASK, DMA1_CH3_VARIANT,  8, 2, CH32_DMA1_CH3_NUMBER},
#endif
#if CH32_DMA1_NUM_CHANNELS > 3
  {DMA1, DMA1_Channel4, CH32_DMA1_CH4_CMASK, DMA1_CH4_VARIANT, 12, 3, CH32_DMA1_CH4_NUMBER},
#endif
#if CH32_DMA1_NUM_CHANNELS > 4
  {DMA1, DMA1_Channel5, CH32_DMA1_CH5_CMASK, DMA1_CH5_VARIANT, 16, 4, CH32_DMA1_CH5_NUMBER},
#endif
#if CH32_DMA1_NUM_CHANNELS > 5
  {DMA1, DMA1_Channel6, CH32_DMA1_CH6_CMASK, DMA1_CH6_VARIANT, 20, 5, CH32_DMA1_CH6_NUMBER},
#endif
#if CH32_DMA1_NUM_CHANNELS > 6
  {DMA1, DMA1_Channel7, CH32_DMA1_CH7_CMASK, DMA1_CH7_VARIANT, 24, 6, CH32_DMA1_CH7_NUMBER},
#endif
#if CH32_DMA1_NUM_CHANNELS > 7
  {DMA1, DMA1_Channel8, CH32_DMA1_CH8_CMASK, DMA1_CH8_VARIANT, 28, 7, CH32_DMA1_CH8_NUMBER},
#endif
#if CH32_DMA2_NUM_CHANNELS > 0
  {DMA2, DMA2_Channel1, CH32_DMA2_CH1_CMASK, DMA2_CH1_VARIANT,  0, 0 + CH32_DMA1_NUM_CHANNELS, CH32_DMA2_CH1_NUMBER},
#endif
#if CH32_DMA2_NUM_CHANNELS > 1
  {DMA2, DMA2_Channel2, CH32_DMA2_CH2_CMASK, DMA2_CH2_VARIANT,  4, 1 + CH32_DMA1_NUM_CHANNELS, CH32_DMA2_CH2_NUMBER},
#endif
#if CH32_DMA2_NUM_CHANNELS > 2
  {DMA2, DMA2_Channel3, CH32_DMA2_CH3_CMASK, DMA2_CH3_VARIANT,  8, 2 + CH32_DMA1_NUM_CHANNELS, CH32_DMA2_CH3_NUMBER},
#endif
#if CH32_DMA2_NUM_CHANNELS > 3
  {DMA2, DMA2_Channel4, CH32_DMA2_CH4_CMASK, DMA2_CH4_VARIANT, 12, 3 + CH32_DMA1_NUM_CHANNELS, CH32_DMA2_CH4_NUMBER},
#endif
#if CH32_DMA2_NUM_CHANNELS > 4
  {DMA2, DMA2_Channel5, CH32_DMA2_CH5_CMASK, DMA2_CH5_VARIANT, 16, 4 + CH32_DMA1_NUM_CHANNELS, CH32_DMA2_CH5_NUMBER},
#endif
#if CH32_DMA2_NUM_CHANNELS > 5
  {DMA2, DMA2_Channel6, CH32_DMA2_CH6_CMASK, DMA2_CH6_VARIANT, 20, 5 + CH32_DMA1_NUM_CHANNELS, CH32_DMA2_CH6_NUMBER},
#endif
#if CH32_DMA2_NUM_CHANNELS > 6
  {DMA2, DMA2_Channel7, CH32_DMA2_CH7_CMASK, DMA2_CH7_VARIANT, 24, 6 + CH32_DMA1_NUM_CHANNELS, CH32_DMA2_CH7_NUMBER},
#endif
#if CH32_DMA2_NUM_CHANNELS > 7
  {DMA2, DMA2_Channel8, CH32_DMA2_CH8_CMASK, DMA2_CH8_VARIANT, 28, 7 + CH32_DMA1_NUM_CHANNELS, CH32_DMA2_CH8_NUMBER},
#endif
};

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Global DMA-related data structures.
 */
static struct {
  /**
   * @brief   Mask of the allocated streams.
   */
  uint32_t          allocated_mask;
  /**
   * @brief   Mask of the enabled streams ISRs.
   */
  uint32_t          isr_mask;
  /**
   * @brief   DMA IRQ redirectors.
   */
  struct {
    /**
     * @brief   DMA callback function.
     */
    ch32_dmaisr_t    func;
    /**
     * @brief   DMA callback parameter.
     */
    void              *param;
  } streams[CH32_DMA_STREAMS];
} dma;

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if defined(CH32_DMA1_CH1_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA1 stream 1 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CH32_DMA1_CH1_HANDLER) {

  dmaServeInterrupt(CH32_DMA1_STREAM1);

}
#endif

#if defined(CH32_DMA1_CH2_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA1 stream 2 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CH32_DMA1_CH2_HANDLER) {

  dmaServeInterrupt(CH32_DMA1_STREAM2);

}
#endif

#if defined(CH32_DMA1_CH3_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA1 stream 3 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CH32_DMA1_CH3_HANDLER) {

  dmaServeInterrupt(CH32_DMA1_STREAM3);

}
#endif

#if defined(CH32_DMA1_CH4_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA1 stream 4 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CH32_DMA1_CH4_HANDLER) {

  dmaServeInterrupt(CH32_DMA1_STREAM4);

}
#endif

#if defined(CH32_DMA1_CH5_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA1 stream 5 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CH32_DMA1_CH5_HANDLER) {

  dmaServeInterrupt(CH32_DMA1_STREAM5);

}
#endif

#if defined(CH32_DMA1_CH6_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA1 stream 6 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CH32_DMA1_CH6_HANDLER) {

  dmaServeInterrupt(CH32_DMA1_STREAM6);

}
#endif

#if defined(CH32_DMA1_CH7_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA1 stream 7 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CH32_DMA1_CH7_HANDLER) {

  dmaServeInterrupt(CH32_DMA1_STREAM7);

}
#endif

#if defined(CH32_DMA1_CH8_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA1 stream 8 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CH32_DMA1_CH8_HANDLER) {

  dmaServeInterrupt(CH32_DMA1_STREAM8);

}
#endif

#if defined(CH32_DMA2_CH1_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA2 stream 1 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CH32_DMA2_CH1_HANDLER) {

  dmaServeInterrupt(CH32_DMA2_STREAM1);

}
#endif

#if defined(CH32_DMA2_CH2_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA2 stream 2 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CH32_DMA2_CH2_HANDLER) {

  dmaServeInterrupt(CH32_DMA2_STREAM2);

}
#endif

#if defined(CH32_DMA2_CH3_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA2 stream 3 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CH32_DMA2_CH3_HANDLER) {

  dmaServeInterrupt(CH32_DMA2_STREAM3);

}
#endif

#if defined(CH32_DMA2_CH4_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA2 stream 4 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CH32_DMA2_CH4_HANDLER) {

  dmaServeInterrupt(CH32_DMA2_STREAM4);

}
#endif

#if defined(CH32_DMA2_CH5_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA2 stream 5 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CH32_DMA2_CH5_HANDLER) {

  dmaServeInterrupt(CH32_DMA2_STREAM5);

}
#endif

#if defined(CH32_DMA2_CH6_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA2 stream 6 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CH32_DMA2_CH6_HANDLER) {

  dmaServeInterrupt(CH32_DMA2_STREAM6);

}
#endif

#if defined(CH32_DMA2_CH7_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA2 stream 7 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CH32_DMA2_CH7_HANDLER) {

  dmaServeInterrupt(CH32_DMA2_STREAM7);

}
#endif

#if defined(CH32_DMA2_CH8_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   DMA2 stream 8 shared ISR.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CH32_DMA2_CH8_HANDLER) {

  dmaServeInterrupt(CH32_DMA2_STREAM8);

}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   CH32 DMA helper initialization.
 *
 * @init
 */
void dmaInit(void) {
  int i;

  dma.allocated_mask = 0U;
  dma.isr_mask       = 0U;
  for (i = 0; i < CH32_DMA_STREAMS; i++) {
    _ch32_dma_streams[i].channel->CFGR = 0;
    dma.streams[i].func = NULL;
  }
  DMA1->INTFCR = 0xFFFFFFFFU;
#if CH32_DMA2_NUM_CHANNELS > 0
  DMA2->INTFCR = 0xFFFFFFFFU;
#endif
}

/**
 * @brief   Allocates a DMA stream.
 * @details The stream is allocated and, if required, the DMA clock enabled.
 *          The function also enables the IRQ vector associated to the stream
 *          and initializes its priority.
 *
 * @param[in] id        numeric identifiers of a specific stream or:
 *                      - @p CH32_DMA_STREAM_ID_ANY for any stream.
 *                      - @p CH32_DMA_STREAM_ID_ANY_DMA1 for any stream
 *                        on DMA1.
 *                      - @p CH32_DMA_STREAM_ID_ANY_DMA2 for any stream
 *                        on DMA2.
 *                      .
 * @param[in] priority  IRQ priority for the DMA stream
 * @param[in] func      handling function pointer, can be @p NULL
 * @param[in] param     a parameter to be passed to the handling function
 * @return              Pointer to the allocated @p ch32_dma_stream_t
 *                      structure.
 * @retval NULL         if a/the stream is not available.
 *
 * @iclass
 */
const ch32_dma_stream_t *dmaStreamAllocI(uint32_t id,
                                          uint32_t priority,
                                          ch32_dmaisr_t func,
                                          void *param) {
  uint32_t i, startid, endid;

  osalDbgCheckClassI();

  if (id < CH32_DMA_STREAMS) {
    startid = id;
    endid   = id;
  }
#if CH32_DMA_SUPPORTS_DMAMUX == TRUE
  else if (id == CH32_DMA_STREAM_ID_ANY) {
    startid = 0U;
    endid   = CH32_DMA_STREAMS - 1U;
  }
  else if (id == CH32_DMA_STREAM_ID_ANY_DMA1) {
    startid = 0U;
    endid   = CH32_DMA1_NUM_CHANNELS - 1U;
  }
#if CH32_DMA2_NUM_CHANNELS > 0
  else if (id == CH32_DMA_STREAM_ID_ANY_DMA2) {
    startid = CH32_DMA1_NUM_CHANNELS;
    endid   = CH32_DMA_STREAMS - 1U;
  }
#endif
#endif
  else {
    osalDbgCheck(false);
    return NULL;
  }

  for (i = startid; i <= endid; i++) {
    uint32_t mask = (1U << i);
    if ((dma.allocated_mask & mask) == 0U) {
      const ch32_dma_stream_t *dmastp = CH32_DMA_STREAM(i);

      /* Installs the DMA handler.*/
      dma.streams[i].func  = func;
      dma.streams[i].param = param;
      dma.allocated_mask  |= mask;

      /* Enabling DMA clocks required by the current streams set.*/
      if ((CH32_DMA1_STREAMS_MASK & mask) != 0U) {
        enableHB(RCC_DMA1EN);
      }
#if CH32_DMA2_NUM_CHANNELS > 0
      if ((CH32_DMA2_STREAMS_MASK & mask) != 0U) {
        enableHB(RCC_DMA2EN);
      }
#endif

#if (CH32_DMA_SUPPORTS_DMAMUX == TRUE) && defined(rccEnableDMAMUX)
      /* Enabling DMAMUX if present.*/
      if (dma.allocated_mask != 0U) {
        // Nothing shall do.
        // rccEnableDMAMUX(true);
      }
#endif

      /* Enables the associated IRQ vector if not already enabled and if a
         callback is defined.*/
      if (func != NULL) {
        if ((dma.isr_mask & dmastp->cmask) == 0U) {
          NVIC_SetPriority(dmastp->vector,priority);
          NVIC_EnableIRQ(dmastp->vector);
        }
        dma.isr_mask |= mask;
      }

      /* Putting the stream in a known state.*/
      dmaStreamDisable(dmastp);
      dmastp->channel->CFGR = 0;

      return dmastp;
    }
  }

  return NULL;
}

/**
 * @brief   Allocates a DMA stream.
 * @details The stream is allocated and, if required, the DMA clock enabled.
 *          The function also enables the IRQ vector associated to the stream
 *          and initializes its priority.
 *
 * @param[in] id        numeric identifiers of a specific stream or:
 *                      - @p CH32_DMA_STREAM_ID_ANY for any stream.
 *                      - @p CH32_DMA_STREAM_ID_ANY_DMA1 for any stream
 *                        on DMA1.
 *                      - @p CH32_DMA_STREAM_ID_ANY_DMA2 for any stream
 *                        on DMA2.
 *                      .
 * @param[in] priority  IRQ priority for the DMA stream
 * @param[in] func      handling function pointer, can be @p NULL
 * @param[in] param     a parameter to be passed to the handling function
 * @return              Pointer to the allocated @p ch32_dma_stream_t
 *                      structure.
 * @retval NULL         if a/the stream is not available.
 *
 * @api
 */
const ch32_dma_stream_t *dmaStreamAlloc(uint32_t id,
                                         uint32_t priority,
                                         ch32_dmaisr_t func,
                                         void *param) {
  const ch32_dma_stream_t *dmastp;

  osalSysLock();
  dmastp = dmaStreamAllocI(id, priority, func, param);
  osalSysUnlock();

  return dmastp;
}

/**
 * @brief   Releases a DMA stream.
 * @details The stream is freed and, if required, the DMA clock disabled.
 *          Trying to release a unallocated stream is an illegal operation
 *          and is trapped if assertions are enabled.
 *
 * @param[in] dmastp    pointer to a ch32_dma_stream_t structure
 *
 * @iclass
 */
void dmaStreamFreeI(const ch32_dma_stream_t *dmastp) {
  uint32_t selfindex = (uint32_t)dmastp->selfindex;

  osalDbgCheck(dmastp != NULL);

  /* Check if the streams is not taken.*/
  osalDbgAssert((dma.allocated_mask & (1 << selfindex)) != 0U,
                "not allocated");

  /* Marks the stream as not allocated.*/
  dma.allocated_mask &= ~(1U << selfindex);
  dma.isr_mask &= ~(1U << selfindex);

  /* Disables the associated IRQ vector if it is no more in use.*/
  if ((dma.isr_mask & dmastp->cmask) == 0U) {
    NVIC_EnableIRQ(dmastp->vector);
  }

  /* Removes the DMA handler.*/
  dma.streams[selfindex].func  = NULL;
  dma.streams[selfindex].param = NULL;

  /* Shutting down clocks that are no more required, if any.*/
  if ((dma.allocated_mask & CH32_DMA1_STREAMS_MASK) == 0U) {
    disableHB(RCC_DMA1EN);
  }
#if CH32_DMA2_NUM_CHANNELS > 0
  if ((dma.allocated_mask & CH32_DMA2_STREAMS_MASK) == 0U) {
    disableHB(RCC_DMA2EN);
  }
#endif

#if (CH32_DMA_SUPPORTS_DMAMUX == TRUE) && defined(rccDisableDMAMUX)
  /* Shutting down DMAMUX if present.*/
  if (dma.allocated_mask == 0U) {
    // Nothing shall do.
    // rccDisableDMAMUX();
  }
#endif
}

/**
 * @brief   Releases a DMA stream.
 * @details The stream is freed and, if required, the DMA clock disabled.
 *          Trying to release a unallocated stream is an illegal operation
 *          and is trapped if assertions are enabled.
 *
 * @param[in] dmastp    pointer to a ch32_dma_stream_t structure
 *
 * @api
 */
void dmaStreamFree(const ch32_dma_stream_t *dmastp) {

  osalSysLock();
  dmaStreamFreeI(dmastp);
  osalSysUnlock();
}

/**
 * @brief   Serves a DMA IRQ.
 *
 * @param[in] dmastp    pointer to a ch32_dma_stream_t structure
 *
 * @special
 */
void dmaServeInterrupt(const ch32_dma_stream_t *dmastp) {
  uint32_t flags;
  uint32_t selfindex = (uint32_t)dmastp->selfindex;

  flags = (dmastp->dma->INTFR >> dmastp->shift) & CH32_DMA_ISR_MASK;
  if (flags & (dmastp->channel->CFGR)) {
    dmastp->dma->INTFCR = 0xf << dmastp->shift;
    if (dma.streams[selfindex].func) {
      dma.streams[selfindex].func(dma.streams[selfindex].param, flags);
    }
  }
}

#if (CH32_DMA_SUPPORTS_DMAMUX == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   Associates a peripheral request to a DMA stream.
 * @note    This function can be invoked in both ISR or thread context.
 *
 * @param[in] dmastp    pointer to a @p ch32_dma_stream_t structure
 * @param[in] per       peripheral identifier
 *
 * @special
 */
void dmaSetRequestSource(const ch32_dma_stream_t *dmastp, uint32_t per) {

  osalDbgCheck(per < 256U);

  *(uint32_t *)(((uint32_t)&(dmastp->mux->CFGR0_3)) + ((dmastp->selfindex / 4) * 4)) =
      ((*(uint32_t *)(((uint32_t)&(dmastp->mux->CFGR0_3)) + ((dmastp->selfindex / 4) * 4))) & ~(0b1111111 << ((dmastp->selfindex % 4) * 8))) |
      ((per - 1) << ((dmastp->selfindex % 4) * 8));
}
#endif

#endif /* CH32_DMA_REQUIRED */

/** @} */
