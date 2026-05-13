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
 * @brief   PLATFORM HAL subsystem low level driver source.
 *
 * @addtogroup HAL
 * @{
 */

#include "hal.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

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

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

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
  // hal_lld_clock_switch_mode(&hal_clkcfg_reset);
  // resetHB1(0xffffffff);
  // resetHB2(0xffffffff);
  hal_lld_clock_switch_mode(&hal_clkcfg_default);
}

#if defined(HAL_LLD_USE_CLOCK_MANAGEMENT) || defined(__DOXYGEN__)
/**
 * @brief   Switches to a different clock configuration
 *
 * @param[in] ccp       pointer to clock a @p halclkcfg_t structure
 * @return              The clock switch result.
 * @retval false        if the clock switch succeeded
 * @retval true         if the clock switch failed
 *
 * @notapi
 */
bool hal_lld_clock_switch_mode(const halclkcfg_t *ccp)
{
    RCC->CTLR |= (uint32_t)0x00000001;
    RCC->CFGR0 &= (uint32_t)0x305C0000;
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x00)
    {
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

    __IO uint32_t StartUpCounter = 0, HSEStatus = 0, FLASH_Temp = 0;
    if (ccp->PLLSRC == RCC_PLLSRC_HSE || ccp->SW == RCC_SW_HSE)
    {
        RCC->CTLR |= ((uint32_t)RCC_HSEON);
        do
        {
            HSEStatus = RCC->CTLR & RCC_HSERDY;
            StartUpCounter++;
        } while ((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

        if ((RCC->CTLR & RCC_HSERDY) != RESET)
        {
            HSEStatus = (uint32_t)0x01;
        }
        else
        {
            HSEStatus = (uint32_t)0x00;
        }
      }

        if (HSEStatus == (uint32_t)0x01 || (ccp->SW != RCC_SW_HSE && ccp->PLLSRC != RCC_PLLSRC_HSE))
        {
            /* configure PLL Clock */
            RCC->PLLCFGR &= (uint32_t)((uint32_t)~(RCC_PLLMUL));
            RCC->PLLCFGR |= (uint32_t)ccp->PLLMUL;
            RCC->PLLCFGR &= (uint32_t)((uint32_t)~(RCC_PLL_SRC_DIV));
            RCC->PLLCFGR |= (uint32_t)ccp->PLLDIV;
            RCC->PLLCFGR &= (uint32_t)((uint32_t)~(RCC_PLLSRC));
            RCC->PLLCFGR |= (uint32_t)ccp->PLLSRC;

            /* Wait till HSE clock is used as PLL clock source */
            while ((RCC->PLLCFGR & (uint32_t)RCC_PLLSRC) != (uint32_t)ccp->PLLSRC)
            {
            }

            /* Enable PLL */
            if (ccp->SW == RCC_SW_PLL)
            {
                RCC->CTLR |= RCC_PLLON;

                /* Wait till PLL is ready */
                while ((RCC->CTLR & RCC_PLLRDY) != (uint32_t)RCC_PLLRDY)
                {
                }

                /* Select PLL Clock as SYSPLL clock source */
                RCC->PLLCFGR &= (uint32_t)((uint32_t)~(RCC_SYSPLL_GATE));
                RCC->PLLCFGR &= (uint32_t)((uint32_t)~(RCC_SYSPLL_SEL));

                /* Wait till PLL is used as system clock source */
                while ((RCC->PLLCFGR & (uint32_t)RCC_SYSPLL_SEL) != (uint32_t)0x00)
                {
                }
            }

            /* V5F core clock = SYSCLK */
            RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_HPRE));
            RCC->CFGR0 |= (uint32_t)ccp->HPRE;

            /* V3F core clock = HCLK = SYSCLK/4 */
            RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_FPRE));
            RCC->CFGR0 |= (uint32_t)ccp->FPRE;

            /* Select FLASH clock frequency*/
            FLASH_Temp = FLASH->ACTLR;
            FLASH_Temp &= ~((uint32_t)0x3);
            FLASH_Temp |= FLASH_ACTLR_LATENCY_HCLK_DIV2;
            FLASH->ACTLR = FLASH_Temp;

            /* Select PLL as system clock source */
            RCC->PLLCFGR |= (uint32_t)RCC_SYSPLL_GATE;
            RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
            RCC->CFGR0 |= (uint32_t)ccp->SW;

            /* Wait till PLL is used as system clock source */
            while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)(ccp->SW << 2))
            {
            }
        }
        else
        {
            /* If HSE fails to start-up, the application will have wrong clock
             * configuration. User can add here some code to deal with this error
             */
        }
    
    return false;
}

static __I uint8_t PLLMULTB[32] = {4,6,7,8,17,9,19,10,21,11,23,12,25,13,14,15,16,17,18,19,20,22,24,26,28,30,32,34,36,38,40,59};
static __I uint8_t HBPrescTB[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
static __I uint8_t SERDESPLLMULTB[16] = {25, 28, 30, 32, 35, 38, 40, 45, 50, 56, 60, 64, 70, 76, 80, 90};
static __I uint8_t FPRETB[4] = {0, 1, 2, 2};
uint32_t SystemClock;         /* System Clock Frequency */
uint32_t SystemCoreClock;
uint32_t HCLKClock;
void SystemAndCoreClockUpdate(void)
{
    uint32_t tmp = 0,tmp1 = 0, tmp2 = 0, tmp3 = 0, pllmull = 0, pllsource = 0, presc = 0, presc1 = 0;

    tmp = RCC->CFGR0 & RCC_SWS;
    tmp2 = RCC->PLLCFGR & RCC_SYSPLL_SEL;

    switch(tmp)
    {
        case 0x00:
            SystemClock = HSI_VALUE;
            break;

        case 0x04:
            SystemClock = HSE_VALUE;
            break;

        case 0x08:
            switch(tmp2)
            { 
                case RCC_SYSPLL_PLL:
                    pllmull = RCC->PLLCFGR & RCC_PLLMUL;
                    pllsource = RCC->PLLCFGR & RCC_PLLSRC;
                    presc = (((RCC->PLLCFGR & RCC_PLL_SRC_DIV) >> 8) + 1);

                    if(pllsource == 0xA0)
                    {
                        tmp1 = 500000000 / presc;
                    }
                    else if(pllsource == 0xE0)
                    {
                        tmp1 = HSE_VALUE*SERDESPLLMULTB[RCC->PLLCFGR2>>16]/2/presc;
                    }

                    else if(pllsource == 0x80)
                    {
                        tmp1 = 480000000 / presc;
                    }
                    else if(pllsource == 0xC0)
                    {
                        tmp1 = 125000000 / presc;
                    }
                    else if(pllsource == 0x20)
                    {
                        tmp1 = HSE_VALUE / presc;
                    }
                    else
                    {
                        tmp1 = HSI_VALUE / presc;
                    }

                    if((pllmull == 4) || (pllmull == 6) || (pllmull == 8) || (pllmull == 10) || (pllmull == 12))
                    {
                        SystemClock = (tmp1 * PLLMULTB[pllmull]) >> 1;
                    }
                    else
                    {
                        SystemClock = tmp1 * PLLMULTB[pllmull];
                    }

                    break;

                case RCC_SYSPLL_USBHS:
                    SystemClock = 480000000;
                    break;

                case RCC_SYSPLL_ETH:
                    SystemClock = 500000000;
                    break;

                case RCC_SYSPLL_SERDES:
                    SystemClock = HSE_VALUE*SERDESPLLMULTB[RCC->PLLCFGR2>>16]/2;
                    break;

                case RCC_SYSPLL_USBSS:
                    SystemClock = 125000000;
                    break;

                default:
                    SystemClock = HSI_VALUE;
                    break;
            }  
            break;

        default:
            SystemClock = HSI_VALUE;
            break;
    }

    tmp = (RCC->CFGR0 & RCC_HPRE) >> 4;
    presc1 = HBPrescTB[tmp];

    tmp3 = SystemClock >> presc1;

    tmp = (RCC->CFGR0 & RCC_FPRE) >> 16;
    presc1 = FPRETB[tmp];
    HCLKClock = tmp3 >> presc1;

    if(NVIC_GetCurrentCoreID() == 0)//V3F
    {
        SystemCoreClock = HCLKClock;
    }
    else 
    {
        SystemCoreClock = tmp3;
    }
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
