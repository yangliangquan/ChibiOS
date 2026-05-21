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
 * @file    hal_adc_lld.h
 * @brief   CH32 ADC subsystem low level driver header.
 *
 * @addtogroup ADC
 * @{
 */

#ifndef HAL_ADC_LLD_H
#define HAL_ADC_LLD_H

#if (HAL_USE_ADC == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    Possible ADC errors mask bits.
 * @{
 */
#define ADC_ERR_DMAFAILURE      1U  /**< DMA operations failure.            */
#define ADC_ERR_OVERFLOW        2U  /**< ADC overflow condition.            */
#define ADC_ERR_AWD             4U  /**< Watchdog triggered.                */
/** @} */


/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    CH32 configuration options
 * @{
 */
/**
 * @brief   ADC1 driver enable switch.
 * @details If set to @p TRUE the support for ADC1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_ADC_USE_ADC1) || defined(__DOXYGEN__)
#define CH32_ADC_USE_ADC1                  FALSE
#endif

/**
 * @brief   ADC2 driver enable switch.
 * @details If set to @p TRUE the support for ADC2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_ADC_USE_ADC2) || defined(__DOXYGEN__)
#define CH32_ADC_USE_ADC2                  FALSE
#endif

#if defined(CH32_ADC_USE_ADC2) || defined(CH32_ADC_USE_ADC1) || defined(__DOXYGEN__)
#define CH32_ADC1_2_HANDLER                  ADC1_2_IRQHandler
#endif

#if !defined(CH32_ADC_CLOCKSOURCE_FROM_PLL) || defined(__DOXYGEN__)
#define CH32_ADC_CLOCKSOURCE_FROM_PLL      FALSE
#endif

#if (CH32_ADC_SUPPORTS_PRESCALER == TRUE) || defined(__DOXYGEN__)
/*
 * @brief   ADC prescaler setting.
 * @note    This setting has effect only in asynchronous clock mode (the
 *          default, @p CH32_ADC_CKMODE_ADCCLK).
 */
#if !defined(CH32_ADC_PRESCALER_VALUE) || defined(__DOXYGEN__)
#define CH32_ADC_PRESCALER_VALUE                      2
#endif

#if !defined(CH32_HCLKPREDIV_ADC_PRESCALER_VALUE) || defined(__DOXYGEN__)
#define CH32_HCLKPREDIV_ADC_PRESCALER_VALUE           2
#endif
#endif

/**
 * @brief   ADC1 driver enable switch.
 * @details If set to @p TRUE the support for ADC1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_ADC_USE_ADC1) || defined(__DOXYGEN__)
#define CH32_ADC_USE_ADC1                  FALSE
#endif

/**
 * @brief   ADC2 driver enable switch.
 * @details If set to @p TRUE the support for ADC2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(CH32_ADC_USE_ADC2) || defined(__DOXYGEN__)
#define CH32_ADC_USE_ADC2                  FALSE
#endif

/**
 * @brief   ADC1 DMA priority (0..3|lowest..highest).
 */
#if !defined(CH32_ADC_ADC1_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_ADC_ADC1_DMA_PRIORITY         2
#endif

/**
 * @brief   ADC2 DMA priority (0..3|lowest..highest).
 */
#if !defined(CH32_ADC_ADC2_DMA_PRIORITY) || defined(__DOXYGEN__)
#define CH32_ADC_ADC2_DMA_PRIORITY         2
#endif

/**
 * @brief   ADC1 DMA interrupt priority level setting.
 */
#if !defined(CH32_ADC_ADC1_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_ADC_ADC1_DMA_IRQ_PRIORITY     5
#endif

/**
 * @brief   ADC2 DMA interrupt priority level setting.
 */
#if !defined(CH32_ADC_ADC2_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define CH32_ADC_ADC2_DMA_IRQ_PRIORITY     5
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/
/* Registry checks.*/
#if !defined(CH32_HAS_ADC1)
#error "CH32_HAS_ADC1 not defined in registry"
#endif

#if !defined(CH32_HAS_ADC2)
#error "CH32_HAS_ADC1 not defined in registry"
#endif

#if (CH32_ADC_USE_ADC1 == TRUE) && (CH32_ADC_USE_ADC2 == TRUE)
#define CH32_DMA_REQUIRED                  TRUE
#endif

#if !defined(CH32_ADC_SUPPORTS_PRESCALER)
#error "CH32_ADC_SUPPORTS_PRESCALER not defined in registry"
#endif

/* ADC clock source checks.*/
#if CH32_ADC_SUPPORTS_PRESCALER == TRUE
#if CH32_ADC_PRESCALER_VALUE == 1
#define CH32_ADC_PRESC                     0U
#elif CH32_ADC_PRESCALER_VALUE == 2
#define CH32_ADC_PRESC                     1U
#elif CH32_ADC_PRESCALER_VALUE == 4
#define CH32_ADC_PRESC                     2U
#elif CH32_ADC_PRESCALER_VALUE == 8
#define CH32_ADC_PRESC                     4U
#else
#error "Invalid value assigned to CH32_ADC_PRESCALER_VALUE"
#endif

#if CH32_HCLKPREDIV_ADC_PRESCALER_VALUE == 2
#define CH32_ADC_PRESCALER_HCLKPREDIV_VALUE         0U
#elif CH32_HCLKPREDIV_ADC_PRESCALER_VALUE == 4
#define CH32_ADC_PRESCALER_HCLKPREDIV_VALUE         1U
#elif CH32_HCLKPREDIV_ADC_PRESCALER_VALUE == 6
#define CH32_ADC_PRESCALER_HCLKPREDIV_VALUE         2U
#elif CH32_HCLKPREDIV_ADC_PRESCALER_VALUE == 8
#define CH32_ADC_PRESCALER_HCLKPREDIV_VALUE         3U
#else
#error "Invalid value assigned to CH32_HCLKPREDIV_ADC_PRESCALER_VALUE"
#endif
#endif
/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   ADC sample data type.
 */
typedef uint16_t adcsample_t;

/**
 * @brief   Channels number in a conversion group.
 */
typedef uint16_t adc_channels_num_t;

/**
 * @brief   Type of an ADC error mask.
 */
typedef uint32_t adcerror_t;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Low level fields of the ADC driver structure.
 */
#define adc_lld_driver_fields                                               \
  /* Dummy field, it is not needed.*/                                       \
  ADC_TypeDef               *adcr;                                          \
  const ch32_dma_stream_t   *dmastp;
/**
 * @brief   Low level fields of the ADC configuration structure.
 */
#define adc_lld_config_fields                                               \
  /* Dummy configuration, it is not needed.*/                               \
  uint32_t                  dmacfg;                                         



/**
 * @brief   Low level fields of the ADC configuration structure.
 */
#define adc_lld_configuration_group_fields                                  \
  /* Dummy configuration, it is not needed.*/                               \
  uint32_t                  adcchannels[16];                                \
  uint32_t                  adcsampletime[16];                              \
  uint32_t                  adcmode;                                        \
  uint32_t                  adcoutputbuffer;                                \
  uint32_t                  adcpga;                                         \
  uint32_t                  adcdataalign;                                   \
  uint32_t                  adcscanmode;                                    \
  uint32_t                  adccont;                                        \
  uint32_t                  adcexttrig;                                     


/* ADC_mode */
#define ADC_Mode_Independent                        ((uint32_t)0x00000000)
#define ADC_Mode_RegInjecSimult                     ((uint32_t)0x00010000)
#define ADC_Mode_RegSimult_AlterTrig                ((uint32_t)0x00020000)
#define ADC_Mode_InjecSimult_FastInterl             ((uint32_t)0x00030000)
#define ADC_Mode_InjecSimult_SlowInterl             ((uint32_t)0x00040000)
#define ADC_Mode_InjecSimult                        ((uint32_t)0x00050000)
#define ADC_Mode_RegSimult                          ((uint32_t)0x00060000)
#define ADC_Mode_FastInterl                         ((uint32_t)0x00070000)
#define ADC_Mode_SlowInterl                         ((uint32_t)0x00080000)
#define ADC_Mode_AlterTrig                          ((uint32_t)0x00090000)

/* ADC_external_trigger_sources_for_regular_channels_conversion */
#define ADC_ExternalTrigConv_T1_CC1                 ((uint32_t)0x00000000) 
#define ADC_ExternalTrigConv_T1_CC2                 ((uint32_t)0x00020000) 
#define ADC_ExternalTrigConv_T1_CC3                 ((uint32_t)0x00040000) 
#define ADC_ExternalTrigConv_T2_CC2                 ((uint32_t)0x00060000) 
#define ADC_ExternalTrigConv_T3_TRGO                ((uint32_t)0x00080000) 
#define ADC_ExternalTrigConv_T4_CC4                 ((uint32_t)0x000A0000)
#define ADC_ExternalTrigConv_Ext_IT11_TIM8_TRGO     ((uint32_t)0x000C0000) 
#define ADC_ExternalTrigConv_None                   ((uint32_t)0x000E0000) 

/* ADC_data_align */
#define ADC_DataAlign_Right                         ((uint32_t)0x00000000)
#define ADC_DataAlign_Left                          ((uint32_t)0x00000800)

/* ADC_channels */
#define ADC_Channel_0                               ((uint8_t)0x00)
#define ADC_Channel_1                               ((uint8_t)0x01)
#define ADC_Channel_2                               ((uint8_t)0x02)
#define ADC_Channel_3                               ((uint8_t)0x03)
#define ADC_Channel_4                               ((uint8_t)0x04)
#define ADC_Channel_5                               ((uint8_t)0x05)
#define ADC_Channel_6                               ((uint8_t)0x06)
#define ADC_Channel_7                               ((uint8_t)0x07)
#define ADC_Channel_8                               ((uint8_t)0x08)
#define ADC_Channel_9                               ((uint8_t)0x09)
#define ADC_Channel_10                              ((uint8_t)0x0A)
#define ADC_Channel_11                              ((uint8_t)0x0B)
#define ADC_Channel_12                              ((uint8_t)0x0C)
#define ADC_Channel_13                              ((uint8_t)0x0D)
#define ADC_Channel_14                              ((uint8_t)0x0E)
#define ADC_Channel_15                              ((uint8_t)0x0F)
#define ADC_Channel_16                              ((uint8_t)0x10)
#define ADC_Channel_17                              ((uint8_t)0x11)

#define ADC_Channel_TempSensor                      ((uint8_t)ADC_Channel_16)
#define ADC_Channel_Vrefint                         ((uint8_t)ADC_Channel_17)

/*ADC_output_buffer*/
#define ADC_OutputBuffer_Enable                     ((uint32_t)0x04000000)
#define ADC_OutputBuffer_Disable                    ((uint32_t)0x00000000)

/*ADC_pga*/
#define ADC_Pga_1                                   ((uint32_t)0x00000000)
#define ADC_Pga_4                                   ((uint32_t)0x08000000)
#define ADC_Pga_16                                  ((uint32_t)0x10000000)
#define ADC_Pga_64                                  ((uint32_t)0x18000000)

/* ADC_sampling_time */
#define ADC_SampleTime_CyclesMode0                  ((uint8_t)0x00)
#define ADC_SampleTime_CyclesMode1                  ((uint8_t)0x01)
#define ADC_SampleTime_CyclesMode2                  ((uint8_t)0x02)
#define ADC_SampleTime_CyclesMode3                  ((uint8_t)0x03)
#define ADC_SampleTime_CyclesMode4                  ((uint8_t)0x04)
#define ADC_SampleTime_CyclesMode5                  ((uint8_t)0x05)
#define ADC_SampleTime_CyclesMode6                  ((uint8_t)0x06)
#define ADC_SampleTime_CyclesMode7                  ((uint8_t)0x07)

/* ADC_external_trigger_sources_for_injected_channels_conversion */
#define ADC_ExternalTrigInjecConv_T1_TRGO           ((uint32_t)0x00000000) 
#define ADC_ExternalTrigInjecConv_T1_CC4            ((uint32_t)0x00001000) 
#define ADC_ExternalTrigInjecConv_T2_TRGO           ((uint32_t)0x00002000) 
#define ADC_ExternalTrigInjecConv_T2_CC1            ((uint32_t)0x00003000) 
#define ADC_ExternalTrigInjecConv_T3_CC4            ((uint32_t)0x00004000) 
#define ADC_ExternalTrigInjecConv_T4_TRGO           ((uint32_t)0x00005000) 
#define ADC_ExternalTrigInjecConv_Ext_IT15_TIM8_CC4 ((uint32_t)0x00006000) 
#define ADC_ExternalTrigInjecConv_None              ((uint32_t)0x00007000) 

/* ADC_injected_channel_selection */
#define ADC_InjectedChannel_1                       ((uint8_t)0x14)
#define ADC_InjectedChannel_2                       ((uint8_t)0x18)
#define ADC_InjectedChannel_3                       ((uint8_t)0x1C)
#define ADC_InjectedChannel_4                       ((uint8_t)0x20)

/* ADC_analog_watchdog_selection */
#define ADC_AnalogWatchdog_SingleRegEnable          ((uint32_t)0x00800200)
#define ADC_AnalogWatchdog_SingleInjecEnable        ((uint32_t)0x00400200)
#define ADC_AnalogWatchdog_SingleRegOrInjecEnable   ((uint32_t)0x00C00200)
#define ADC_AnalogWatchdog_AllRegEnable             ((uint32_t)0x00800000)
#define ADC_AnalogWatchdog_AllInjecEnable           ((uint32_t)0x00400000)
#define ADC_AnalogWatchdog_AllRegAllInjecEnable     ((uint32_t)0x00C00000)
#define ADC_AnalogWatchdog_None                     ((uint32_t)0x00000000)

/* ADC_interrupts_definition */
#define ADC_IT_EOC                                  ((uint16_t)0x0220)
#define ADC_IT_AWD                                  ((uint16_t)0x0140)
#define ADC_IT_JEOC                                 ((uint16_t)0x0480)

/* ADC_flags_definition */
#define ADC_FLAG_AWD                                ((uint16_t)0x0001)
#define ADC_FLAG_EOC                                ((uint16_t)0x0002)
#define ADC_FLAG_JEOC                               ((uint16_t)0x0004)
#define ADC_FLAG_JSTRT                              ((uint16_t)0x0008)
#define ADC_FLAG_STRT                               ((uint16_t)0x0010)
#define ADC_FLAG_RST                                ((uint16_t)0x8000)

/* ADC_SMP_CFG_MODE_definition */
#define ADC_SMP_CFG_MODE0                           ((uint8_t)0x00)
#define ADC_SMP_CFG_MODE1                           ((uint8_t)0x01)

/* ADC_CalibrationVoltage_Mode_definition */
#define ADC_CalibrationVoltage_Mode0                ((uint32_t)0x00000000)
#define ADC_CalibrationVoltage_Mode1                ((uint32_t)0x00000010)
#define ADC_CalibrationVoltage_Mode2                ((uint32_t)0x00000020)
#define ADC_CalibrationVoltage_Mode3                ((uint32_t)0x00000030)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (CH32_ADC_USE_ADC1 == TRUE) && !defined(__DOXYGEN__)
extern ADCDriver ADCD1;
#endif

#if (CH32_ADC_USE_ADC1 == TRUE) && !defined(__DOXYGEN__)
extern ADCDriver ADCD2;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void adc_lld_init(void);
  void adc_lld_start(ADCDriver *adcp);
  void adc_lld_stop(ADCDriver *adcp);
  void adc_lld_start_conversion(ADCDriver *adcp);
  void adc_lld_stop_conversion(ADCDriver *adcp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_ADC == TRUE */

#endif /* HAL_ADC_LLD_H */

/** @} */
