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
 * @file    hal_dac_lld.c
 * @brief   CH32 DAC subsystem low level driver source.
 *
 * @addtogroup DAC
 * @{
 */

#include "hal.h"

#if (HAL_USE_DAC == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief DAC1 CH1 driver identifier.*/
#if (CH32_DAC_USE_DAC1_CH1 == TRUE) || defined(__DOXYGEN__)
DACDriver DACD1;
#endif

/** @brief DAC1 CH2 driver identifier.*/
#if (CH32_DAC_USE_DAC1_CH2 == TRUE) || defined(__DOXYGEN__)
DACDriver DACD2;
#endif

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

#if CH32_DAC_USE_DAC1_CH1 == TRUE
static const dacparams_t dac1_ch1_params = {
  .channel      = 0U,
  .dmastream    = CH32_DMA_STREAM_ID_ANY,
  .dmamux       = DMA_MUX_DAC1,
  .dmamode      = DMA_DIR_PeripheralDST |
                  DMA_MemoryInc_Enable |
                  DMA_PeripheralInc_Disable |
                  DMA_Mode_Circular |
                  DMA_PeripheralDataSize_HalfWord |
                  DMA_MemoryDataSize_HalfWord |
                  (DMA_CFGR1_TCIE | DMA_CFGR1_HTIE | DMA_CFGR1_TEIE),
  .dmairqprio   = CH32_DAC_DAC1_CH1_IRQ_PRIORITY
};
#endif

#if CH32_DAC_USE_DAC1_CH2 == TRUE
static const dacparams_t dac1_ch2_params = {
  .channel      = 1U,
  .dmastream    = CH32_DMA_STREAM_ID_ANY,
  .dmamux       = DMA_MUX_DAC2,
  .dmamode      = DMA_DIR_PeripheralDST |
                  DMA_MemoryInc_Enable |
                  DMA_PeripheralInc_Disable |
                  DMA_Mode_Circular |
                  DMA_PeripheralDataSize_HalfWord |
                  DMA_MemoryDataSize_HalfWord |
                  (DMA_CFGR1_TCIE | DMA_CFGR1_HTIE | DMA_CFGR1_TEIE),
  .dmairqprio   = CH32_DAC_DAC1_CH2_IRQ_PRIORITY
};
#endif

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Shared DMA end/half-tx service routine.
 *
 * @param[in] dacp      pointer to the @p DACDriver object
 * @param[in] flags     pre-shifted content of the ISR register
 */
static void dac_lld_serve_dma_interrupt(DACDriver *dacp, uint32_t flags) {

  if ((flags & (DMA_TEIF1)) != 0) {
    /* DMA errors handling.*/
    dac_lld_stop_conversion(dacp);
    _dac_isr_error_code(dacp, DAC_ERR_DMAFAILURE);
  }
  else {
    /* It is possible that the conversion group has already been reset by the
       DAC error handler, in this case this interrupt is spurious.*/
    if (dacp->grpp != NULL) {
      if ((flags & DMA_TCIF1) != 0) {
        /* Transfer complete processing.*/
        _dac_isr_full_code(dacp);
      }
      else if ((flags & DMA_HTIF1) != 0) {
        /* Half transfer processing.*/
        _dac_isr_half_code(dacp);
      }
    }
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level DAC driver initialization.
 *
 * @notapi
 */
void dac_lld_init(void) {

#if CH32_DAC_USE_DAC1_CH1 == TRUE
  dacObjectInit(&DACD1);
  DACD1.params = &dac1_ch1_params;
  DACD1.dma = NULL;
  resetHB1(RCC_DACRST);
#endif

#if CH32_DAC_USE_DAC1_CH2 == TRUE
  dacObjectInit(&DACD2);
  DACD2.params = &dac1_ch2_params;
  DACD2.dma = NULL;
#endif
}

/**
 * @brief   Configures and activates the DAC peripheral.
 *
 * @param[in] dacp      pointer to the @p DACDriver object
 *
 * @notapi
 */
void dac_lld_start(DACDriver *dacp) {

  /* If the driver is in DAC_STOP state then a full initialization is
     required.*/
  if (dacp->state == DAC_STOP) {
    /* Enabling the clock source.*/
    enableHB1(RCC_DACEN);
    resetHB1(RCC_DACRST);

    /* Apply configuration from the cr field.
       The user-supplied cr value contains the desired BOFF, TEN, TSEL,
       WAVE and MAMP bits for the channel. The EN and DMAEN bits are
       managed by the LLD internally and are not taken from cr. */
    if (dacp->params->channel == 0U) {
      /* Clear CH1 configurable bits, then set from cr (minus EN1/DMAEN1). */
      DAC->CTLR &= ~(DAC_BOFF1 | DAC_TEN1 | DAC_TSEL1 |
                     DAC_WAVE1 | DAC_MAMP1);
      DAC->CTLR |= dacp->config->cr & (DAC_BOFF1 | DAC_TEN1 | DAC_TSEL1 |
                                        DAC_WAVE1 | DAC_MAMP1);
      /* Enable the channel. */
      DAC->CTLR |= DAC_EN1;
    }
    else {
      /* Clear CH2 configurable bits, then set from cr (minus EN2/DMAEN2). */
      DAC->CTLR &= ~(DAC_BOFF2 | DAC_TEN2 | DAC_TSEL2 |
                     DAC_WAVE2 | DAC_MAMP2);
      DAC->CTLR |= dacp->config->cr & (DAC_BOFF2 | DAC_TEN2 | DAC_TSEL2 |
                                        DAC_WAVE2 | DAC_MAMP2);
      /* Enable the channel. */
      DAC->CTLR |= DAC_EN2;
    }

    /* Set initial output value.*/
    dac_lld_put_channel(dacp, dacp->params->channel,
                        (dacsample_t)dacp->config->init);
  }
}

/**
 * @brief   Deactivates the DAC peripheral.
 *
 * @param[in] dacp      pointer to the @p DACDriver object
 *
 * @notapi
 */
void dac_lld_stop(DACDriver *dacp) {

  /* If in ready state then disables the DAC clock.*/
  if (dacp->state == DAC_READY) {

    /* Disable the DAC channel.*/
    if (dacp->params->channel == 0U) {
      DAC->CTLR &= ~DAC_EN1;
    }
    else {
      DAC->CTLR &= ~DAC_EN2;
    }

    /* Disable DAC clock if both channels are disabled.*/
    if ((DAC->CTLR & (DAC_EN1 | DAC_EN2)) == 0U) {
      disableHB1(RCC_DACEN);
    }
  }
}

/**
 * @brief   Outputs a value directly on a DAC channel.
 *
 * @param[in] dacp      pointer to the @p DACDriver object
 * @param[in] channel   DAC channel number
 * @param[in] sample    value to be output
 *
 * @api
 */
void dac_lld_put_channel(DACDriver *dacp,
                         dacchannel_t channel,
                         dacsample_t sample) {

  (void)dacp;

  switch (dacp->config->datamode) {
  case DAC_DHRM_12BIT_RIGHT:
    if (channel == 0U) {
      DAC->R12BDHR1 = (uint32_t)sample;
    }
    else {
      DAC->R12BDHR2 = (uint32_t)sample;
    }
    break;
  case DAC_DHRM_12BIT_LEFT:
    if (channel == 0U) {
      DAC->L12BDHR1 = (uint32_t)sample;
    }
    else {
      DAC->L12BDHR2 = (uint32_t)sample;
    }
    break;
  case DAC_DHRM_8BIT_RIGHT:
    if (channel == 0U) {
      DAC->R8BDHR1 = (uint32_t)sample;
    }
    else {
      DAC->R8BDHR2 = (uint32_t)sample;
    }
    break;
  default:
    osalDbgAssert(false, "unexpected DAC mode");
    break;
  }

  /* Trigger software conversion to update the output.*/
  if (channel == 0U) {
    DAC->SWTR |= DAC_SWTRIG1;
  }
  else {
    DAC->SWTR |= DAC_SWTRIG2;
  }
}

/**
 * @brief   Starts a DAC conversion.
 * @details Starts an asynchronous conversion operation.
 *
 * @param[in] dacp      pointer to the @p DACDriver object
 *
 * @notapi
 */
void dac_lld_start_conversion(DACDriver *dacp) {
  uint32_t n;
  uint32_t dmamode;

  /* Number of DMA operations per buffer.*/
  n = dacp->depth * dacp->grpp->num_channels;

  /* Allocating the DMA channel.*/
  dacp->dma = dmaStreamAllocI(dacp->params->dmastream,
                              dacp->params->dmairqprio,
                              (ch32_dmaisr_t)dac_lld_serve_dma_interrupt,
                              (void *)dacp);
  osalDbgAssert(dacp->dma != NULL, "unable to allocate stream");

#if CH32_DMA_SUPPORTS_DMAMUX
  dmaSetRequestSource(dacp->dma, dacp->params->dmamux);
#endif

  /* Set up DMA based on data mode.*/
  switch (dacp->config->datamode) {
  case DAC_DHRM_12BIT_RIGHT:
    osalDbgAssert(dacp->grpp->num_channels == 1, "invalid number of channels");
    if (dacp->params->channel == 0U) {
      dmaStreamSetPeripheral(dacp->dma, &DAC->R12BDHR1);
    }
    else {
      dmaStreamSetPeripheral(dacp->dma, &DAC->R12BDHR2);
    }
    dmamode = dacp->params->dmamode |
              DMA_PeripheralDataSize_HalfWord |
              DMA_MemoryDataSize_HalfWord;
    break;
  case DAC_DHRM_12BIT_LEFT:
    osalDbgAssert(dacp->grpp->num_channels == 1, "invalid number of channels");
    if (dacp->params->channel == 0U) {
      dmaStreamSetPeripheral(dacp->dma, &DAC->L12BDHR1);
    }
    else {
      dmaStreamSetPeripheral(dacp->dma, &DAC->L12BDHR2);
    }
    dmamode = dacp->params->dmamode |
              DMA_PeripheralDataSize_HalfWord |
              DMA_MemoryDataSize_HalfWord;
    break;
  case DAC_DHRM_8BIT_RIGHT:
    osalDbgAssert(dacp->grpp->num_channels == 1, "invalid number of channels");
    if (dacp->params->channel == 0U) {
      dmaStreamSetPeripheral(dacp->dma, &DAC->R8BDHR1);
    }
    else {
      dmaStreamSetPeripheral(dacp->dma, &DAC->R8BDHR2);
    }
    dmamode = dacp->params->dmamode |
              DMA_PeripheralDataSize_Byte |
              DMA_MemoryDataSize_Byte;
    break;
  default:
    osalDbgAssert(false, "unexpected DAC mode");
    return;
  }

  dmaStreamSetMemory0(dacp->dma, dacp->samples);
  dmaStreamSetTransactionSize(dacp->dma, n);
  dmaStreamSetMode(dacp->dma, dmamode);
  dmaStreamEnable(dacp->dma);

  /* Enable DMA on the DAC channel and trigger via hardware (TIM) or
     software as configured.*/
  if (dacp->params->channel == 0U) {
    DAC->CTLR |= DAC_DMAEN1;
    DAC->CTLR |= DAC_TEN1;
  }
  else {
    DAC->CTLR |= DAC_DMAEN2;
    DAC->CTLR |= DAC_TEN2;
  }
}

/**
 * @brief   Stops an ongoing conversion.
 * @details This function stops the currently ongoing conversion and returns
 *          the driver in the @p DAC_READY state. If there was no conversion
 *          being processed then the function does nothing.
 *
 * @param[in] dacp      pointer to the @p DACDriver object
 *
 * @iclass
 */
void dac_lld_stop_conversion(DACDriver *dacp) {

  /* Disable DMA on the DAC channel.*/
  if (dacp->params->channel == 0U) {
    DAC->CTLR &= ~DAC_DMAEN1;
    DAC->CTLR &= ~DAC_TEN1;
  }
  else {
    DAC->CTLR &= ~DAC_DMAEN2;
    DAC->CTLR &= ~DAC_TEN2;
  }

  /* Free the DMA stream.*/
  if (dacp->dma != NULL) {
    dmaStreamFreeI(dacp->dma);
    dacp->dma = NULL;
  }
}

#endif /* HAL_USE_DAC == TRUE */

/** @} */
