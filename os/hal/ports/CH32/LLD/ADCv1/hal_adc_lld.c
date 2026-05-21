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
 * @file    hal_adc_lld.c
 * @brief   CH32 ADC subsystem low level driver source.
 *
 * @addtogroup ADC
 * @{
 */

#include "hal.h"

#if (HAL_USE_ADC == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/
/* ADC DISCNUM mask */
#define CTLR1_DISCNUM_Reset              ((uint32_t)0xFFFF1FFF)

/* ADC DISCEN mask */
#define CTLR1_DISCEN_Set                 ((uint32_t)0x00000800)
#define CTLR1_DISCEN_Reset               ((uint32_t)0xFFFFF7FF)

/* ADC JAUTO mask */
#define CTLR1_JAUTO_Set                  ((uint32_t)0x00000400)
#define CTLR1_JAUTO_Reset                ((uint32_t)0xFFFFFBFF)

/* ADC JDISCEN mask */
#define CTLR1_JDISCEN_Set                ((uint32_t)0x00001000)
#define CTLR1_JDISCEN_Reset              ((uint32_t)0xFFFFEFFF)

/* ADC AWDCH mask */
#define CTLR1_AWDCH_Reset                ((uint32_t)0xFFFFFFE0)

/* ADC Analog watchdog enable mode mask */
#define CTLR1_AWDMode_Reset              ((uint32_t)0xFF3FFDFF)

/* CTLR1 register Mask */
#define CTLR1_CLEAR_Mask                 ((uint32_t)0xE0F0FEFF)

/* ADC ADON mask */
#define CTLR2_ADON_Set                   ((uint32_t)0x00000001)
#define CTLR2_ADON_Reset                 ((uint32_t)0xFFFFFFFE)

/* ADC DMA mask */
#define CTLR2_DMA_Set                    ((uint32_t)0x00000100)
#define CTLR2_DMA_Reset                  ((uint32_t)0xFFFFFEFF)

/* ADC RSTCAL mask */
#define CTLR2_RSTCAL_Set                 ((uint32_t)0x00000008)

/* ADC CAL mask */
#define CTLR2_CAL_Set                    ((uint32_t)0x00000004)

/* ADC SWSTART mask */
#define CTLR2_SWSTART_Set                ((uint32_t)0x00400000)

/* ADC EXTTRIG mask */
#define CTLR2_EXTTRIG_Set                ((uint32_t)0x00100000)
#define CTLR2_EXTTRIG_Reset              ((uint32_t)0xFFEFFFFF)

/* ADC Software start mask */
#define CTLR2_EXTTRIG_SWSTART_Set        ((uint32_t)0x00500000)
#define CTLR2_EXTTRIG_SWSTART_Reset      ((uint32_t)0xFFAFFFFF)

/* ADC JEXTSEL mask */
#define CTLR2_JEXTSEL_Reset              ((uint32_t)0xFFFF8FFF)

/* ADC JEXTTRIG mask */
#define CTLR2_JEXTTRIG_Set               ((uint32_t)0x00008000)
#define CTLR2_JEXTTRIG_Reset             ((uint32_t)0xFFFF7FFF)

/* ADC JSWSTART mask */
#define CTLR2_JSWSTART_Set               ((uint32_t)0x00200000)

/* ADC injected software start mask */
#define CTLR2_JEXTTRIG_JSWSTART_Set      ((uint32_t)0x00208000)
#define CTLR2_JEXTTRIG_JSWSTART_Reset    ((uint32_t)0xFFDF7FFF)

/* ADC TSPD mask */
#define CTLR2_TSVREFE_Set                ((uint32_t)0x00800000)
#define CTLR2_TSVREFE_Reset              ((uint32_t)0xFF7FFFFF)

/* CTLR2 register Mask */
#define CTLR2_CLEAR_Mask                 ((uint32_t)0xFFF1F7FD)

/* ADC SQx mask */
#define RSQR3_SQ_Set                     ((uint32_t)0x0000001F)
#define RSQR2_SQ_Set                     ((uint32_t)0x0000001F)
#define RSQR1_SQ_Set                     ((uint32_t)0x0000001F)

/* RSQR1 register Mask */
#define RSQR1_CLEAR_Mask                 ((uint32_t)0xFF0FFFFF)

/* ADC JSQx mask */
#define ISQR_JSQ_Set                     ((uint32_t)0x0000001F)

/* ADC JL mask */
#define ISQR_JL_Set                      ((uint32_t)0x00300000)
#define ISQR_JL_Reset                    ((uint32_t)0xFFCFFFFF)

/* ADC SMPx mask */
#define SAMPTR1_SMP_Set                  ((uint32_t)0x00000007)
#define SAMPTR2_SMP_Set                  ((uint32_t)0x00000007)

/* ADC IDATARx registers offset */
#define IDATAR_Offset                    ((uint8_t)0x28)

/* ADC1 RDATAR register base address */
#define RDATAR_ADDRESS                   ((uint32_t)0x4001244C)

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   ADC1 driver identifier.
 */
#if (CH32_ADC_USE_ADC1 == TRUE) || defined(__DOXYGEN__)
ADCDriver ADCD1;
#endif

/**
 * @brief   ADC2 driver identifier.
 */
#if (CH32_ADC_USE_ADC2 == TRUE) || defined(__DOXYGEN__)
ADCDriver ADCD2;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/
/**
 * @brief   ADC DMA service routine.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 * @param[in] flags     pre-shifted content of the ISR register
 */
static void adc_lld_serve_dma_interrupt(ADCDriver *adcp, uint32_t flags) {
  /* DMA errors handling.*/
  if ((flags & (DMA_TEIF1)) != 0) {
    /* DMA, this could help only if the DMA tries to access an unmapped
       address space or violates alignment rules.*/
    _adc_isr_error_code(adcp, ADC_ERR_DMAFAILURE);
  }
  else {
    /* It is possible that the conversion group has already be reset by the
       ADC error handler, in this case this interrupt is spurious.*/
    if (adcp->grpp != NULL) {
      if ((flags & DMA_TCIF1) != 0) {
        /* Transfer complete processing.*/
        _adc_isr_full_code(adcp);
      }
      else if ((flags & DMA_HTIF1) != 0) {
        /* Half transfer processing.*/
        _adc_isr_half_code(adcp);
      }
    }
  }
}

/**
 * @brief   ADC IRQ service routine.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 * @param[in] isr       content of the ISR register
 */
static void adc_lld_serve_interrupt(ADCDriver *adcp, uint32_t isr) {

  /* It could be a spurious interrupt caused by overflows after DMA disabling,
     just ignore it in this case.*/
  if (adcp->grpp != NULL) {
    adcerror_t emask = 0U;

    /* Note, an overflow may occur after the conversion ended before the driver
       is able to stop the ADC, this is why the state is checked too.*/
    if (isr & ADC_AWD) {
      /* Analog watchdog error.*/
      emask |= ADC_ERR_AWD;
    }
    if (emask != 0U) {
      _adc_isr_error_code(adcp, emask);
    }
  }
}

#if CH32_ADC_USE_ADC1 || CH32_ADC_USE_ADC2 || defined(__DOXYGEN__)
/**
 * @brief   ADC1/ADC2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(CH32_ADC1_2_HANDLER) {
  uint32_t isr;
#if CH32_ADC_USE_ADC1
  isr  = ADC1->STATR;
  ADC1->STATR &= ~isr;
#if defined(CH32_ADC_ADC1_IRQ_HOOK)
  CH32_ADC_ADC1_IRQ_HOOK
#endif
  adc_lld_serve_interrupt(&ADCD1, isr);
#endif

#if CH32_ADC_USE_ADC2
  isr  = ADC2->STATR;
  ADC2->STATR &= ~isr;
#if defined(CH32_ADC_ADC2_IRQ_HOOK)
  CH32_ADC_ADC2_IRQ_HOOK
#endif
  adc_lld_serve_interrupt(&ADCD2, isr);
#endif
}
#endif /* CH32_ADC_USE_ADC1 */
/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level ADC driver initialization.
 *
 * @notapi
 */
void adc_lld_init(void) {

#if CH32_ADC_USE_ADC1 == TRUE
  /* Driver initialization.*/
  adcObjectInit(&ADCD1);
  ADCD1.adcr = ADC1;
  NVIC_EnableIRQ(ADC1_2_IRQn);
  resetHB2(RCC_ADC1RST);
#endif

#if CH32_ADC_USE_ADC2 == TRUE
  /* Driver initialization.*/
  adcObjectInit(&ADCD2);
  ADCD2.adcr = ADC2;
  NVIC_EnableIRQ(ADC1_2_IRQn);
  resetHB2(RCC_ADC2RST);
#endif
}

/**
 * @brief   Configures and activates the ADC peripheral.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_lld_start(ADCDriver *adcp) {

  if (adcp->state == ADC_STOP) {
    /* Enables the peripheral.*/
#if CH32_ADC_USE_ADC1 == TRUE
    if (&ADCD1 == adcp) {
      adcp->dmastp = dmaStreamAllocI(CH32_DMA_STREAM_ID_ANY,
                                     CH32_ADC_ADC1_DMA_IRQ_PRIORITY,
                                     (ch32_dmaisr_t)adc_lld_serve_dma_interrupt,
                                     (void *)adcp);
      osalDbgAssert(adcp->dmastp != NULL, "unable to allocate stream");
      resetHB2(RCC_ADC1RST);
      enableHB2(RCC_ADC1EN);
#if CH32_DMA_SUPPORTS_DMAMUX
      dmaSetRequestSource(adcp->dmastp, DMA_MUX_ADC1);
#endif
    }
#endif

#if CH32_ADC_USE_ADC2 == TRUE
    if (&ADCD2 == adcp) {
      adcp->dmastp = dmaStreamAllocI(CH32_DMA_STREAM_ID_ANY,
                                     CH32_ADC_ADC2_DMA_IRQ_PRIORITY,
                                     (ch32_dmaisr_t)adc_lld_serve_dma_interrupt,
                                     (void *)adcp);
      osalDbgAssert(adcp->dmastp != NULL, "unable to allocate stream");
      resetHB2(RCC_ADC2RST);
      enableHB2(RCC_ADC2EN);
#if CH32_DMA_SUPPORTS_DMAMUX
      dmaSetRequestSource(adcp->dmastp, DMA_MUX_ADC2);
#endif
    }
#endif
    dmaStreamSetPeripheral(adcp->dmastp, &adcp->adcr->RDATAR);
    RCC->CFGR0 &= ~RCC_ADCSRC;
#if CH32_ADC_CLOCKSOURCE_FROM_PLL == TRUE
    RCC->CFGR0 |= (1 << 31);
#endif
    RCC->CFGR0 =
        (RCC->CFGR0 & ~(RCC_PPRE2 | RCC_ADCPRE)) | (CH32_ADC_PRESC << 11) | (CH32_ADC_PRESCALER_HCLKPREDIV_VALUE << 14);

    adcp->adcr->CTLR2 |= CTLR2_ADON_Set;
    adcp->adcr->CTLR2 |= CTLR2_RSTCAL_Set;
    while (adcp->adcr->CTLR2 & CTLR2_RSTCAL_Set)
        ;
    adcp->adcr->SAMPTR1 |= (0x7 << 27);
    adcp->adcr->CTLR2 |= CTLR2_CAL_Set;
    while (adcp->adcr->CTLR2 & CTLR2_CAL_Set)
        ;
    adcp->adcr->CTLR2 &= ~CTLR2_ADON_Set;
  }
  /* Configures the peripheral.*/

}

/**
 * @brief   Deactivates the ADC peripheral.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_lld_stop(ADCDriver *adcp) {

  if (adcp->state == ADC_READY) {
    NVIC_DisableIRQ(ADC1_2_IRQn);
    /* Resets the peripheral.*/
    dmaStreamFreeI(adcp->dmastp);
    adcp->dmastp = NULL;
    adcp->adcr->CTLR2 &= ~ADC_ADON;
    /* Disables the peripheral.*/
#if CH32_ADC_USE_ADC1 == TRUE
    if (&ADCD1 == adcp) {
      disableHB2(RCC_ADC1EN);
    }
#endif

#if CH32_ADC_USE_ADC2 == TRUE
    if (&ADCD2 == adcp) {
      disableHB2(RCC_ADC2EN);
    }
#endif
  }
}

/**
 * @brief   Starts an ADC conversion.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_lld_start_conversion(ADCDriver *adcp) {
  ADCConversionGroup *grpp;
  grpp = (ADCConversionGroup *)(adcp->grpp);
  dmaStreamDisable(adcp->dmastp);
  dmaStreamSetMemory0(adcp->dmastp, adcp->samples);
  dmaStreamSetMode(adcp->dmastp, adcp->config->dmacfg);
  dmaStreamSetTransactionSize(adcp->dmastp, (uint32_t)grpp->num_channels * (uint32_t)adcp->depth);
  dmaStreamEnable(adcp->dmastp);
  NVIC_EnableIRQ(ADC1_2_IRQn);
  adcp->adcr->CTLR1 = (adcp->adcr->CTLR1 & CTLR1_CLEAR_Mask) | adcp->grpp->adcmode | adcp->grpp->adcoutputbuffer |
                      adcp->grpp->adcpga | (adcp->grpp->adcscanmode << 8);
  adcp->adcr->CTLR2 = (adcp->adcr->CTLR2 & CTLR2_CLEAR_Mask) | ((adcp->grpp->adccont ? ADC_CONT : 0)) |
                      adcp->grpp->adcexttrig | adcp->grpp->adcdataalign;

  adcp->adcr->RSQR1 = (grpp->num_channels << 20) | (grpp->adcchannels[15] << 15) | (grpp->adcchannels[14] << 10) |
                      (grpp->adcchannels[13] << 5) | grpp->adcchannels[12];
  adcp->adcr->RSQR2 = (grpp->adcchannels[11] << 25) | (grpp->adcchannels[10] << 20) | (grpp->adcchannels[9] << 15) |
                      (grpp->adcchannels[8] << 10) | (grpp->adcchannels[7] << 5) | grpp->adcchannels[6];
  adcp->adcr->RSQR3 = (grpp->adcchannels[5] << 25) | (grpp->adcchannels[4] << 20) | (grpp->adcchannels[3] << 15) |
                      (grpp->adcchannels[2] << 10) | (grpp->adcchannels[1] << 5) | grpp->adcchannels[0];
  adcp->adcr->CTLR2 |= CTLR2_ADON_Set;
  adcp->adcr->CTLR2 |= CTLR2_DMA_Set;
  adcp->adcr->CTLR2 |= CTLR2_EXTTRIG_SWSTART_Set;
}

/**
 * @brief   Stops an ongoing conversion.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_lld_stop_conversion(ADCDriver *adcp) {
  adcp->adcr->CTLR2 &= ~CTLR2_DMA_Set;
  adcp->adcr->CTLR2 &= ~CTLR2_ADON_Set;
}

#endif /* HAL_USE_ADC == TRUE */

/** @} */
