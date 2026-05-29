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
 * @file    hal_lld.c
 * @brief   CH32H417 HAL subsystem low level driver source.
 *
 * @addtogroup HAL
 * @{
 */

#include "hal.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/**
 * @name    RCC register bit masks
 * @{
 */
/* CTLR register bit mask */
#define CTLR_HSEBYP_Reset          ((uint32_t)0xFFFBFFFF)
#define CTLR_HSEBYP_Set            ((uint32_t)0x00040000)
#define CTLR_HSEON_Reset           ((uint32_t)0xFFFEFFFF)
#define CTLR_HSEON_Set             ((uint32_t)0x00010000)
#define CTLR_HSITRIM_Mask          ((uint32_t)0xFFFFFF07)

/* CFGR0 register bit mask */
#define CFGR0_SWS_Mask             ((uint32_t)0x0000000C)
#define CFGR0_SW_Mask              ((uint32_t)0xFFFFFFFC)
#define CFGR0_HPRE_Set_Mask        ((uint32_t)0x000000F0)

/* RSTSCKR register bit mask */
#define RSTSCKR_RMVF_Set           ((uint32_t)0x01000000)

/* RCC Flag Mask */
#define FLAG_Mask                  ((uint8_t)0x1F)
/** @} */

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

#if defined(HAL_LLD_USE_CLOCK_MANAGEMENT) && !defined(__DOXYGEN__)
/**
 * @brief   Post-reset configuration, must be implemented.
 */
const halclkcfg_t hal_clkcfg_reset = {
  .SW = RCC_SW_HSI,
  .HPRE = RCC_HPRE_DIV1,
  .FPRE = RCC_FPRE_DIV1
};

/**
 * @brief   Configuration from mcuconf.h, must be implemented.
 */
const halclkcfg_t hal_clkcfg_default = {
  .SW = RCC_SW_PLL,
  .HPRE = RCC_HPRE_DIV4,
  .FPRE = RCC_FPRE_DIV1,
  .SYSPLL_SEL = RCC_SYSPLL_PLL,
  .PLLMUL = RCC_PLLMUL16,
  .PLLDIV = RCC_PLL_SRC_DIV1,
  .PLLSRC = RCC_PLLSRC_HSE
};
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   PLL multiplication lookup table.
 * @details Maps PLLMUL register values to multiplication factors.
 */
static const uint8_t PLLMULTable[32] = {
  4, 6, 7, 8, 17, 9, 19, 10, 21, 11, 23, 12, 25, 13, 14, 15,
  16, 17, 18, 19, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 59
};

/**
 * @brief   AHB prescaler lookup table.
 * @details Maps HPRE register values to prescaler shift values.
 */
static const uint8_t HBPrescTable[16] = {
  0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9
};

/**
 * @brief   SERDES PLL multiplication lookup table.
 * @details Maps PLLCFGR2 register values to multiplication factors.
 */
static const uint8_t SERDESPLLMULTable[16] = {
  25, 28, 30, 32, 35, 38, 40, 45, 50, 56, 60, 64, 70, 76, 80, 90
};

/**
 * @brief   Fast peripheral clock prescaler lookup table.
 */
static const uint8_t FPRETable[4] = {0, 1, 2, 2};

/**
 * @brief   APB2 prescaler lookup table.
 */
static const uint8_t PPRE2Table[8] = {0, 0, 0, 0, 1, 2, 3, 4};

/**
 * @brief   ADC prescaler lookup table.
 */
static const uint8_t ADCPRETable[4] = {2, 4, 6, 8};

/**
 * @brief   System clock frequency.
 */
uint32_t SystemClock;

/**
 * @brief   Core clock frequency.
 */
uint32_t SystemCoreClock;

/**
 * @brief   HCLK clock frequency.
 */
uint32_t HCLKClock;

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Calculates system clock frequency based on current RCC configuration.
 *
 * @param[out] RCC_Clocks  pointer to RCC_ClocksTypeDef structure to fill
 */
static void calc_system_clock(RCC_ClocksTypeDef *RCC_Clocks) {
  uint32_t tmp, tmp2, tmp1;
  uint32_t pllmull, pllsource, presc;

  tmp = RCC->CFGR0 & CFGR0_SWS_Mask;
  tmp2 = RCC->PLLCFGR & RCC_SYSPLL_SEL;

  switch (tmp) {
  case 0x00:
    /* HSI used as system clock */
    RCC_Clocks->SYSCLK_Frequency = HSI_VALUE;
    break;

  case 0x04:
    /* HSE used as system clock */
    RCC_Clocks->SYSCLK_Frequency = HSE_VALUE;
    break;

  case 0x08:
    /* PLL used as system clock */
    switch (tmp2) {
    case RCC_SYSPLL_PLL:
      pllmull = RCC->PLLCFGR & RCC_PLLMUL;
      pllsource = RCC->PLLCFGR & RCC_PLLSRC;
      presc = (((RCC->PLLCFGR & RCC_PLL_SRC_DIV) >> 8) + 1);

      if (pllsource == 0xA0) {
        /* Source: 500MHz internal clock */
        tmp1 = 500000000 / presc;
      }
      else if (pllsource == 0xE0) {
        /* Source: HSE through SERDES PLL */
        tmp1 = HSE_VALUE * SERDESPLLMULTable[RCC->PLLCFGR2 >> 16] / 2 / presc;
      }
      else if (pllsource == 0x80) {
        /* Source: 480MHz USB HS PLL */
        tmp1 = 480000000 / presc;
      }
      else if (pllsource == 0xC0) {
        /* Source: 125MHz USB SS PLL */
        tmp1 = 125000000 / presc;
      }
      else if (pllsource == 0x20) {
        /* Source: HSE direct */
        tmp1 = HSE_VALUE / presc;
      }
      else {
        /* Source: HSI */
        tmp1 = HSI_VALUE / presc;
      }

      /* Handle half multiplication for even PLLMUL values */
      if ((pllmull == 4) || (pllmull == 6) || (pllmull == 8) ||
          (pllmull == 10) || (pllmull == 12)) {
        RCC_Clocks->SYSCLK_Frequency = (tmp1 * PLLMULTable[pllmull]) >> 1;
      }
      else {
        RCC_Clocks->SYSCLK_Frequency = tmp1 * PLLMULTable[pllmull];
      }
      break;

    case RCC_SYSPLL_USBHS:
      RCC_Clocks->SYSCLK_Frequency = 480000000;
      break;

    case RCC_SYSPLL_ETH:
      RCC_Clocks->SYSCLK_Frequency = 500000000;
      break;

    case RCC_SYSPLL_SERDES:
      tmp1 = RCC->PLLCFGR2 >> 16;
      RCC_Clocks->SYSCLK_Frequency = HSE_VALUE * SERDESPLLMULTable[tmp1] / 2;
      break;

    case RCC_SYSPLL_USBSS:
      RCC_Clocks->SYSCLK_Frequency = 125000000;
      break;

    default:
      RCC_Clocks->SYSCLK_Frequency = HSI_VALUE;
      break;
    }
    break;

  default:
    RCC_Clocks->SYSCLK_Frequency = HSI_VALUE;
    break;
  }
}

/**
 * @brief   Calculates HCLK and Core clock frequencies.
 *
 * @param[in,out] RCC_Clocks  pointer to RCC_ClocksTypeDef structure
 */
static void calc_hclk_clock(RCC_ClocksTypeDef *RCC_Clocks) {
  uint32_t tmp, presc1, tmp3;

  /* Calculate HCLK frequency from SYSCLK using HPRE prescaler */
  tmp = (RCC->CFGR0 & CFGR0_HPRE_Set_Mask) >> 4;
  presc1 = HBPrescTable[tmp];
  tmp3 = RCC_Clocks->SYSCLK_Frequency >> presc1;

  /* Apply FPRE prescaler for HCLK */
  tmp = (RCC->CFGR0 & RCC_FPRE) >> 16;
  presc1 = FPRETable[tmp];
  RCC_Clocks->HCLK_Frequency = tmp3 >> presc1;

  /* Core clock depends on which core is current */
  if (NVIC_GetCurrentCoreID() == 0) {
    /* V3F core: Core clock = HCLK */
    RCC_Clocks->Core_Frequency = RCC_Clocks->HCLK_Frequency;
  }
  else {
    /* V5F core: Core clock = SYSCLK/HPRE (before FPRE) */
    RCC_Clocks->Core_Frequency = tmp3;
  }
}

/**
 * @brief   Calculates ADC clock frequency.
 *
 * @param[in,out] RCC_Clocks  pointer to RCC_ClocksTypeDef structure
 */
static void calc_adc_clock(RCC_ClocksTypeDef *RCC_Clocks) {
  uint32_t tmp, presc;

  if ((RCC->CFGR0 & RCC_ADCSRC) == RCC_ADCSRC) {
    /* ADC clock from USB HS PLL (480MHz) */
    RCC_Clocks->ADCCLK_Frequency = 480000000 / (((RCC->CFGR0 & 0xF800) >> 11) + 5);
  }
  else {
    /* ADC clock from APB2 */
    tmp = (RCC->CFGR0 & RCC_PPRE2) >> 11;
    presc = PPRE2Table[tmp];
    RCC_Clocks->ADCCLK_Frequency = RCC_Clocks->HCLK_Frequency >> presc;

    /* Apply ADC prescaler */
    tmp = (RCC->CFGR0 & RCC_ADCPRE) >> 14;
    presc = ADCPRETable[tmp];
    RCC_Clocks->ADCCLK_Frequency = RCC_Clocks->ADCCLK_Frequency / presc;
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level HAL driver initialization.
 *
 * @notapi
 */
void hal_lld_init(void) {
  /* Switch to default clock configuration */
  hal_lld_clock_switch_mode(&hal_clkcfg_default);
}

/**
 * @brief   Returns the frequencies of various on-chip clocks.
 * @note    This function is based on the library source from WCH.
 *
 * @param[out] RCC_Clocks  pointer to a @p RCC_ClocksTypeDef structure which
 *                         will receive the clocks frequencies
 *
 * @notapi
 */
void RCC_GetClocksFreq(RCC_ClocksTypeDef *RCC_Clocks) {

  /* Calculate system clock frequency */
  calc_system_clock(RCC_Clocks);

  /* Calculate HCLK and Core clock frequencies */
  calc_hclk_clock(RCC_Clocks);

  /* Calculate ADC clock frequency */
  calc_adc_clock(RCC_Clocks);
}

#if defined(HAL_LLD_USE_CLOCK_MANAGEMENT) || defined(__DOXYGEN__)
/**
 * @brief   Switches to a different clock configuration.
 *
 * @param[in] ccp       pointer to a @p halclkcfg_t structure
 * @return              The clock switch result.
 * @retval false        if the clock switch succeeded
 * @retval true         if the clock switch failed
 *
 * @notapi
 */
bool hal_lld_clock_switch_mode(const halclkcfg_t *ccp) {
  __IO uint32_t StartUpCounter = 0;
  __IO uint32_t HSEStatus = 0;
  __IO uint32_t FLASH_Temp = 0;

  /* Reset RCC registers to default state */
  RCC->CTLR |= (uint32_t)0x00000001;
  RCC->CFGR0 &= (uint32_t)0x305C0000;

  /* Wait until system clock switches to HSI */
  while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x00) {
  }

  RCC->CFGR0 &= (uint32_t)0xFFBFFFFF;
  RCC->PLLCFGR &= (uint32_t)0x7FFFFFFF;
  RCC->CTLR &= (uint32_t)0x6AA6FFFF;
  RCC->CTLR &= (uint32_t)0xFFFBFFFF;
  RCC->PLLCFGR &= (uint32_t)0x0FFFC000;
  RCC->PLLCFGR |= (uint32_t)0x00000004;
  RCC->INTR = 0x00FF0000;
  RCC->CFGR2 &= 0x0C600000;
  RCC->PLLCFGR2 &= 0xFFF0E080;
  RCC->PLLCFGR2 |= 0x00080020;

  /* Enable HSE if needed */
  if (ccp->PLLSRC == RCC_PLLSRC_HSE || ccp->SW == RCC_SW_HSE) {
    RCC->CTLR |= ((uint32_t)RCC_HSEON);

    /* Wait until HSE is ready or timeout */
    do {
      HSEStatus = RCC->CTLR & RCC_HSERDY;
      StartUpCounter++;
    } while ((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

    if ((RCC->CTLR & RCC_HSERDY) != RESET) {
      HSEStatus = (uint32_t)0x01;
    }
    else {
      HSEStatus = (uint32_t)0x00;
    }
  }

  /* Proceed if HSE is ready or HSE is not required */
  if (HSEStatus == (uint32_t)0x01 ||
      (ccp->SW != RCC_SW_HSE && ccp->PLLSRC != RCC_PLLSRC_HSE)) {

    /* Configure PLL */
    RCC->PLLCFGR &= (uint32_t)~(RCC_PLLMUL);
    RCC->PLLCFGR |= (uint32_t)ccp->PLLMUL;

    RCC->PLLCFGR &= (uint32_t)~(RCC_PLL_SRC_DIV);
    RCC->PLLCFGR |= (uint32_t)ccp->PLLDIV;

    RCC->PLLCFGR &= (uint32_t)~(RCC_PLLSRC);
    RCC->PLLCFGR |= (uint32_t)ccp->PLLSRC;

    /* Wait till PLL clock source is set */
    while ((RCC->PLLCFGR & (uint32_t)RCC_PLLSRC) != (uint32_t)ccp->PLLSRC) {
    }

    /* Enable PLL if selected as system clock */
    if (ccp->SW == RCC_SW_PLL) {
      RCC->CTLR |= RCC_PLLON;

      /* Wait till PLL is ready */
      while ((RCC->CTLR & RCC_PLLRDY) != (uint32_t)RCC_PLLRDY) {
      }

      /* Select PLL Clock as SYSPLL clock source */
      RCC->PLLCFGR &= (uint32_t)~(RCC_SYSPLL_GATE);
      RCC->PLLCFGR &= (uint32_t)~(RCC_SYSPLL_SEL);

      /* Wait till PLL is used as system clock source */
      while ((RCC->PLLCFGR & (uint32_t)RCC_SYSPLL_SEL) != (uint32_t)0x00) {
      }
    }

    /* Configure V5F core clock = SYSCLK */
    RCC->CFGR0 &= (uint32_t)~(RCC_HPRE);
    RCC->CFGR0 |= (uint32_t)ccp->HPRE;

    /* Configure V3F core clock = HCLK = SYSCLK/4 */
    RCC->CFGR0 &= (uint32_t)~(RCC_FPRE);
    RCC->CFGR0 |= (uint32_t)ccp->FPRE;

    /* Configure FLASH latency */
    FLASH_Temp = FLASH->ACTLR;
    FLASH_Temp &= ~((uint32_t)0x3);
    FLASH_Temp |= FLASH_ACTLR_LATENCY_HCLK_DIV2;
    FLASH->ACTLR = FLASH_Temp;

    /* Select PLL as system clock source */
    RCC->PLLCFGR |= (uint32_t)RCC_SYSPLL_GATE;
    RCC->CFGR0 &= (uint32_t)~(RCC_SW);
    RCC->CFGR0 |= (uint32_t)ccp->SW;

    /* Wait till selected clock is used as system clock source */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)(ccp->SW << 2)) {
    }
  }
  else {
    /* If HSE fails to start-up, the application will have wrong clock
     * configuration. User can add here some code to deal with this error.
     */
  }

  return false;
}

/**
 * @brief   Updates SystemCoreClock and HCLKClock global variables.
 * @note    This function is based on the library source from WCH.
 *
 * @notapi
 */
void SystemAndCoreClockUpdate(void) {
  RCC_ClocksTypeDef clocks;

  /* Get all clock frequencies */
  RCC_GetClocksFreq(&clocks);

  /* Update global variables */
  SystemClock = clocks.SYSCLK_Frequency;
  HCLKClock = clocks.HCLK_Frequency;
  SystemCoreClock = clocks.Core_Frequency;
}

/**
 * @brief   Returns the frequency of a clock point in Hz.
 *
 * @param[in] clkpt     clock point to be returned
 * @return              The clock point frequency in Hz or zero if the
 *                      frequency is unknown.
 *
 * @notapi
 */
halfreq_t hal_lld_get_clock_point(halclkpt_t clkpt) {

  (void)clkpt;

  SystemAndCoreClockUpdate();

  return (halfreq_t)SystemCoreClock;
}
#endif /* defined(HAL_LLD_USE_CLOCK_MANAGEMENT) */

/** @} */
