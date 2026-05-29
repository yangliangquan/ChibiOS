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
 * @file    hal_pal_lld.c
 * @brief   CH32 PAL subsystem low level driver source.
 *
 * @addtogroup PAL
 * @{
 */

#include "hal.h"

#if (HAL_USE_PAL == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/
#if (PAL_USE_WAIT == TRUE) || (PAL_USE_CALLBACKS == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   Event records for the 16 GPIO EXTI channels.
 */
palevent_t _pal_events[16];
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
 * @brief   STM32 I/O ports configuration.
 *
 * @notapi
 */
void _pal_lld_init(void) {
#if PAL_USE_CALLBACKS || PAL_USE_WAIT || defined(__DOXYGEN__)
  unsigned i;

  for (i = 0; i < 16; i++) {
    _pal_init_event(i);
  }
#endif
}

/**
 * @brief   Pads mode setup.
 * @details This function programs a pads group belonging to the same port
 *          with the specified mode.
 *
 * @param[in] port      the port identifier
 * @param[in] mask      the group mask
 * @param[in] mode      the mode
 *
 * @notapi
 */
void _pal_lld_setgroupmode(ioportid_t port, ioportmask_t mask, iomode_t mode)
{
    uint32_t modeval = 0x44444444, odrval = 0;
    if (IOPORT1 == port)
    {
        enableHB2(RCC_IOPAEN);
    }
    else if (IOPORT2 == port)
    {
        enableHB2(RCC_IOPBEN);
    }
    else if (IOPORT3 == port)
    {
        enableHB2(RCC_IOPCEN);
    }
    else if (IOPORT4 == port)
    {
        enableHB2(RCC_IOPDEN);
    }
    else if (IOPORT5 == port)
    {
        enableHB2(RCC_IOPEEN);
    }
    else if (IOPORT6 == port)
    {
        enableHB2(RCC_IOPFEN);
    }

    if ((mode & 0xff) == PAL_MODE_INPUT)
    {
        modeval = 0x44444444;
        enableHB2(RCC_AFIOEN);
        uint32_t afioreg = ((((uint32_t)port - (uint32_t)IOPORT1)) / 0x400 * 8) + (uint32_t)&(AFIO->GPIOA_AFLR);
        for(size_t i = 0; i < PAL_IOPORTS_WIDTH; i++){
            if((1U << i) & mask){
                if(i < 8)
                {
                    *(uint32_t *)afioreg = (*(uint32_t *)afioreg & ~(0xF << ((i & 0xF) * 4))) | (((mode >> 8) & 0xF) << ((i & 0xF) * 4));
                }else if(i < 16)
                {
                    *(uint32_t *)(afioreg + 4) = (*(uint32_t *)(afioreg + 4) & ~(0xF << (((i - 8) & 0xF) * 4))) | (((mode >> 8) & 0xF) << (((i - 8) & 0xF) * 4));
                }
            }
        }
    }
    else if (mode == PAL_MODE_OUTPUT_PUSHPULL)
    {
        modeval = 0x33333333;
    }
    else if (mode == PAL_MODE_OUTPUT_OPENDRAIN)
    {
        modeval = 0x77777777;
    }
    else if ((mode & 0xff) == PAL_MODE_INPUT_PULLUP)
    {
        modeval = 0x88888888;
        odrval = 0xFFFFFFFF;
        enableHB2(RCC_AFIOEN);
        uint32_t afioreg = ((((uint32_t)port - (uint32_t)IOPORT1)) / 0x400* 8) + (uint32_t)&(AFIO->GPIOA_AFLR);
        for(size_t i = 0; i < PAL_IOPORTS_WIDTH; i++){
            if((1U << i) & mask){
                if(i < 8)
                {
                    *(uint32_t *)afioreg = (*(uint32_t *)afioreg & ~(0xF << ((i & 0xF) * 4))) | (((mode >> 8) & 0xF) << ((i & 0xF) * 4));
                }else if(i < 16)
                {
                    *(uint32_t *)(afioreg + 4) = (*(uint32_t *)(afioreg + 4) & ~(0xF << (((i - 8) & 0xF) * 4))) | (((mode >> 8) & 0xF) << (((i - 8) & 0xF) * 4));
                }
            }
        }
    }
    else if ((mode & 0xff) == PAL_MODE_INPUT_PULLDOWN)
    {
        modeval = 0x88888888;
        enableHB2(RCC_AFIOEN);
        uint32_t afioreg = ((((uint32_t)port - (uint32_t)IOPORT1)) / 0x400* 8) + (uint32_t)&(AFIO->GPIOA_AFLR);
        for(size_t i = 0; i < PAL_IOPORTS_WIDTH; i++){
            if((1U << i) & mask){
                if(i < 8)
                {
                    *(uint32_t *)afioreg = (*(uint32_t *)afioreg & ~(0xF << ((i & 0xF) * 4))) | (((mode >> 8) & 0xF) << ((i & 0xF) * 4));
                }else if(i < 16)
                {
                    *(uint32_t *)(afioreg + 4) = (*(uint32_t *)(afioreg + 4) & ~(0xF << (((i - 8) & 0xF) * 4))) | (((mode >> 8) & 0xF) << (((i - 8) & 0xF) * 4));
                }
            }
        }
    }
    else if (mode == PAL_MODE_INPUT_ANALOG)
    {
        modeval = 0x00000000;
    }
    else if (mode == PAL_MODE_RESET)
    {
        modeval = 0x44444444;
    }
    else if ((mode & 0xff) == PAL_MODE_CH32_ALTERNATE_PUSHPULL)
    {
        modeval = 0xbbbbbbbb;
        enableHB2(RCC_AFIOEN);
        uint32_t afioreg = ((((uint32_t)port - (uint32_t)IOPORT1)) / 0x400 * 8) + (uint32_t)&( AFIO->GPIOA_AFLR);
        for(size_t i = 0; i < PAL_IOPORTS_WIDTH; i++){
            if((1U << i) & mask){
                if(i < 8)
                {
                    *(uint32_t *)afioreg = (*(uint32_t *)afioreg & ~(0xF << ((i & 0xF) * 4))) | (((mode >> 8) & 0xF) << ((i & 0xF) * 4));
                }else if(i < 16)
                {
                    *(uint32_t *)(afioreg + 4) = (*(uint32_t *)(afioreg + 4) & ~(0xF << (((i - 8) & 0xF) * 4))) | (((mode >> 8) & 0xF) << (((i - 8) & 0xF) * 4));
                }
            }
        }
    }
    else if ((mode & 0xff) == PAL_MODE_CH32_ALTERNATE_OPENDRAIN)
    {
        modeval = 0xffffffff;
        enableHB2(RCC_AFIOEN);
        uint32_t afioreg = ((((uint32_t)port - (uint32_t)IOPORT1)) / 0x400 * 8) + (uint32_t)&(AFIO->GPIOA_AFLR);
        for(size_t i = 0; i < PAL_IOPORTS_WIDTH; i++){
            if((1U << i) & mask){
                if(i < 8)
                {
                    *(uint32_t *)afioreg = (*(uint32_t *)afioreg & ~(0xF << ((i & 0xF) * 4))) | (((mode >> 8) & 0xF) << ((i & 0xF) * 4));
                }else if(i < 16)
                {
                    *(uint32_t *)(afioreg + 4) = (*(uint32_t *)(afioreg + 4) & ~(0xF << (((i - 8) & 0xF) * 4))) | (((mode >> 8) & 0xF) << (((i - 8) & 0xF) * 4));
                }
            }
        }
    }

    

    ioportid_t p = port;

    uint32_t _cfglrmask = 0, _cfghrmask = 0;
    for (int i = 7; i >= 0; i--)
    {
        _cfglrmask <<= 4;
        if (mask & (1 << i))
        {
            _cfglrmask |= 0xF;
        }
    }

    p->CFGLR = (p->CFGLR & ~_cfglrmask) | (_cfglrmask & modeval);

    for (int i = 15; i >= 8; i--)
    {
        _cfghrmask <<= 4;
        if (mask & (1 << (i)))
        {
            _cfghrmask |= 0xF;
        }
    }

    p->CFGHR = (p->CFGHR & ~_cfghrmask) | (_cfghrmask & modeval);

    p->OUTDR = (p->OUTDR & ~mask) | (mask & odrval);

    p->SPEED = 0xffffffff;
}

#if PAL_USE_CALLBACKS || PAL_USE_WAIT
void _pal_lld_enablepadevent(ioportid_t port, uint8_t pad, ioeventmode_t mode)
{
    uint32_t portnum = (((uint32_t)port - (uint32_t)IOPORT1) / 0x400);

    enableHB2(RCC_AFIOEN);

    if(pad < GPIO_PIN8){
        AFIO->EXTICR1 = (AFIO->EXTICR1 & ~(0xf << (pad * 4)) ) | (portnum << (pad * 4));
        NVIC_EnableIRQ(EXTI7_0_IRQn);
    }else{
        AFIO->EXTICR2 = (AFIO->EXTICR2 & ~(0xf << ((pad - 8) * 4)) ) | (portnum << ((pad - 8) * 4));
        NVIC_EnableIRQ(EXTI15_8_IRQn);
    }
    EXTI->INTENR |= (mode << pad);
    if(mode == PAL_EVENT_MODE_RISING_EDGE){
        EXTI->RTENR |= (1 << pad);
        EXTI->FTENR &= ~(1 << pad);
    }else if(mode == PAL_EVENT_MODE_FALLING_EDGE){
        EXTI->FTENR |= (1 << pad);
        EXTI->RTENR &= ~(1 << pad);
    }else if(mode == PAL_EVENT_MODE_BOTH_EDGES){
        EXTI->RTENR |= (1 << pad);
        EXTI->FTENR |= (1 << pad);
    }
}
void _pal_lld_disablepadevent(ioportid_t port, uint8_t pad)
{
    EXTI->INTENR &= ~(1 << pad);
    EXTI->RTENR &= ~(1 << pad);
    EXTI->FTENR &= ~(1 << pad);
    if (pad < GPIO_PIN8)
    {
        NVIC_DisableIRQ(EXTI7_0_IRQn);
    }
    else
    {
        NVIC_DisableIRQ(EXTI15_8_IRQn);
    }
}
#endif /* PAL_USE_CALLBACKS || PAL_USE_WAIT */

#endif /* HAL_USE_PAL == TRUE */

/** @} */
