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
 * @file    hal_usb_lld.c
 * @brief   CH32H417 USBFS (OTGv1) USB subsystem low level driver source.
 *
 * @addtogroup USB
 * @{
 */

#include "hal.h"

#if (HAL_USE_USB == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   USB1 driver identifier.
 */
#if (CH32_OTG_USE_USB1 == TRUE) || defined(__DOXYGEN__)
USBDriver USBD1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   EP0 state.
 * @note    It is an union because IN and OUT endpoints are never used at the
 *          same time for EP0.
 */
static union {
  /**
   * @brief   IN EP0 state.
   */
  USBInEndpointState in;
  /**
   * @brief   OUT EP0 state.
   */
  USBOutEndpointState out;
} ep0_state;

/**
 * @brief   Buffer for the EP0 setup packets.
 */
static uint8_t ep0setup_buffer[64];

/**
 * @brief   EP0 initialization structure.
 */
static const USBEndpointConfig ep0config = {
  USB_EP_MODE_TYPE_CTRL,
  _usb_ep0setup,
  _usb_ep0in,
  _usb_ep0out,
  DEF_USBD_UEP0_SIZE,
  DEF_USBD_UEP0_SIZE,
  &ep0_state.in,
  &ep0_state.out
};

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Endpoint service routine.
 * @details Handles IN/OUT transactions for all endpoints.
 */
static void usb_serve_endpoints(USBDriver *usbp, uint8_t intst)
{
  (void)usbp;
  uint8_t ep = intst & USBFS_UIS_ENDP_MASK;
  uint8_t token = intst & USBFS_UIS_TOKEN_MASK;
  volatile uint16_t *rxlen_reg = NULL;
  volatile uint8_t *txctlr_reg = NULL;
  volatile uint8_t *rxctlr_reg = NULL;
  volatile uint32_t *dma_reg = NULL;
  volatile uint16_t *txlen_reg = NULL;

  /* Map registers for the endpoint */
  switch (ep)
  {
  case 0:
    dma_reg   = &(CH32_USBFS_DEVICE->UEP0_DMA);
    txlen_reg = (volatile uint16_t *)&(CH32_USBFS_DEVICE->UEP0_TX_LEN);
    txctlr_reg = &(CH32_USBFS_DEVICE->UEP0_TX_CTRL);
    rxctlr_reg = &(CH32_USBFS_DEVICE->UEP0_RX_CTRL);
    rxlen_reg  = &(CH32_USBFS_DEVICE->RX_LEN);
    break;
  case 1:
    dma_reg   = &(CH32_USBFS_DEVICE->UEP1_DMA);
    txlen_reg = (volatile uint16_t *)&(CH32_USBFS_DEVICE->UEP1_TX_LEN);
    txctlr_reg = &(CH32_USBFS_DEVICE->UEP1_TX_CTRL);
    rxctlr_reg = &(CH32_USBFS_DEVICE->UEP1_RX_CTRL);
    rxlen_reg  = &(CH32_USBFS_DEVICE->RX_LEN);
    break;
  case 2:
    dma_reg   = &(CH32_USBFS_DEVICE->UEP2_DMA);
    txlen_reg = (volatile uint16_t *)&(CH32_USBFS_DEVICE->UEP2_TX_LEN);
    txctlr_reg = &(CH32_USBFS_DEVICE->UEP2_TX_CTRL);
    rxctlr_reg = &(CH32_USBFS_DEVICE->UEP2_RX_CTRL);
    rxlen_reg  = &(CH32_USBFS_DEVICE->RX_LEN);
    break;
  case 3:
    dma_reg   = &(CH32_USBFS_DEVICE->UEP3_DMA);
    txlen_reg = (volatile uint16_t *)&(CH32_USBFS_DEVICE->UEP3_TX_LEN);
    txctlr_reg = &(CH32_USBFS_DEVICE->UEP3_TX_CTRL);
    rxctlr_reg = &(CH32_USBFS_DEVICE->UEP3_RX_CTRL);
    rxlen_reg  = &(CH32_USBFS_DEVICE->RX_LEN);
    break;
  default:
    return;
  }

  if (token == USBFS_UIS_TOKEN_IN)
  {
    /* IN transaction */
    usb_lld_clear_in(usbp, ep);

    USBInEndpointState *iesp = usbp->epc[ep]->in_state;
    iesp->txcnt += iesp->txlast;

    if (iesp->txsize - iesp->txcnt == 0)
    {
      /* Transfer complete */
      _usb_isr_invoke_in_cb(usbp, ep);

      if ((ep == 0) && (usbp->ep0state == USB_EP0_STP_WAITING))
      {
        /* Prepare EP0 for next setup */
        USBOutEndpointState *oesp = usbp->epc[0]->out_state;
        oesp->rxpkts = 1;
        oesp->rxbuf = ep0setup_buffer;
        oesp->rxsize = 8;
        *dma_reg = (uint32_t)oesp->rxbuf;
        *rxctlr_reg &= ~USBFS_UEP_R_TOG;
        *rxctlr_reg = (*rxctlr_reg & ~USBFS_UEP_R_RES_MASK) | USBFS_UEP_R_RES_ACK;
      }
    }
    else
    {
      /* More data to send */
      size_t remaining = iesp->txsize - iesp->txcnt;
      iesp->txlast = (remaining < usbp->epc[ep]->in_maxsize) ?
                      remaining : usbp->epc[ep]->in_maxsize;

      *dma_reg = (uint32_t)(iesp->txbuf + iesp->txcnt);
      *txlen_reg = (uint16_t)iesp->txlast;
      *txctlr_reg ^= USBFS_UEP_T_TOG;
      *txctlr_reg = (*txctlr_reg & ~USBFS_UEP_T_RES_MASK) | USBFS_UEP_T_RES_ACK;
    }
  }
  else if ((token == USBFS_UIS_TOKEN_OUT) || (token == USBFS_UIS_TOKEN_SETUP))
  {
    /* OUT or SETUP transaction */
    usb_lld_clear_out(usbp, ep);

    USBOutEndpointState *oesp = usbp->epc[ep]->out_state;
    uint16_t rx_len = *rxlen_reg;

    oesp->rxcnt += rx_len;
    oesp->rxsize -= rx_len;
    oesp->rxpkts--;

    if (token == USBFS_UIS_TOKEN_SETUP)
    {
      /* Setup packet received - data is in the EP0 buffer via DMA */
      *txctlr_reg |= USBFS_UEP_T_TOG;
      *rxctlr_reg |= USBFS_UEP_R_TOG;
      _usb_isr_invoke_setup_cb(usbp, ep);
    }
    else
    {
      /* OUT transaction */
      if ((oesp->rxpkts == 0) || (rx_len < usbp->epc[ep]->out_maxsize))
      {
        /* Transfer complete */
        _usb_isr_invoke_out_cb(usbp, ep);

        if ((ep == 0) && (usbp->ep0state == USB_EP0_STP_WAITING))
        {
          /* Re-arm EP0 for next setup */
          oesp->rxpkts = 1;
          oesp->rxbuf = ep0setup_buffer;
          oesp->rxsize = 8;
          *dma_reg = (uint32_t)oesp->rxbuf;
          *rxctlr_reg &= ~USBFS_UEP_R_TOG;
          *rxctlr_reg = (*rxctlr_reg & ~USBFS_UEP_R_RES_MASK) | USBFS_UEP_R_RES_ACK;
        }
      }
      else
      {
        /* More data to receive */
        *rxctlr_reg ^= USBFS_UEP_R_TOG;
        *dma_reg = (uint32_t)oesp->rxbuf;
        *rxctlr_reg = (*rxctlr_reg & ~USBFS_UEP_R_RES_MASK) | USBFS_UEP_R_RES_ACK;
      }
    }
  }
}

/*===========================================================================*/
/* Driver interrupt handlers and threads.                                    */
/*===========================================================================*/

/**
 * @brief   USBFS interrupt handler.
 */
OSAL_IRQ_HANDLER(USBFS_IRQHandler)
{
  uint8_t intflag = CH32_USBFS_DEVICE->INT_FG;
  uint8_t intst = CH32_USBFS_DEVICE->INT_ST;

  if (intflag & USBFS_UIF_BUS_RST)
  {
    /* Bus reset */
    CH32_USBFS_DEVICE->INT_FG = USBFS_UIF_BUS_RST;
    _usb_reset(&USBD1);
  }
  else if (intflag & USBFS_UIF_SUSPEND)
  {
    /* Suspend */
    CH32_USBFS_DEVICE->INT_FG = USBFS_UIF_SUSPEND;
    _usb_suspend(&USBD1);
  }
  else if (intflag & USBFS_UIF_TRANSFER)
  {
    /* Transfer completed */
    usb_serve_endpoints(&USBD1, intst);
    CH32_USBFS_DEVICE->INT_FG = USBFS_UIF_TRANSFER;
  }
  else
  {
    /* Clear any other interrupt flags */
    CH32_USBFS_DEVICE->INT_FG = intflag;
  }
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level USB driver initialization.
 *
 * @notapi
 */
void usb_lld_init(void)
{
#if CH32_OTG_USE_USB1 == TRUE
  /* Driver initialization.*/
  usbObjectInit(&USBD1);
#endif
}

/**
 * @brief   Enables the USBFS peripheral clock.
 * @note    Uses the same clock scheme as the reference example:
 *          USBFS clock from USBHSPLL via DIV10 to get 48MHz.
 */
static void usb_lld_clock_enable(void)
{


  /* If system PLL source is not USBHS, configure USBHS PLL for 480MHz */
  if ((RCC->PLLCFGR & RCC_SYSPLL_SEL) != RCC_SYSPLL_USBHS)
  {
    RCC->CTLR &= ~RCC_USBHS_PLLON;
    RCC->PLLCFGR2 &= ~RCC_USBHSPLLSRC;
    RCC->PLLCFGR2 |= RCC_USBHSPLLSRC_HSE;
    RCC->PLLCFGR2 &= ~RCC_USBHSPLL_REFSEL;
    RCC->PLLCFGR2 |= RCC_USBHSPLL_REFSEL_25MHZ;
    RCC->PLLCFGR2 &= ~RCC_USBHSPLL_IN_DIV;
    RCC->PLLCFGR2 |= RCC_USBHSPLL_IN_DIV1;
    RCC->CTLR |= RCC_USBHS_PLLON;
    while (!(RCC->CTLR & RCC_USBHS_PLLRDY))
      ;
  }

  // /* Enable USBHS PLL clock output for USBFS */
  // RCC->CFGR0 |= RCC_UTMION;

  /* Select USBFS clock source from USBHSPLL and set divider */
  RCC->CFGR2 &= ~RCC_USBFSSRC;
  RCC->CFGR2 |= RCC_USBFSSRC_USBHSPLL;
  RCC->CFGR2 &= ~RCC_USBFSDIV;
  RCC->CFGR2 |= RCC_USBFSDIV_DIV10;

  /* Enable OTG_FS peripheral clock (HB bus) */
  RCC->HBPCENR |= RCC_USBOTGEN;

  /* Enable GPIOA clock (for USB DP/DM on PA11/PA12) */
  RCC->HB2PCENR |= RCC_IOPAEN;
}

/**
 * @brief   Disables the USBFS peripheral clock.
 */
static void usb_lld_clock_disable(void)
{
  /* Disable OTG_FS peripheral clock */
  RCC->HBPCENR &= ~RCC_USBOTGEN;
}

/**
 * @brief   Configures and activates the USB peripheral.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_start(USBDriver *usbp)
{
  if (usbp->state == USB_STOP)
  {
    /* Enables the peripheral.*/
#if CH32_OTG_USE_USB1 == TRUE
    if (&USBD1 == usbp)
    {
      /* Enable clocks */
      usb_lld_clock_enable();

      /* Software reset the SIE */
      CH32_USBFS_HOST->BASE_CTRL = USBFS_UC_RESET_SIE | USBFS_UC_CLR_ALL;

      /* Small delay for reset */
      {
        volatile uint32_t delay = 100000;
        while (delay) delay--;
      }

      CH32_USBFS_HOST->BASE_CTRL = 0;

      /* Disable and configure interrupt */
      NVIC_DisableIRQ(USBFS_IRQn);

      /* Enable interrupts: bus reset, suspend, transfer */
      CH32_USBFS_DEVICE->INT_EN = USBFS_UIE_BUS_RST |
                                  USBFS_UIE_SUSPEND |
                                  USBFS_UIE_TRANSFER;

      /* Configure device mode: pull-up enable, interrupt busy, DMA enable */
      CH32_USBFS_DEVICE->BASE_CTRL = USBFS_UC_DEV_PU_EN |
                                     USBFS_UC_INT_BUSY |
                                     USBFS_UC_DMA_EN;
      /* Enable interrupt */
      NVIC_EnableIRQ(USBFS_IRQn);

      /* Enable USB port */
      CH32_USBFS_DEVICE->UDEV_CTRL = USBFS_UD_PD_DIS |
                                     USBFS_UD_PORT_EN;

    }
#endif
  }
  /* Configures the peripheral.*/
  usb_lld_reset(usbp);
}

/**
 * @brief   Deactivates the USB peripheral.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_stop(USBDriver *usbp)
{
  if (usbp->state == USB_READY)
  {
    /* Disables the peripheral.*/
#if CH32_OTG_USE_USB1 == TRUE
    if (&USBD1 == usbp)
    {
      /* Reset SIE */
      CH32_USBFS_HOST->BASE_CTRL = USBFS_UC_RESET_SIE | USBFS_UC_CLR_ALL;
      {
        volatile uint32_t delay = 100;
        while (delay) delay--;
      }
      CH32_USBFS_DEVICE->BASE_CTRL = 0;

      /* Disable interrupt */
      NVIC_DisableIRQ(USBFS_IRQn);

      /* Disable clock */
      usb_lld_clock_disable();
    }
#endif
  }
}

/**
 * @brief   USB low level reset routine.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_reset(USBDriver *usbp)
{
  /* Post reset initialization.*/
  CH32_USBFS_DEVICE->DEV_ADDR = 0;

  /* EP0 initialization.*/
  usbp->epc[0] = &ep0config;
  usb_lld_init_endpoint(usbp, 0);
}

/**
 * @brief   Sets the USB address.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_set_address(USBDriver *usbp)
{
  CH32_USBFS_DEVICE->DEV_ADDR = (CH32_USBFS_DEVICE->DEV_ADDR & USBFS_UDA_GP_BIT) |
                                (usbp->address & USBFS_USB_ADDR_MASK);
}

/**
 * @brief   Enables an endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_init_endpoint(USBDriver *usbp, usbep_t ep)
{
  (void)usbp;
  uint8_t ep_num = ep & 0x7F;

  if (ep_num == 0)
  {
    /* EP0: configure for control transfers */
    USBOutEndpointState *oesp = usbp->epc[0]->out_state;
    oesp->rxbuf = ep0setup_buffer;
    oesp->rxsize = 8;
    oesp->rxpkts = 1;

    /* Set EP0 DMA address to setup buffer */
    CH32_USBFS_DEVICE->UEP0_DMA = (uint32_t)ep0setup_buffer;

    /* Enable EP0 RX with ACK */
    CH32_USBFS_DEVICE->UEP0_RX_CTRL = USBFS_UEP_R_RES_ACK;
    CH32_USBFS_DEVICE->UEP0_TX_CTRL = USBFS_UEP_T_RES_NAK;
  }
  else if ((ep_num >= 1) && (ep_num <= 3))
  {
    /* EP1-3: configure based on endpoint type */
    const USBEndpointConfig *epcp = usbp->epc[ep_num];

    switch (ep_num)
    {
    case 1:
      if (epcp->in_maxsize > 0)
      {
        CH32_USBFS_DEVICE->UEP4_1_MOD |= USBFS_UEP1_TX_EN;
        CH32_USBFS_DEVICE->UEP1_TX_LEN = 0;
        CH32_USBFS_DEVICE->UEP1_TX_CTRL = USBFS_UEP_T_RES_NAK;
      }
      if (epcp->out_maxsize > 0)
      {
        CH32_USBFS_DEVICE->UEP4_1_MOD |= USBFS_UEP1_RX_EN;
        CH32_USBFS_DEVICE->UEP1_RX_CTRL = USBFS_UEP_R_RES_NAK;
      }
      break;

    case 2:
      if (epcp->in_maxsize > 0)
      {
        CH32_USBFS_DEVICE->UEP2_3_MOD |= USBFS_UEP2_TX_EN;
        CH32_USBFS_DEVICE->UEP2_TX_LEN = 0;
        CH32_USBFS_DEVICE->UEP2_TX_CTRL = USBFS_UEP_T_RES_NAK;
      }
      if (epcp->out_maxsize > 0)
      {
        CH32_USBFS_DEVICE->UEP2_3_MOD |= USBFS_UEP2_RX_EN;
        CH32_USBFS_DEVICE->UEP2_RX_CTRL = USBFS_UEP_R_RES_NAK;
      }
      break;

    case 3:
      if (epcp->in_maxsize > 0)
      {
        CH32_USBFS_DEVICE->UEP2_3_MOD |= USBFS_UEP3_TX_EN;
        CH32_USBFS_DEVICE->UEP3_TX_LEN = 0;
        CH32_USBFS_DEVICE->UEP3_TX_CTRL = USBFS_UEP_T_RES_NAK;
      }
      if (epcp->out_maxsize > 0)
      {
        CH32_USBFS_DEVICE->UEP2_3_MOD |= USBFS_UEP3_RX_EN;
        CH32_USBFS_DEVICE->UEP3_RX_CTRL = USBFS_UEP_R_RES_NAK;
      }
      break;
    }
  }
}

/**
 * @brief   Disables all the active endpoints except the endpoint zero.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_disable_endpoints(USBDriver *usbp)
{
  (void)usbp;

  /* Clear all endpoint mode registers */
  CH32_USBFS_DEVICE->UEP4_1_MOD = 0;
  CH32_USBFS_DEVICE->UEP2_3_MOD = 0;
  CH32_USBFS_DEVICE->UEP5_6_MOD = 0;
  CH32_USBFS_DEVICE->UEP7_MOD = 0;

  /* Set all non-EP0 endpoints to NAK/disabled */
  for (uint8_t ep = 1; ep <= 7; ep++)
  {
    volatile uint8_t *txctlr = NULL;
    volatile uint8_t *rxctlr = NULL;

    switch (ep)
    {
    case 1:
      txctlr = &(CH32_USBFS_DEVICE->UEP1_TX_CTRL);
      rxctlr = &(CH32_USBFS_DEVICE->UEP1_RX_CTRL);
      break;
    case 2:
      txctlr = &(CH32_USBFS_DEVICE->UEP2_TX_CTRL);
      rxctlr = &(CH32_USBFS_DEVICE->UEP2_RX_CTRL);
      break;
    case 3:
      txctlr = &(CH32_USBFS_DEVICE->UEP3_TX_CTRL);
      rxctlr = &(CH32_USBFS_DEVICE->UEP3_RX_CTRL);
      break;
    default:
      continue;
    }

    if (txctlr)
      *txctlr = USBFS_UEP_T_RES_NAK;
    if (rxctlr)
      *rxctlr = USBFS_UEP_R_RES_NAK;
  }
}

/**
 * @brief   Returns the status of an OUT endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 * @return              The endpoint status.
 * @retval EP_STATUS_DISABLED The endpoint is not active.
 * @retval EP_STATUS_STALLED  The endpoint is stalled.
 * @retval EP_STATUS_ACTIVE   The endpoint is active.
 *
 * @notapi
 */
usbepstatus_t usb_lld_get_status_out(USBDriver *usbp, usbep_t ep)
{
  (void)usbp;
  volatile uint8_t *rxctlr_reg = NULL;

  switch (ep & 0x7F)
  {
  case 0:
    rxctlr_reg = &(CH32_USBFS_DEVICE->UEP0_RX_CTRL);
    break;
  case 1:
    rxctlr_reg = &(CH32_USBFS_DEVICE->UEP1_RX_CTRL);
    break;
  case 2:
    rxctlr_reg = &(CH32_USBFS_DEVICE->UEP2_RX_CTRL);
    break;
  case 3:
    rxctlr_reg = &(CH32_USBFS_DEVICE->UEP3_RX_CTRL);
    break;
  default:
    return EP_STATUS_DISABLED;
  }

  if ((*rxctlr_reg & USBFS_UEP_R_RES_MASK) == USBFS_UEP_R_RES_ACK)
    return EP_STATUS_ACTIVE;
  else if ((*rxctlr_reg & USBFS_UEP_R_RES_MASK) == USBFS_UEP_R_RES_STALL)
    return EP_STATUS_STALLED;

  return EP_STATUS_DISABLED;
}

/**
 * @brief   Returns the status of an IN endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 * @return              The endpoint status.
 * @retval EP_STATUS_DISABLED The endpoint is not active.
 * @retval EP_STATUS_STALLED  The endpoint is stalled.
 * @retval EP_STATUS_ACTIVE   The endpoint is active.
 *
 * @notapi
 */
usbepstatus_t usb_lld_get_status_in(USBDriver *usbp, usbep_t ep)
{
  (void)usbp;
  volatile uint8_t *txctlr_reg = NULL;

  switch (ep & 0x7F)
  {
  case 0:
    txctlr_reg = &(CH32_USBFS_DEVICE->UEP0_TX_CTRL);
    break;
  case 1:
    txctlr_reg = &(CH32_USBFS_DEVICE->UEP1_TX_CTRL);
    break;
  case 2:
    txctlr_reg = &(CH32_USBFS_DEVICE->UEP2_TX_CTRL);
    break;
  case 3:
    txctlr_reg = &(CH32_USBFS_DEVICE->UEP3_TX_CTRL);
    break;
  default:
    return EP_STATUS_DISABLED;
  }

  if ((*txctlr_reg & USBFS_UEP_T_RES_MASK) == USBFS_UEP_T_RES_ACK)
    return EP_STATUS_ACTIVE;
  else if ((*txctlr_reg & USBFS_UEP_T_RES_MASK) == USBFS_UEP_T_RES_STALL)
    return EP_STATUS_STALLED;

  return EP_STATUS_DISABLED;
}

/**
 * @brief   Reads a setup packet from the dedicated packet buffer.
 * @details This function must be invoked in the context of the @p setup_cb
 *          callback in order to read the received setup packet.
 * @pre     In order to use this function the endpoint must have been
 *          initialized as a control endpoint.
 * @post    The endpoint is ready to accept another packet.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 * @param[out] buf      buffer where to copy the packet data
 *
 * @notapi
 */
void usb_lld_read_setup(USBDriver *usbp, usbep_t ep, uint8_t *buf)
{
  (void)usbp;
  (void)ep;

  for (size_t i = 0; i < 8; i++)
  {
    buf[i] = ep0setup_buffer[i];
  }
}

/**
 * @brief   Prepares for a receive operation.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_prepare_receive(USBDriver *usbp, usbep_t ep)
{
  (void)usbp;
  (void)ep;
}

/**
 * @brief   Prepares for a transmit operation.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_prepare_transmit(USBDriver *usbp, usbep_t ep)
{
  (void)usbp;
  (void)ep;
}

/**
 * @brief   Starts a receive operation on an OUT endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_start_out(USBDriver *usbp, usbep_t ep)
{
  uint8_t ep_num = ep & 0x7F;
  volatile uint32_t *dma_reg = NULL;
  volatile uint8_t *rxctlr_reg = NULL;
  switch (ep_num)
  {
  case 0:
    dma_reg    = &(CH32_USBFS_DEVICE->UEP0_DMA);
    rxctlr_reg = &(CH32_USBFS_DEVICE->UEP0_RX_CTRL);
    break;
  case 1:
    dma_reg    = &(CH32_USBFS_DEVICE->UEP1_DMA);
    rxctlr_reg = &(CH32_USBFS_DEVICE->UEP1_RX_CTRL);
    break;
  case 2:
    dma_reg    = &(CH32_USBFS_DEVICE->UEP2_DMA);
    rxctlr_reg = &(CH32_USBFS_DEVICE->UEP2_RX_CTRL);
    break;
  case 3:
    dma_reg    = &(CH32_USBFS_DEVICE->UEP3_DMA);
    rxctlr_reg = &(CH32_USBFS_DEVICE->UEP3_RX_CTRL);
    break;
  default:
    return;
  }

  USBOutEndpointState *osp = usbp->epc[ep_num]->out_state;

  /* Handle EP0 specific states */
  if (ep_num == 0)
  {
    if (usbp->ep0state == USB_EP0_STP_WAITING)
    {
      *rxctlr_reg &= ~USBFS_UEP_R_TOG;
    }
    else if (usbp->ep0state == USB_EP0_OUT_WAITING_STS)
    {
      *rxctlr_reg |= USBFS_UEP_R_TOG;
    }
  }

  /* Calculate number of packets */
  if (osp->rxsize == 0)
  {
    osp->rxpkts = 1;
  }
  else
  {
    osp->rxpkts = (uint16_t)((osp->rxsize +
                     usbp->epc[ep_num]->out_maxsize - 1) /
                     usbp->epc[ep_num]->out_maxsize);
  }

  /* Set DMA address and length */
  *dma_reg = (uint32_t)osp->rxbuf;

  /* Enable RX endpoint */
  *rxctlr_reg = (*rxctlr_reg & ~USBFS_UEP_R_RES_MASK) | USBFS_UEP_R_RES_ACK;
}

/**
 * @brief   Starts a transmit operation on an IN endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_start_in(USBDriver *usbp, usbep_t ep)
{
  uint8_t ep_num = ep & 0x7F;
  volatile uint32_t *dma_reg = NULL;
  volatile uint8_t *txctlr_reg = NULL;
  volatile uint16_t *txlen_reg = NULL;

  switch (ep_num)
  {
  case 0:
    dma_reg    = &(CH32_USBFS_DEVICE->UEP0_DMA);
    txctlr_reg = &(CH32_USBFS_DEVICE->UEP0_TX_CTRL);
    txlen_reg  = (volatile uint16_t *)&(CH32_USBFS_DEVICE->UEP0_TX_LEN);
    break;
  case 1:
    dma_reg    = &(CH32_USBFS_DEVICE->UEP1_DMA);
    txctlr_reg = &(CH32_USBFS_DEVICE->UEP1_TX_CTRL);
    txlen_reg  = (volatile uint16_t *)&(CH32_USBFS_DEVICE->UEP1_TX_LEN);
    break;
  case 2:
    dma_reg    = &(CH32_USBFS_DEVICE->UEP2_DMA);
    txctlr_reg = &(CH32_USBFS_DEVICE->UEP2_TX_CTRL);
    txlen_reg  = (volatile uint16_t *)&(CH32_USBFS_DEVICE->UEP2_TX_LEN);
    break;
  case 3:
    dma_reg    = &(CH32_USBFS_DEVICE->UEP3_DMA);
    txctlr_reg = &(CH32_USBFS_DEVICE->UEP3_TX_CTRL);
    txlen_reg  = (volatile uint16_t *)&(CH32_USBFS_DEVICE->UEP3_TX_LEN);
    break;
  default:
    return;
  }

  USBInEndpointState *isp = usbp->epc[ep_num]->in_state;

  /* Calculate first packet size */
  isp->txlast = (isp->txsize < usbp->epc[ep_num]->in_maxsize) ?
                 isp->txsize : usbp->epc[ep_num]->in_maxsize;

  /* Handle EP0 specific states */
  if (ep_num == 0)
  {
    if (usbp->ep0state == USB_EP0_IN_SENDING_STS)
    {
      *txctlr_reg |= USBFS_UEP_T_TOG;
    }
  }

  /* Set DMA address, length, and enable transmission */
  *dma_reg = (uint32_t)(isp->txbuf + isp->txcnt);
  *txlen_reg = (uint16_t)isp->txlast;
  *txctlr_reg = (*txctlr_reg & ~USBFS_UEP_T_RES_MASK) | USBFS_UEP_T_RES_ACK;
}

/**
 * @brief   Brings an OUT endpoint in the stalled state.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_stall_out(USBDriver *usbp, usbep_t ep)
{
  (void)usbp;
  volatile uint8_t *rxctlr_reg = NULL;

  switch (ep & 0x7F)
  {
  case 0:
    rxctlr_reg = &(CH32_USBFS_DEVICE->UEP0_RX_CTRL);
    break;
  case 1:
    rxctlr_reg = &(CH32_USBFS_DEVICE->UEP1_RX_CTRL);
    break;
  case 2:
    rxctlr_reg = &(CH32_USBFS_DEVICE->UEP2_RX_CTRL);
    break;
  case 3:
    rxctlr_reg = &(CH32_USBFS_DEVICE->UEP3_RX_CTRL);
    break;
  default:
    return;
  }

  *rxctlr_reg = (*rxctlr_reg & ~USBFS_UEP_R_RES_MASK) | USBFS_UEP_R_RES_STALL;
}

/**
 * @brief   Brings an IN endpoint in the stalled state.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_stall_in(USBDriver *usbp, usbep_t ep)
{
  (void)usbp;
  volatile uint8_t *txctlr_reg = NULL;

  switch (ep & 0x7F)
  {
  case 0:
    txctlr_reg = &(CH32_USBFS_DEVICE->UEP0_TX_CTRL);
    break;
  case 1:
    txctlr_reg = &(CH32_USBFS_DEVICE->UEP1_TX_CTRL);
    break;
  case 2:
    txctlr_reg = &(CH32_USBFS_DEVICE->UEP2_TX_CTRL);
    break;
  case 3:
    txctlr_reg = &(CH32_USBFS_DEVICE->UEP3_TX_CTRL);
    break;
  default:
    return;
  }

  *txctlr_reg = (*txctlr_reg & ~USBFS_UEP_T_RES_MASK) | USBFS_UEP_T_RES_STALL;
}

/**
 * @brief   Brings an OUT endpoint in the active state.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_clear_out(USBDriver *usbp, usbep_t ep)
{
  (void)usbp;
  volatile uint8_t *rxctlr_reg = NULL;

  switch (ep & 0x7F)
  {
  case 0:
    rxctlr_reg = &(CH32_USBFS_DEVICE->UEP0_RX_CTRL);
    break;
  case 1:
    rxctlr_reg = &(CH32_USBFS_DEVICE->UEP1_RX_CTRL);
    break;
  case 2:
    rxctlr_reg = &(CH32_USBFS_DEVICE->UEP2_RX_CTRL);
    break;
  case 3:
    rxctlr_reg = &(CH32_USBFS_DEVICE->UEP3_RX_CTRL);
    break;
  default:
    return;
  }

  /* Clear the response bits (remove ACK/STALL/NAK) */
  *rxctlr_reg &= ~USBFS_UEP_R_RES_MASK;
}

/**
 * @brief   Brings an IN endpoint in the active state.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_clear_in(USBDriver *usbp, usbep_t ep)
{
  (void)usbp;
  volatile uint8_t *txctlr_reg = NULL;

  switch (ep & 0x7F)
  {
  case 0:
    txctlr_reg = &(CH32_USBFS_DEVICE->UEP0_TX_CTRL);
    break;
  case 1:
    txctlr_reg = &(CH32_USBFS_DEVICE->UEP1_TX_CTRL);
    break;
  case 2:
    txctlr_reg = &(CH32_USBFS_DEVICE->UEP2_TX_CTRL);
    break;
  case 3:
    txctlr_reg = &(CH32_USBFS_DEVICE->UEP3_TX_CTRL);
    break;
  default:
    return;
  }

  /* Clear the response bits (remove ACK/STALL/NAK) */
  *txctlr_reg &= ~USBFS_UEP_T_RES_MASK;
}

#endif /* HAL_USE_USB == TRUE */

/** @} */
