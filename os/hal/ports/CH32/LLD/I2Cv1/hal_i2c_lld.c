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
 * @file    hal_i2c_lld.c
 * @brief   CH32 I2C (v1) subsystem low level driver source.
 *
 * @addtogroup I2C
 * @{
 */

#include "hal.h"

#if (HAL_USE_I2C == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   I2C1 driver identifier.
 */
#if (CH32_I2C_USE_I2C1 == TRUE) || defined(__DOXYGEN__)
I2CDriver I2CD1;
#endif

/**
 * @brief   I2C2 driver identifier.
 */
#if (CH32_I2C_USE_I2C2 == TRUE) || defined(__DOXYGEN__)
I2CDriver I2CD2;
#endif

/**
 * @brief   I2C3 driver identifier.
 */
#if (CH32_I2C_USE_I2C3 == TRUE) || defined(__DOXYGEN__)
I2CDriver I2CD3;
#endif

/**
 * @brief   I2C4 driver identifier.
 */
#if (CH32_I2C_USE_I2C4 == TRUE) || defined(__DOXYGEN__)
I2CDriver I2CD4;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   DMA RX error callback.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 * @param[in] flags     pre-shifted content of the ISR register
 */
static void i2c_lld_serve_rx_interrupt(I2CDriver *i2cp, uint32_t flags) {

  /* DMA errors handling.*/
#if defined(CH32_I2C_DMA_ERROR_HOOK)
  if ((flags & (DMA_CFGR1_TEIE)) != 0) {
    i2cp->errors |= I2C_ERR_DMA;
    CH32_I2C_DMA_ERROR_HOOK(i2cp);
  }
#else
  (void)i2cp;
  (void)flags;
#endif
}

/**
 * @brief   DMA TX error callback.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 * @param[in] flags     pre-shifted content of the ISR register
 */
static void i2c_lld_serve_tx_interrupt(I2CDriver *i2cp, uint32_t flags) {

  /* DMA errors handling.*/
#if defined(CH32_I2C_DMA_ERROR_HOOK)
  if ((flags & (DMA_CFGR1_TEIE)) != 0) {
    i2cp->errors |= I2C_ERR_DMA;
    CH32_I2C_DMA_ERROR_HOOK(i2cp);
  }
#else
  (void)i2cp;
  (void)flags;
#endif
}

/**
 * @brief   I2C event interrupt handler (state machine).
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 */
static void i2c_lld_serve_event_interrupt(I2CDriver *i2cp) {
  uint16_t star1, star2;

  star1 = i2cp->i2c->STAR1;
  star2 = i2cp->i2c->STAR2;

  /* Check for errors first.*/
  if (star1 & (I2C_STAR1_BERR | I2C_STAR1_ARLO | I2C_STAR1_AF |
               I2C_STAR1_OVR | I2C_STAR1_TIMEOUT)) {
    if (star1 & I2C_STAR1_BERR)    i2cp->errors |= I2C_ERR_BERR;
    if (star1 & I2C_STAR1_ARLO)    i2cp->errors |= I2C_ERR_ARLO;
    if (star1 & I2C_STAR1_AF)      i2cp->errors |= I2C_ERR_AF;
    if (star1 & I2C_STAR1_OVR)     i2cp->errors |= I2C_ERR_OVR;
    if (star1 & I2C_STAR1_TIMEOUT) i2cp->errors |= I2C_ERR_TIMEOUT;

    /* Disable event, buffer and error interrupts.*/
    i2cp->i2c->CTLR2 &= ~(I2C_CTLR2_ITEVTEN | I2C_CTLR2_ITBUFEN |
                          I2C_CTLR2_ITERREN);
    if (star2 & I2C_STAR2_MSL) {
      i2cp->i2c->CTLR1 |= I2C_CTLR1_STOP;
    }

    /* Wake up the waiting thread signaling error.*/
    if (i2cp->thread != NULL) {
      osalSysLockFromISR();
      osalThreadResumeI(&i2cp->thread, MSG_RESET);
      osalSysUnlockFromISR();
    }
    return;
  }

  /* SB: Start bit sent (master mode).*/
  if (star1 & I2C_STAR1_SB) {
    if (i2cp->addr & 0x8000) {
      /* 10-bit addressing: send header 1111 0XX R/W.*/
      if (i2cp->txbytes > 0) {
        i2cp->i2c->DATAR = ((i2cp->addr >> 8) & 0x06) | 0xF0;
      }
      else {
        i2cp->i2c->DATAR = ((i2cp->addr >> 8) & 0x06) | 0xF0 | 0x01;
      }
    }
    else {
      /* 7-bit addressing: send address byte.*/
      if (i2cp->txbytes > 0) {
        i2cp->i2c->DATAR = (i2cp->addr & 0xFE);
      }
      else {
        i2cp->i2c->DATAR = (i2cp->addr & 0xFE) | 0x01;
      }
    }
    return;
  }

  /* ADD10: 10-bit address header sent.*/
  if (star1 & I2C_STAR1_ADD10) {
    i2cp->i2c->DATAR = i2cp->addr & 0xFF;
    return;
  }

  /* ADDR: Address sent (master) or matched (slave).*/
  if (star1 & I2C_STAR1_ADDR) {
    /* Clear ADDR by reading STAR2.*/
    (void)i2cp->i2c->STAR2;

    if (star2 & I2C_STAR2_MSL) {
      /* Master mode.*/
      if (star2 & I2C_STAR2_TRA) {
        /* Transmitter: enable buffer interrupt for TX data.*/
        if (i2cp->txbytes > 0) {
          i2cp->i2c->CTLR2 |= I2C_CTLR2_ITBUFEN;
        }
        else {
          i2cp->i2c->CTLR2 &= ~I2C_CTLR2_ITBUFEN;
        }
      }
      else {
        /* Receiver: configure for number of bytes to receive.*/
        if (i2cp->rxbytes == 1) {
          i2cp->i2c->CTLR1 &= ~I2C_CTLR1_ACK;
          i2cp->i2c->CTLR1 |= I2C_CTLR1_STOP;
        }
        else if (i2cp->rxbytes == 2) {
          i2cp->i2c->CTLR1 |= I2C_CTLR1_POS;
          i2cp->i2c->CTLR1 |= I2C_CTLR1_ACK;
        }
        else {
          i2cp->i2c->CTLR1 |= I2C_CTLR1_ACK;
        }
      }
    }
    return;
  }

  /* BTF: Byte transfer finished.*/
  if (star1 & I2C_STAR1_BTF) {
    if (star2 & I2C_STAR2_TRA) {
      /* Transmitter: last byte transferred.*/
      i2cp->i2c->CTLR1 |= I2C_CTLR1_STOP;
      i2cp->i2c->CTLR2 &= ~(I2C_CTLR2_ITEVTEN | I2C_CTLR2_ITBUFEN |
                            I2C_CTLR2_ITERREN);

      if (i2cp->rxbytes > 0) {
        /* Combined TX/RX: issue repeated START for receive phase.*/
        i2cp->i2c->CTLR2 |= I2C_CTLR2_ITEVTEN | I2C_CTLR2_ITERREN;
        i2cp->i2c->CTLR1 |= I2C_CTLR1_START;
      }
      else {
        /* No more RX: done.*/
        if (i2cp->thread != NULL) {
          osalSysLockFromISR();
          osalThreadResumeI(&i2cp->thread, MSG_OK);
          osalSysUnlockFromISR();
        }
      }
    }
    return;
  }

  /* TXE + TRA: Data register empty and in transmitter mode.*/
  if ((star1 & I2C_STAR1_TXE) && (star2 & I2C_STAR2_TRA)) {
    if (i2cp->txbytes > 0) {
      i2cp->i2c->DATAR = *i2cp->txbuf;
      i2cp->txbuf++;
      i2cp->txbytes--;

      if (i2cp->txbytes == 0) {
        if (i2cp->rxbytes == 0) {
          /* All TX done, no more RX: STOP.*/
          i2cp->i2c->CTLR1 |= I2C_CTLR1_STOP;
          i2cp->i2c->CTLR2 &= ~(I2C_CTLR2_ITEVTEN | I2C_CTLR2_ITBUFEN |
                                I2C_CTLR2_ITERREN);
          if (i2cp->thread != NULL) {
            osalSysLockFromISR();
            osalThreadResumeI(&i2cp->thread, MSG_OK);
            osalSysUnlockFromISR();
          }
        }
        else {
          /* TX done, RX pending: disable buffer interrupt, wait for BTF.*/
          i2cp->i2c->CTLR2 &= ~I2C_CTLR2_ITBUFEN;
        }
      }
    }
    return;
  }

  /* RXNE + !TRA: Data register not empty and in receiver mode.*/
  if ((star1 & I2C_STAR1_RXNE) && !(star2 & I2C_STAR2_TRA)) {
    if (i2cp->rxbytes > 0) {
      if (i2cp->rxbytes > 2) {
        /* More than 2 bytes remaining: normal read.*/
        *i2cp->rxbuf = i2cp->i2c->DATAR;
        i2cp->rxbuf++;
        i2cp->rxbytes--;
      }
      else if (i2cp->rxbytes == 2) {
        /* 2 bytes remaining: disable ACK, set POS, then read.*/
        i2cp->i2c->CTLR1 &= ~I2C_CTLR1_ACK;
        i2cp->i2c->CTLR1 |= I2C_CTLR1_POS;
        *i2cp->rxbuf = i2cp->i2c->DATAR;
        i2cp->rxbuf++;
        i2cp->rxbytes--;
      }
      else {
        /* 1 byte remaining: generate STOP, then read.*/
        i2cp->i2c->CTLR1 |= I2C_CTLR1_STOP;
        *i2cp->rxbuf = i2cp->i2c->DATAR;
        i2cp->rxbuf++;
        i2cp->rxbytes--;

        if (i2cp->rxbytes == 0) {
          i2cp->i2c->CTLR2 &= ~(I2C_CTLR2_ITEVTEN | I2C_CTLR2_ITBUFEN |
                                I2C_CTLR2_ITERREN);
          if (i2cp->thread != NULL) {
            osalSysLockFromISR();
            osalThreadResumeI(&i2cp->thread, MSG_OK);
            osalSysUnlockFromISR();
          }
        }
      }
    }
    return;
  }

  /* STOPF: Stop condition detected (slave mode).*/
  if (star1 & I2C_STAR1_STOPF) {
    (void)i2cp->i2c->STAR1;
    i2cp->i2c->CTLR1 |= 0x0000;
    return;
  }
}

/**
 * @brief   I2C error interrupt handler.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 */
static void i2c_lld_serve_error_interrupt(I2CDriver *i2cp) {
  uint16_t star1;

  star1 = i2cp->i2c->STAR1;
  (void)i2cp->i2c->STAR2;

  if (star1 & I2C_STAR1_BERR)    i2cp->errors |= I2C_ERR_BERR;
  if (star1 & I2C_STAR1_ARLO)    i2cp->errors |= I2C_ERR_ARLO;
  if (star1 & I2C_STAR1_AF)      i2cp->errors |= I2C_ERR_AF;
  if (star1 & I2C_STAR1_OVR)     i2cp->errors |= I2C_ERR_OVR;
  if (star1 & I2C_STAR1_TIMEOUT) i2cp->errors |= I2C_ERR_TIMEOUT;

  /* Generate STOP and disable interrupts.*/
  i2cp->i2c->CTLR1 |= I2C_CTLR1_STOP;
  i2cp->i2c->CTLR2 &= ~(I2C_CTLR2_ITEVTEN | I2C_CTLR2_ITBUFEN |
                        I2C_CTLR2_ITERREN);

  /* Wake up waiting thread with error status.*/
  if (i2cp->thread != NULL) {
    osalSysLockFromISR();
    osalThreadResumeI(&i2cp->thread, MSG_RESET);
    osalSysUnlockFromISR();
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

OSAL_IRQ_HANDLER(I2C1_EV_IRQHandler) {
  i2c_lld_serve_event_interrupt(&I2CD1);
}

OSAL_IRQ_HANDLER(I2C1_ER_IRQHandler) {
  i2c_lld_serve_error_interrupt(&I2CD1);
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level I2C driver initialization.
 *
 * @notapi
 */
void i2c_lld_init(void) {

#if (CH32_I2C_USE_I2C1 == TRUE)
  i2cObjectInit(&I2CD1);
  I2CD1.i2c       = I2C1;
  I2CD1.dmarx     = NULL;
  I2CD1.dmatx     = NULL;
  I2CD1.thread    = NULL;
  I2CD1.rxdmamode = DMA_CFGR1_TCIE | DMA_CFGR1_TEIE;
  I2CD1.txdmamode = DMA_CFGR1_TEIE;
#endif

#if (CH32_I2C_USE_I2C2 == TRUE)
  i2cObjectInit(&I2CD2);
  I2CD2.i2c       = I2C2;
  I2CD2.dmarx     = NULL;
  I2CD2.dmatx     = NULL;
  I2CD2.thread    = NULL;
  I2CD2.rxdmamode = DMA_CFGR1_TCIE | DMA_CFGR1_TEIE;
  I2CD2.txdmamode = DMA_CFGR1_TEIE;
#endif

#if (CH32_I2C_USE_I2C3 == TRUE)
  i2cObjectInit(&I2CD3);
  I2CD3.i2c       = I2C3;
  I2CD3.dmarx     = NULL;
  I2CD3.dmatx     = NULL;
  I2CD3.thread    = NULL;
  I2CD3.rxdmamode = DMA_CFGR1_TCIE | DMA_CFGR1_TEIE;
  I2CD3.txdmamode = DMA_CFGR1_TEIE;
#endif

#if (CH32_I2C_USE_I2C4 == TRUE)
  i2cObjectInit(&I2CD4);
  I2CD4.i2c       = I2C4;
  I2CD4.dmarx     = NULL;
  I2CD4.dmatx     = NULL;
  I2CD4.thread    = NULL;
  I2CD4.rxdmamode = DMA_CFGR1_TCIE | DMA_CFGR1_TEIE;
  I2CD4.txdmamode = DMA_CFGR1_TEIE;
#endif
}

/**
 * @brief   Configures and activates the I2C peripheral.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
void i2c_lld_start(I2CDriver *i2cp) {

  if (i2cp->state == I2C_STOP) {

#if (CH32_I2C_USE_I2C1 == TRUE)
    if (&I2CD1 == i2cp) {
      /* Allocate RX DMA channel.*/
      i2cp->dmarx = dmaStreamAllocI(CH32_I2C_I2C1_RX_DMA_STREAM,
                                    CH32_I2C_I2C1_IRQ_PRIORITY,
                                    (ch32_dmaisr_t)i2c_lld_serve_rx_interrupt,
                                    (void *)i2cp);
      osalDbgAssert(i2cp->dmarx != NULL, "unable to allocate stream");

      /* Allocate TX DMA channel.*/
      i2cp->dmatx = dmaStreamAllocI(CH32_I2C_I2C1_TX_DMA_STREAM,
                                    CH32_I2C_I2C1_IRQ_PRIORITY,
                                    (ch32_dmaisr_t)i2c_lld_serve_tx_interrupt,
                                    (void *)i2cp);
      osalDbgAssert(i2cp->dmatx != NULL, "unable to allocate stream");

      /* Enable I2C1 clock on HB2 bus.*/
      resetHB2(RCC_I2C1RST);
      enableHB2(RCC_I2C1EN);

      /* Set DMAMUX request sources.*/
      dmaSetRequestSource(i2cp->dmarx, DMA_MUX_I2C1_RX);
      dmaSetRequestSource(i2cp->dmatx, DMA_MUX_I2C1_TX);

      /* Enable NVIC interrupts for I2C1.*/
      NVIC->IPRIOR[46] = CH32_I2C_I2C1_IRQ_PRIORITY;
      NVIC->IPRIOR[47] = CH32_I2C_I2C1_IRQ_PRIORITY;
      NVIC->IENR[1] |= (1U << 14);  /* I2C1_EV_IRQn = 46 */
      NVIC->IENR[1] |= (1U << 15);  /* I2C1_ER_IRQn = 47 */
    }
#endif

#if (CH32_I2C_USE_I2C2 == TRUE)
    if (&I2CD2 == i2cp) {
      i2cp->dmarx = dmaStreamAllocI(CH32_I2C_I2C2_RX_DMA_STREAM,
                                    CH32_I2C_I2C2_IRQ_PRIORITY,
                                    (ch32_dmaisr_t)i2c_lld_serve_rx_interrupt,
                                    (void *)i2cp);
      osalDbgAssert(i2cp->dmarx != NULL, "unable to allocate stream");

      i2cp->dmatx = dmaStreamAllocI(CH32_I2C_I2C2_TX_DMA_STREAM,
                                    CH32_I2C_I2C2_IRQ_PRIORITY,
                                    (ch32_dmaisr_t)i2c_lld_serve_tx_interrupt,
                                    (void *)i2cp);
      osalDbgAssert(i2cp->dmatx != NULL, "unable to allocate stream");

      resetHB1(RCC_I2C2RST);
      enableHB1(RCC_I2C2EN);

      dmaSetRequestSource(i2cp->dmarx, DMA_MUX_I2C2_RX);
      dmaSetRequestSource(i2cp->dmatx, DMA_MUX_I2C2_TX);

      NVIC->IPRIOR[52] = CH32_I2C_I2C2_IRQ_PRIORITY;
      NVIC->IPRIOR[53] = CH32_I2C_I2C2_IRQ_PRIORITY;
      NVIC->IENR[1] |= (1U << 20);  /* I2C2_EV_IRQn = 52 */
      NVIC->IENR[1] |= (1U << 21);  /* I2C2_ER_IRQn = 53 */
    }
#endif

#if (CH32_I2C_USE_I2C3 == TRUE)
    if (&I2CD3 == i2cp) {
      i2cp->dmarx = dmaStreamAllocI(CH32_I2C_I2C3_RX_DMA_STREAM,
                                    CH32_I2C_I2C3_IRQ_PRIORITY,
                                    (ch32_dmaisr_t)i2c_lld_serve_rx_interrupt,
                                    (void *)i2cp);
      osalDbgAssert(i2cp->dmarx != NULL, "unable to allocate stream");

      i2cp->dmatx = dmaStreamAllocI(CH32_I2C_I2C3_TX_DMA_STREAM,
                                    CH32_I2C_I2C3_IRQ_PRIORITY,
                                    (ch32_dmaisr_t)i2c_lld_serve_tx_interrupt,
                                    (void *)i2cp);
      osalDbgAssert(i2cp->dmatx != NULL, "unable to allocate stream");

      resetHB1(RCC_I2C3RST);
      enableHB1(RCC_I2C3EN);

      dmaSetRequestSource(i2cp->dmarx, DMA_MUX_I2C3_RX);
      dmaSetRequestSource(i2cp->dmatx, DMA_MUX_I2C3_TX);

      NVIC->IPRIOR[78] = CH32_I2C_I2C3_IRQ_PRIORITY;
      NVIC->IPRIOR[79] = CH32_I2C_I2C3_IRQ_PRIORITY;
      NVIC->IENR[2] |= (1U << 14);  /* I2C3_EV_IRQn = 78 */
      NVIC->IENR[2] |= (1U << 15);  /* I2C3_ER_IRQn = 79 */
    }
#endif

#if (CH32_I2C_USE_I2C4 == TRUE)
    if (&I2CD4 == i2cp) {
      i2cp->dmarx = dmaStreamAllocI(CH32_I2C_I2C4_RX_DMA_STREAM,
                                    CH32_I2C_I2C4_IRQ_PRIORITY,
                                    (ch32_dmaisr_t)i2c_lld_serve_rx_interrupt,
                                    (void *)i2cp);
      osalDbgAssert(i2cp->dmarx != NULL, "unable to allocate stream");

      i2cp->dmatx = dmaStreamAllocI(CH32_I2C_I2C4_TX_DMA_STREAM,
                                    CH32_I2C_I2C4_IRQ_PRIORITY,
                                    (ch32_dmaisr_t)i2c_lld_serve_tx_interrupt,
                                    (void *)i2cp);
      osalDbgAssert(i2cp->dmatx != NULL, "unable to allocate stream");

      resetHB2(RCC_I2C4RST);
      enableHB2(RCC_I2C4EN);

      dmaSetRequestSource(i2cp->dmarx, DMA_MUX_I2C4_RX);
      dmaSetRequestSource(i2cp->dmatx, DMA_MUX_I2C4_TX);

      NVIC->IPRIOR[80] = CH32_I2C_I2C4_IRQ_PRIORITY;
      NVIC->IPRIOR[81] = CH32_I2C_I2C4_IRQ_PRIORITY;
      NVIC->IENR[2] |= (1U << 16);  /* I2C4_EV_IRQn = 80 */
      NVIC->IENR[2] |= (1U << 17);  /* I2C4_ER_IRQn = 81 */
    }
#endif

    /* DMA peripheral address setup - both streams point to the data reg.*/
    dmaStreamSetPeripheral(i2cp->dmarx, &i2cp->i2c->DATAR);
    dmaStreamSetPeripheral(i2cp->dmatx, &i2cp->i2c->DATAR);
  }

  /* I2C register configuration.*/
  {
    uint32_t pclk;
    uint32_t freq;
    uint16_t ccr;
    uint16_t trise;
    uint16_t reg;
    RCC_ClocksTypeDef Clocks;

    /* Disable I2C peripheral while configuring.*/
    i2cp->i2c->CTLR1 &= ~I2C_CTLR1_PE;

    /* Get peripheral clock frequency.*/
    RCC_GetClocksFreq(&Clocks);
    pclk = hal_lld_get_clock_point(Clocks.HCLK_Frequency);

    /* Set the FREQ field in CTLR2 (peripheral clock in MHz).*/
    freq = pclk / 1000000;
    i2cp->i2c->CTLR2 = (i2cp->i2c->CTLR2 & ~I2C_CTLR2_FREQ) | freq;

    /* Calculate clock control (CCR) value.*/
    if (i2cp->config->clock_speed <= 100000) {
      /* Standard mode (100 kHz or less): Tlow/Thigh = 1/1.*/
      ccr = (uint16_t)(pclk / (i2cp->config->clock_speed * 2));
      trise = (uint16_t)(freq + 1);
      i2cp->i2c->RTR = trise;
      i2cp->i2c->CKCFGR = ccr & I2C_CKCFGR_CCR;
    }
    else {
      /* Fast mode (up to 400 kHz).*/
      uint32_t tmp;

      trise = (uint16_t)((pclk * 300) / 1000000000) + 1;
      i2cp->i2c->RTR = trise;

      if (i2cp->config->duty_cycle == FAST_DUTY_CYCLE_2) {
        /* Tlow/Thigh = 2/1.*/
        tmp = pclk / (i2cp->config->clock_speed * 3);
        ccr = (uint16_t)(tmp & I2C_CKCFGR_CCR);
      }
      else {
        /* Tlow/Thigh = 16/9.*/
        tmp = pclk / (i2cp->config->clock_speed * 25);
        ccr = (uint16_t)((tmp & I2C_CKCFGR_CCR) | I2C_CKCFGR_DUTY);
      }
      ccr |= I2C_CKCFGR_FS;
      i2cp->i2c->CKCFGR = ccr;
    }

    /* Ensure minimum CCR value.*/
    reg = i2cp->i2c->CKCFGR;
    if ((reg & I2C_CKCFGR_CCR) < 4) {
      i2cp->i2c->CKCFGR = (reg & ~I2C_CKCFGR_CCR) | 4;
    }

    /* Enable ACK.*/
    i2cp->i2c->CTLR1 = I2C_CTLR1_ACK;

    /* Re-enable I2C peripheral.*/
    i2cp->i2c->CTLR1 |= I2C_CTLR1_PE;
  }
}

/**
 * @brief   Deactivates the I2C peripheral.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
void i2c_lld_stop(I2CDriver *i2cp) {

  if (i2cp->state != I2C_STOP) {

    /* Disable I2C peripheral.*/
    i2cp->i2c->CTLR1 &= ~I2C_CTLR1_PE;
    i2cp->i2c->CTLR1 = 0;
    i2cp->i2c->CTLR2 = 0;

    /* Free DMA streams.*/
    dmaStreamFreeI(i2cp->dmarx);
    dmaStreamFreeI(i2cp->dmatx);
    i2cp->dmarx = NULL;
    i2cp->dmatx = NULL;

    /* Disable peripheral clock.*/
#if (CH32_I2C_USE_I2C1 == TRUE)
    if (&I2CD1 == i2cp) {
      /* Disable NVIC interrupts.*/
      NVIC->IENR[1] &= ~(1U << 14);
      NVIC->IENR[1] &= ~(1U << 15);
      resetHB2(RCC_I2C1RST);
      disableHB2(RCC_I2C1EN);
    }
#endif
#if (CH32_I2C_USE_I2C2 == TRUE)
    if (&I2CD2 == i2cp) {
      NVIC->IENR[1] &= ~(1U << 20);
      NVIC->IENR[1] &= ~(1U << 21);
      resetHB1(RCC_I2C2RST);
      disableHB1(RCC_I2C2EN);
    }
#endif
#if (CH32_I2C_USE_I2C3 == TRUE)
    if (&I2CD3 == i2cp) {
      NVIC->IENR[2] &= ~(1U << 14);
      NVIC->IENR[2] &= ~(1U << 15);
      resetHB1(RCC_I2C3RST);
      disableHB1(RCC_I2C3EN);
    }
#endif
#if (CH32_I2C_USE_I2C4 == TRUE)
    if (&I2CD4 == i2cp) {
      NVIC->IENR[2] &= ~(1U << 16);
      NVIC->IENR[2] &= ~(1U << 17);
      resetHB2(RCC_I2C4RST);
      disableHB2(RCC_I2C4EN);
    }
#endif
  }
}

/**
 * @brief   Receives data via the I2C bus as master.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 * @param[in] addr      slave device address
 * @param[out] rxbuf    pointer to the receive buffer
 * @param[in] rxbytes   number of bytes to be received
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_INFINITE no timeout.
 * @return              The operation status.
 * @retval MSG_OK       if the function succeeded.
 * @retval MSG_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval MSG_TIMEOUT  if a timeout occurred before operation end. <b>After a
 *                      timeout the driver must be stopped and restarted
 *                      because the bus is in an uncertain state</b>.
 *
 * @notapi
 */
msg_t i2c_lld_master_receive_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                     uint8_t *rxbuf, size_t rxbytes,
                                     sysinterval_t timeout) {

  /* Delegate to transmit function with txbytes = 0 (receive only).*/
  return i2c_lld_master_transmit_timeout(i2cp, addr, NULL, 0,
                                         rxbuf, rxbytes, timeout);
}

/**
 * @brief   Transmits data via the I2C bus as master.
 * @details Combined transmit-then-receive operation.
 *          If rxbytes is 0, only transmit is performed.
 *          If txbytes is 0, only receive is performed.
 *          If both are non-zero, transmit is followed by repeated START
 *          and receive.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 * @param[in] addr      slave device address
 * @param[in] txbuf     pointer to the transmit buffer
 * @param[in] txbytes   number of bytes to be transmitted
 * @param[out] rxbuf    pointer to the receive buffer
 * @param[in] rxbytes   number of bytes to be received
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_INFINITE no timeout.
 * @return              The operation status.
 * @retval MSG_OK       if the function succeeded.
 * @retval MSG_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval MSG_TIMEOUT  if a timeout occurred before operation end. <b>After a
 *                      timeout the driver must be stopped and restarted
 *                      because the bus is in an uncertain state</b>.
 *
 * @notapi
 */
msg_t i2c_lld_master_transmit_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                      const uint8_t *txbuf, size_t txbytes,
                                      uint8_t *rxbuf, size_t rxbytes,
                                      sysinterval_t timeout) {
  msg_t msg;

  /* Setup operation parameters.*/
  i2cp->errors  = 0;
  i2cp->addr    = addr;
  i2cp->txbuf   = txbuf;
  i2cp->txbytes = txbytes;
  i2cp->rxbuf   = rxbuf;
  i2cp->rxbytes = rxbytes;
  i2cp->thread  = NULL;

  /* Enable I2C event and error interrupts.*/
  i2cp->i2c->CTLR2 |= I2C_CTLR2_ITEVTEN | I2C_CTLR2_ITERREN;

  /* Ensure ACK is enabled.*/
  i2cp->i2c->CTLR1 |= I2C_CTLR1_ACK;

  /* Register current thread and send START condition.
     Note: caller (i2cMasterTransmitTimeout) already holds osalSysLock,
     so we use S-class APIs directly without additional lock/unlock.
     chThdSuspendTimeoutS requires *trp == NULL and sets it internally.*/
  i2cp->i2c->CTLR1 |= I2C_CTLR1_START;
  msg = chThdSuspendTimeoutS(&i2cp->thread, timeout);

  if (msg == MSG_TIMEOUT) {
    /* Timeout: disable interrupts, send STOP, reset peripheral.*/
    i2cp->i2c->CTLR2 &= ~(I2C_CTLR2_ITEVTEN | I2C_CTLR2_ITBUFEN |
                          I2C_CTLR2_ITERREN);
    i2cp->i2c->CTLR1 |= I2C_CTLR1_STOP;
    i2cp->i2c->CTLR1 &= ~I2C_CTLR1_PE;
    i2cp->i2c->CTLR1 = 0;
    i2cp->i2c->CTLR1 = I2C_CTLR1_PE | I2C_CTLR1_ACK;
  }

  return msg;
}

#endif /* HAL_USE_I2C == TRUE */

/** @} */
