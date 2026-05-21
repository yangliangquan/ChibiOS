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
 * @brief   CH32 USB subsystem low level driver source.
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
#if (CH32_USB_USE_USB1 == TRUE) || defined(__DOXYGEN__)
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
  DEF_USB_EP0_HS_SIZE,
  DEF_USB_EP0_HS_SIZE,
  &ep0_state.in,
  &ep0_state.out
};

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/
__attribute__((unused)) static uint32_t maxPacketSizeList[] = {DEF_USB_EP0_HS_SIZE, DEF_USB_EP1_HS_SIZE, DEF_USB_EP2_HS_SIZE,
                                       DEF_USB_EP3_HS_SIZE, DEF_USB_EP4_HS_SIZE, DEF_USB_EP5_HS_SIZE,
                                       DEF_USB_EP6_HS_SIZE, DEF_USB_EP7_HS_SIZE};
/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
void usb_serve_endpoints(USBDriver *usbp, uint8_t intst)
{
    (void)usbp;
    uint8_t ep = intst & USBHS_UDIS_EP_ID_MASK;
    uint8_t dir = intst & USBHS_UDIS_EP_DIR;
    volatile uint16_t *rxlen_reg = NULL;
    volatile uint8_t *rxctlr_reg = NULL;
    volatile uint16_t *txlen_reg = NULL;
    volatile uint8_t *txctlr_reg = NULL;
    volatile uint32_t *rxaddr_reg = NULL;
    volatile uint32_t *txaddr_reg = NULL;
    switch (ep & 0x7f)
    {
    case 0:
        rxaddr_reg = &(CH32_USB_DEVICE->UEP0_DMA);
        txaddr_reg = &(CH32_USB_DEVICE->UEP0_DMA);
        txlen_reg = &(CH32_USB_DEVICE->UEP0_TX_LEN);
        txctlr_reg = &(CH32_USB_DEVICE->UEP0_TX_CTRL);
        rxlen_reg = &(CH32_USB_DEVICE->UEP0_RX_LEN);
        rxctlr_reg = &(CH32_USB_DEVICE->UEP0_RX_CTRL);
        break;
    case 1:
        rxaddr_reg = &(CH32_USB_DEVICE->UEP1_RX_DMA);
        txaddr_reg = &(CH32_USB_DEVICE->UEP1_TX_DMA);
        txlen_reg = &(CH32_USB_DEVICE->UEP1_TX_LEN);
        txctlr_reg = &(CH32_USB_DEVICE->UEP1_TX_CTRL);
        rxlen_reg = &(CH32_USB_DEVICE->UEP1_RX_LEN);
        rxctlr_reg = &(CH32_USB_DEVICE->UEP1_RX_CTRL);
        break;
    case 2:
        rxaddr_reg = &(CH32_USB_DEVICE->UEP2_RX_DMA);
        txaddr_reg = &(CH32_USB_DEVICE->UEP2_TX_DMA);
        txlen_reg = &(CH32_USB_DEVICE->UEP2_TX_LEN);
        txctlr_reg = &(CH32_USB_DEVICE->UEP2_TX_CTRL);
        rxlen_reg = &(CH32_USB_DEVICE->UEP2_RX_LEN);
        rxctlr_reg = &(CH32_USB_DEVICE->UEP2_RX_CTRL);
        break;
    case 3:
        rxaddr_reg = &(CH32_USB_DEVICE->UEP3_RX_DMA);
        txaddr_reg = &(CH32_USB_DEVICE->UEP3_TX_DMA);
        txlen_reg = &(CH32_USB_DEVICE->UEP3_TX_LEN);
        txctlr_reg = &(CH32_USB_DEVICE->UEP3_TX_CTRL);
        rxlen_reg = &(CH32_USB_DEVICE->UEP3_RX_LEN);
        rxctlr_reg = &(CH32_USB_DEVICE->UEP3_RX_CTRL);
        break;
    case 4:
        rxaddr_reg = &(CH32_USB_DEVICE->UEP4_RX_DMA);
        txaddr_reg = &(CH32_USB_DEVICE->UEP4_TX_DMA);
        txlen_reg = &(CH32_USB_DEVICE->UEP4_TX_LEN);
        txctlr_reg = &(CH32_USB_DEVICE->UEP4_TX_CTRL);
        rxlen_reg = &(CH32_USB_DEVICE->UEP4_RX_LEN);
        rxctlr_reg = &(CH32_USB_DEVICE->UEP4_RX_CTRL);
        break;
    case 5:
        rxaddr_reg = &(CH32_USB_DEVICE->UEP5_RX_DMA);
        txaddr_reg = &(CH32_USB_DEVICE->UEP5_TX_DMA);
        txlen_reg = &(CH32_USB_DEVICE->UEP5_TX_LEN);
        txctlr_reg = &(CH32_USB_DEVICE->UEP5_TX_CTRL);
        rxlen_reg = &(CH32_USB_DEVICE->UEP5_RX_LEN);
        rxctlr_reg = &(CH32_USB_DEVICE->UEP5_RX_CTRL);
        break;
    case 6:
        rxaddr_reg = &(CH32_USB_DEVICE->UEP6_RX_DMA);
        txaddr_reg = &(CH32_USB_DEVICE->UEP6_TX_DMA);
        txlen_reg = &(CH32_USB_DEVICE->UEP6_TX_LEN);
        txctlr_reg = &(CH32_USB_DEVICE->UEP6_TX_CTRL);
        rxlen_reg = &(CH32_USB_DEVICE->UEP6_RX_LEN);
        rxctlr_reg = &(CH32_USB_DEVICE->UEP6_RX_CTRL);
        break;
    case 7:
        rxaddr_reg = &(CH32_USB_DEVICE->UEP7_RX_DMA);
        txaddr_reg = &(CH32_USB_DEVICE->UEP7_TX_DMA);
        txlen_reg = &(CH32_USB_DEVICE->UEP7_TX_LEN);
        txctlr_reg = &(CH32_USB_DEVICE->UEP7_TX_CTRL);
        rxlen_reg = &(CH32_USB_DEVICE->UEP7_RX_LEN);
        rxctlr_reg = &(CH32_USB_DEVICE->UEP7_RX_CTRL);
        break;
    default:
        return;
        break;
    }
    USBOutEndpointState *oesp;
    USBInEndpointState *iesp;
    oesp = usbp->epc[ep]->out_state;
    iesp = usbp->epc[ep]->in_state;

    if (!dir)
    {
        usb_lld_clear_out(usbp, ep);

        oesp->rxcnt += *rxlen_reg;
        oesp->rxsize -= *rxlen_reg;
        oesp->rxpkts--;
        if ((*rxctlr_reg) & USBHS_UEP_R_SETUP_IS)
        {
            *txctlr_reg |= USBHS_UEP_T_TOG_DATA1;
            *rxctlr_reg |= USBHS_UEP_R_TOG_DATA1;
            _usb_isr_invoke_setup_cb(usbp, ep);
        }
        else
        {
            if ((oesp->rxpkts == 0) || (*rxlen_reg < usbp->epc[ep]->out_maxsize))
            {
                _usb_isr_invoke_out_cb(usbp, ep);
                if (ep == 0)
                {
                  if(usbp->ep0state == USB_EP0_STP_WAITING){
                    oesp->rxpkts = 1;
                    oesp->rxbuf = ep0setup_buffer;
                    oesp->rxsize = 8;
                    *rxaddr_reg = (uint32_t)oesp->rxbuf;
                    *rxlen_reg = oesp->rxsize;
                    *rxctlr_reg &= ~USBHS_UEP_R_TOG_DATA1;
                    *rxctlr_reg = (*rxctlr_reg & ~USBHS_UEP_R_RES_MASK) | USBHS_UEP_R_RES_ACK;
                  }
                }
            }
            else
            {
                oesp->rxbuf = ep0setup_buffer;
                *rxctlr_reg ^= USBHS_UEP_R_TOG_DATA1;
                *rxaddr_reg = (uint32_t)oesp->rxbuf;
                *rxlen_reg = oesp->rxsize < usbp->epc[ep]->out_maxsize ? oesp->rxsize : usbp->epc[ep]->out_maxsize;
                *rxctlr_reg = (*rxctlr_reg & ~USBHS_UEP_R_RES_MASK) | USBHS_UEP_R_RES_ACK;
            }
        }
    }
    else
    {
        usb_lld_clear_in(usbp, ep);
        iesp->txcnt += iesp->txlast;
        if (iesp->txsize - iesp->txcnt == 0)
        {
            _usb_isr_invoke_in_cb(usbp, ep);
            if (ep == 0)
            {
                if(usbp->ep0state == USB_EP0_STP_WAITING){
                  oesp->rxpkts = 1;
                  oesp->rxbuf = ep0setup_buffer;
                  oesp->rxsize = 8;
                  *rxaddr_reg = (uint32_t)oesp->rxbuf;
                  *rxlen_reg = oesp->rxsize;
                  *rxctlr_reg &= ~USBHS_UEP_R_TOG_DATA1;
                  *rxctlr_reg = (*rxctlr_reg & ~USBHS_UEP_R_RES_MASK) | USBHS_UEP_R_RES_ACK;
                }
            }
        }
        else
        {
            iesp->txlast = (iesp->txsize - iesp->txcnt) < usbp->epc[(ep & 0x7f)]->in_maxsize ? (iesp->txsize - iesp->txcnt) : usbp->epc[(ep & 0x7f)]->in_maxsize;
            *txctlr_reg ^= USBHS_UEP_T_TOG_DATA1;
            *txaddr_reg = (uint32_t)iesp->txbuf + iesp->txcnt;
            *txlen_reg = iesp->txlast;
            *txctlr_reg = (*txctlr_reg & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_ACK;
        }
    }
}
/*===========================================================================*/
/* Driver interrupt handlers and threads.                                    */
/*===========================================================================*/
OSAL_IRQ_HANDLER(USBHS_IRQHandler)
{
  
    uint8_t intflag = CH32_USB_DEVICE->INT_FG;
    uint8_t intst = CH32_USB_DEVICE->INT_ST;

    if (intflag & USBHS_UDIF_BUS_RST)
    {
        CH32_USB_DEVICE->INT_FG = USBHS_UDIF_BUS_RST;
        _usb_reset(&USBD1);
    }
    else if (intflag & USBHS_UDIF_SUSPEND)
    {
        // printf("SU %x\n",CH32_USB_DEVICE->INT_FG);
        CH32_USB_DEVICE->INT_FG = USBHS_UDIF_SUSPEND;

        _usb_suspend(&USBD1);
    }
    else if (intflag & USBHS_UDIF_TRANSFER)
    {
        // Handle transfer interrupt
        usb_serve_endpoints(&USBD1, intst);
    }
    else if (intflag & USBHS_UDIF_RX_SOF)
    {
        // Handle SOF interrupt
        CH32_USB_DEVICE->INT_FG = USBHS_UDIF_RX_SOF;
        _usb_isr_invoke_sof_cb(&USBD1);
    }else{
        CH32_USB_DEVICE->INT_FG = intflag;
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
void usb_lld_init(void) {

#if CH32_USB_USE_USB1 == TRUE
  /* Driver initialization.*/
  usbObjectInit(&USBD1);
#endif
}

static void init_usb_clock(bool sta)
{
    if (sta == true)
    {
        
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
        RCC->CFGR0 |= RCC_UTMION;
        enableHB(RCC_USBHSEN);
    }
    else
    {
        disableHB(RCC_USBHSEN);
        RCC->CFGR0 &= ~RCC_UTMION;
        if ((RCC->PLLCFGR & RCC_SYSPLL_SEL) != RCC_SYSPLL_USBHS)
        {
            RCC->CTLR &= ~RCC_USBHS_PLLON;
        }
    }
}
/**
 * @brief   Configures and activates the USB peripheral.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_start(USBDriver *usbp) {

  if (usbp->state == USB_STOP) {
    /* Enables the peripheral.*/
#if CH32_USB_USE_USB1 == TRUE
    if (&USBD1 == usbp)
    {
        enableHB2(RCC_AFIOEN);
        enableHB2(RCC_IOPBEN);
        AFIO->PCFR1 = (AFIO->PCFR1 & ~AFIO_PCFR1_SWJ_CFG) | AFIO_PCFR1_SWJ_CFG_2;
        resetHB(RCC_USBHSEN);
        init_usb_clock(true);
        NVIC_DisableIRQ(USBHS_IRQn);
        CH32_USB_DEVICE->CONTROL = USBHS_UD_RST_LINK | USBHS_UD_PHY_SUSPENDM;
        CH32_USB_DEVICE->INT_EN = USBHS_UDIE_BUS_RST | USBHS_UDIE_SUSPEND | USBHS_UDIE_BUS_SLEEP | USBHS_UDIE_LPM_ACT |
                                  USBHS_UDIE_TRANSFER | USBHS_UDIE_LINK_RDY | USBHS_UDIE_SOF_ACT;
        CH32_USB_DEVICE->BASE_MODE = USBHS_UD_SPEED_HIGH;
        CH32_USB_DEVICE->CONTROL = USBHS_UD_DEV_EN | USBHS_UD_DMA_EN | USBHS_UD_LPM_EN | USBHS_UD_PHY_SUSPENDM;
        NVIC_EnableIRQ(USBHS_IRQn);
        usb_lld_reset(usbp);
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
void usb_lld_stop(USBDriver *usbp) {

  if (usbp->state == USB_READY) {
    /* Resets the peripheral.*/

    /* Disables the peripheral.*/
#if CH32_USB_USE_USB1 == TRUE
    if (&USBD1 == usbp) {
      init_usb_clock(false);
      NVIC_DisableIRQ(USBHS_IRQn);
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
void usb_lld_reset(USBDriver *usbp) {

  /* Post reset initialization.*/
  CH32_USB_DEVICE->CONTROL |= USBHS_UD_RST_SIE;
  CH32_USB_DEVICE->CONTROL &= ~(USBHS_UD_RST_SIE);
  CH32_USB_DEVICE->DEV_AD = 0;
  CH32_USB_DEVICE->UEP_TX_TOG_AUTO = 0xfe;
  CH32_USB_DEVICE->UEP_RX_TOG_AUTO = 0xfe;
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
void usb_lld_set_address(USBDriver *usbp) {

  (void)usbp;
  CH32_USB_DEVICE->DEV_AD = (usbp->address & USBHS_UD_DEV_ADDR);
}

/**
 * @brief   Enables an endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_init_endpoint(USBDriver *usbp, usbep_t ep) {
  __attribute__((unused)) const USBEndpointConfig *epcp = usbp->epc[ep];
  __attribute__((unused)) uint32_t *dma_addr_reg;
  __attribute__((unused)) uint32_t *maxlen_reg;
  __attribute__((unused)) uint16_t *rxlen_reg;
  if(ep == 0){
    dma_addr_reg = (uint32_t *)&(CH32_USB_DEVICE->UEP0_DMA);
    maxlen_reg = (uint32_t *)&(CH32_USB_DEVICE->UEP0_MAX_LEN);
    rxlen_reg = (uint16_t *)&(CH32_USB_DEVICE->UEP0_RX_LEN);
  }else{
    dma_addr_reg = (uint32_t *)(&(CH32_USB_DEVICE->UEP1_RX_DMA) + 4 * ((ep & 0x7f) - 1));
    maxlen_reg = (uint32_t *)(&(CH32_USB_DEVICE->UEP1_MAX_LEN) + 4 * ((ep & 0x7f) - 1));
    rxlen_reg = (uint16_t *)&(CH32_USB_DEVICE->UEP1_RX_LEN) + 4 * ((ep & 0x7f) - 1);
  }

  CH32_USB_DEVICE->UEP_TX_EN |= USBHS_UEP0_T_EN << (ep & 0x7F);
  CH32_USB_DEVICE->UEP_RX_EN |= USBHS_UEP0_R_EN << (ep & 0x7F);
  if((ep & 0x7f) == 0){
    usbp->epc[0]->out_state->rxbuf = ep0setup_buffer;
    usbp->epc[0]->out_state->rxsize = 8;
    usb_lld_start_out(usbp,ep);
    // usb_lld_start_out(usbp,ep);
  }
}

/**
 * @brief   Disables all the active endpoints except the endpoint zero.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_disable_endpoints(USBDriver *usbp) {

  (void)usbp;
    CH32_USB_DEVICE->UEP_TX_EN &= ~(0xffffu);
    CH32_USB_DEVICE->UEP_RX_EN &= ~(0xffffu);
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
usbepstatus_t usb_lld_get_status_out(USBDriver *usbp, usbep_t ep) {

  (void)usbp;
  (void)ep;
  uint8_t volatile *rxctlr_reg;

  switch((ep & 0x7f)){
    case 0:
      rxctlr_reg = &(CH32_USB_DEVICE->UEP0_RX_CTRL);
      break;
    case 1:
      rxctlr_reg = &(CH32_USB_DEVICE->UEP1_RX_CTRL);
      break;
    case 2:
      rxctlr_reg = &(CH32_USB_DEVICE->UEP2_RX_CTRL);
      break;
    case 3:
      rxctlr_reg = &(CH32_USB_DEVICE->UEP3_RX_CTRL);
      break;
    case 4:
      rxctlr_reg = &(CH32_USB_DEVICE->UEP4_RX_CTRL);
      break;
    case 5:
      rxctlr_reg = &(CH32_USB_DEVICE->UEP5_RX_CTRL);
      break;
    case 6:
      rxctlr_reg = &(CH32_USB_DEVICE->UEP6_RX_CTRL);
      break;
    case 7:
      rxctlr_reg = &(CH32_USB_DEVICE->UEP7_RX_CTRL);
      break;
    default:
      return EP_STATUS_DISABLED;
  }
  if((*rxctlr_reg & USBHS_UEP_R_RES_MASK) == USBHS_UEP_R_RES_ACK)
    return EP_STATUS_ACTIVE;
  else if ((*rxctlr_reg & USBHS_UEP_R_RES_MASK) == USBHS_UEP_R_RES_STALL)
  {
    return EP_STATUS_STALLED;
  }
  
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
usbepstatus_t usb_lld_get_status_in(USBDriver *usbp, usbep_t ep) {

  (void)usbp;
  (void)ep;
  uint8_t volatile *txctlr_reg;
  switch((ep & 0x7f)){ 
    case 0:
      txctlr_reg = &(CH32_USB_DEVICE->UEP0_TX_CTRL);
      break;
    case 1:
      txctlr_reg = &(CH32_USB_DEVICE->UEP1_TX_CTRL);
      break;
    case 2:
      txctlr_reg = &(CH32_USB_DEVICE->UEP2_TX_CTRL);
      break;
    case 3:
      txctlr_reg = &(CH32_USB_DEVICE->UEP3_TX_CTRL);
      break;
    case 4:
      txctlr_reg = &(CH32_USB_DEVICE->UEP4_TX_CTRL);
      break;
    case 5:
      txctlr_reg = &(CH32_USB_DEVICE->UEP5_TX_CTRL);
      break;
    case 6:
      txctlr_reg = &(CH32_USB_DEVICE->UEP6_TX_CTRL);
      break;
    case 7:
      txctlr_reg = &(CH32_USB_DEVICE->UEP7_TX_CTRL);
      break;
    default:
      return EP_STATUS_DISABLED;
  }

  if((*txctlr_reg & USBHS_UEP_R_RES_MASK) == USBHS_UEP_R_RES_ACK)
    return EP_STATUS_ACTIVE;
  else if ((*txctlr_reg & USBHS_UEP_R_RES_MASK) == USBHS_UEP_R_RES_STALL)
  {
    return EP_STATUS_STALLED;
  }
  
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
void usb_lld_read_setup(USBDriver *usbp, usbep_t ep, uint8_t *buf) {

  (void)usbp;
  (void)ep;
  (void)buf;

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
void usb_lld_prepare_receive(USBDriver *usbp, usbep_t ep) {
  // usb_lld_start_out(usbp, ep);
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
void usb_lld_prepare_transmit(USBDriver *usbp, usbep_t ep) {
  // usb_lld_start_in(usbp, ep);
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
void usb_lld_start_out(USBDriver *usbp, usbep_t ep) {

  (void)usbp;
  (void)ep;
  __attribute__((unused)) volatile uint16_t *rxlen_reg=NULL;
  __attribute__((unused)) volatile uint8_t *txctlr_reg=NULL;
  __attribute__((unused)) volatile uint8_t *rxctlr_reg=NULL;
  __attribute__((unused)) volatile uint32_t *rxaddr_reg=NULL;
  

  switch (ep & 0x7f)
  {
  case 0:    rxlen_reg = &(CH32_USB_DEVICE->UEP0_RX_LEN);   txctlr_reg = &(CH32_USB_DEVICE->UEP0_TX_CTRL  ); rxctlr_reg = &(CH32_USB_DEVICE->UEP0_RX_CTRL) ;rxaddr_reg = &(CH32_USB_DEVICE->UEP0_DMA);    break;  
  case 1:    rxlen_reg = &(CH32_USB_DEVICE->UEP1_RX_LEN);   txctlr_reg = &(CH32_USB_DEVICE->UEP1_TX_CTRL  ); rxctlr_reg = &(CH32_USB_DEVICE->UEP1_RX_CTRL) ;rxaddr_reg = &(CH32_USB_DEVICE->UEP1_RX_DMA);    break;  
  case 2:    rxlen_reg = &(CH32_USB_DEVICE->UEP2_RX_LEN);   txctlr_reg = &(CH32_USB_DEVICE->UEP2_TX_CTRL  ); rxctlr_reg = &(CH32_USB_DEVICE->UEP2_RX_CTRL) ;rxaddr_reg = &(CH32_USB_DEVICE->UEP2_RX_DMA);    break;  
  case 3:    rxlen_reg = &(CH32_USB_DEVICE->UEP3_RX_LEN);   txctlr_reg = &(CH32_USB_DEVICE->UEP3_TX_CTRL  ); rxctlr_reg = &(CH32_USB_DEVICE->UEP3_RX_CTRL) ;rxaddr_reg = &(CH32_USB_DEVICE->UEP3_RX_DMA);    break;  
  case 4:    rxlen_reg = &(CH32_USB_DEVICE->UEP4_RX_LEN);   txctlr_reg = &(CH32_USB_DEVICE->UEP4_TX_CTRL  ); rxctlr_reg = &(CH32_USB_DEVICE->UEP4_RX_CTRL) ;rxaddr_reg = &(CH32_USB_DEVICE->UEP4_RX_DMA);    break;  
  case 5:    rxlen_reg = &(CH32_USB_DEVICE->UEP5_RX_LEN);   txctlr_reg = &(CH32_USB_DEVICE->UEP5_TX_CTRL  ); rxctlr_reg = &(CH32_USB_DEVICE->UEP5_RX_CTRL) ;rxaddr_reg = &(CH32_USB_DEVICE->UEP5_RX_DMA);    break;  
  case 6:    rxlen_reg = &(CH32_USB_DEVICE->UEP6_RX_LEN);   txctlr_reg = &(CH32_USB_DEVICE->UEP6_TX_CTRL  ); rxctlr_reg = &(CH32_USB_DEVICE->UEP6_RX_CTRL) ;rxaddr_reg = &(CH32_USB_DEVICE->UEP6_RX_DMA);    break;  
  case 7:    rxlen_reg = &(CH32_USB_DEVICE->UEP7_RX_LEN);   txctlr_reg = &(CH32_USB_DEVICE->UEP7_TX_CTRL  ); rxctlr_reg = &(CH32_USB_DEVICE->UEP7_RX_CTRL) ;rxaddr_reg = &(CH32_USB_DEVICE->UEP7_RX_DMA);    break;  
  default:
  return;
    break;
  }
  // *txctlr_reg = (*txctlr_reg & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_NAK;
  // *rxctlr_reg = (*rxctlr_reg & ~USBHS_UEP_R_RES_MASK) | USBHS_UEP_R_RES_NAK;
  USBOutEndpointState *osp = usbp->epc[(ep & 0x7f)]->out_state;
  if(ep == 0){
    if(usbp->ep0state == USB_EP0_STP_WAITING)
    {
        *rxctlr_reg &= ~USBHS_UEP_R_TOG_DATA1;
    }else if(usbp->ep0state == USB_EP0_OUT_WAITING_STS){
        *rxctlr_reg |= USBHS_UEP_R_TOG_DATA1;
    }
  }
  if(osp->rxsize == 0){
    osp->rxpkts = 1;
  }else{
    osp->rxpkts = (uint16_t)((osp->rxsize + usbp->epc[(ep & 0x7f)]->out_maxsize - 1) / usbp->epc[(ep & 0x7f)]->out_maxsize);
  }
  *rxaddr_reg = (uint32_t)osp->rxbuf;
  *rxlen_reg = osp -> rxsize < usbp->epc[(ep & 0x7f)]->out_maxsize ? osp -> rxsize : usbp->epc[(ep & 0x7f)]->out_maxsize;
  *rxctlr_reg = (*rxctlr_reg & USBHS_UEP_R_RES_MASK) | USBHS_UEP_R_RES_ACK;
}

/**
 * @brief   Starts a transmit operation on an IN endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_start_in(USBDriver *usbp, usbep_t ep) {

  (void)usbp;
  (void)ep;
  __attribute__((unused)) volatile uint16_t *txlen_reg=NULL;
  __attribute__((unused)) volatile uint8_t *txctlr_reg=NULL;
  __attribute__((unused)) volatile uint8_t *rxctlr_reg=NULL;
  __attribute__((unused)) volatile uint32_t *txaddr_reg=NULL;

  switch (ep & 0x7f)
  {
  case 0:    txlen_reg = &(CH32_USB_DEVICE->UEP0_TX_LEN);    txctlr_reg = &(CH32_USB_DEVICE->UEP0_TX_CTRL  );rxctlr_reg = &(CH32_USB_DEVICE->UEP0_RX_CTRL) ; txaddr_reg = &(CH32_USB_DEVICE->UEP0_DMA);    break;  
  case 1:    txlen_reg = &(CH32_USB_DEVICE->UEP1_TX_LEN);    txctlr_reg = &(CH32_USB_DEVICE->UEP1_TX_CTRL  );rxctlr_reg = &(CH32_USB_DEVICE->UEP1_RX_CTRL) ; txaddr_reg = &(CH32_USB_DEVICE->UEP1_TX_DMA);   break;  
  case 2:    txlen_reg = &(CH32_USB_DEVICE->UEP2_TX_LEN);    txctlr_reg = &(CH32_USB_DEVICE->UEP2_TX_CTRL  );rxctlr_reg = &(CH32_USB_DEVICE->UEP2_RX_CTRL) ; txaddr_reg = &(CH32_USB_DEVICE->UEP2_TX_DMA);   break;  
  case 3:    txlen_reg = &(CH32_USB_DEVICE->UEP3_TX_LEN);    txctlr_reg = &(CH32_USB_DEVICE->UEP3_TX_CTRL  );rxctlr_reg = &(CH32_USB_DEVICE->UEP3_RX_CTRL) ; txaddr_reg = &(CH32_USB_DEVICE->UEP3_TX_DMA);   break;  
  case 4:    txlen_reg = &(CH32_USB_DEVICE->UEP4_TX_LEN);    txctlr_reg = &(CH32_USB_DEVICE->UEP4_TX_CTRL  );rxctlr_reg = &(CH32_USB_DEVICE->UEP4_RX_CTRL) ; txaddr_reg = &(CH32_USB_DEVICE->UEP4_TX_DMA);   break;  
  case 5:    txlen_reg = &(CH32_USB_DEVICE->UEP5_TX_LEN);    txctlr_reg = &(CH32_USB_DEVICE->UEP5_TX_CTRL  );rxctlr_reg = &(CH32_USB_DEVICE->UEP5_RX_CTRL) ; txaddr_reg = &(CH32_USB_DEVICE->UEP5_TX_DMA);   break;  
  case 6:    txlen_reg = &(CH32_USB_DEVICE->UEP6_TX_LEN);    txctlr_reg = &(CH32_USB_DEVICE->UEP6_TX_CTRL  );rxctlr_reg = &(CH32_USB_DEVICE->UEP6_RX_CTRL) ; txaddr_reg = &(CH32_USB_DEVICE->UEP6_TX_DMA);   break;  
  case 7:    txlen_reg = &(CH32_USB_DEVICE->UEP7_TX_LEN);    txctlr_reg = &(CH32_USB_DEVICE->UEP7_TX_CTRL  );rxctlr_reg = &(CH32_USB_DEVICE->UEP7_RX_CTRL) ; txaddr_reg = &(CH32_USB_DEVICE->UEP7_TX_DMA);   break;  
  default:
  return;
    break;
  }
  // *txctlr_reg = (*txctlr_reg & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_NAK;
  // *rxctlr_reg = (*rxctlr_reg & ~USBHS_UEP_R_RES_MASK) | USBHS_UEP_R_RES_NAK;
  USBInEndpointState *isp = usbp->epc[(ep & 0x7f)]->in_state;
  isp->txlast = isp->txsize < usbp->epc[(ep & 0x7f)]->in_maxsize ? isp->txsize : usbp->epc[(ep & 0x7f)]->in_maxsize;
  if(ep == 0){
    if(usbp->ep0state == USB_EP0_IN_SENDING_STS)
    {
        *txctlr_reg |= USBHS_UEP_T_TOG_DATA1;
    }
  }
  *txaddr_reg = (uint32_t)isp->txbuf + isp->txcnt;
  *txlen_reg = isp->txlast;
  *txctlr_reg = (*txctlr_reg & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_ACK;
}

/**
 * @brief   Brings an OUT endpoint in the stalled state.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_stall_out(USBDriver *usbp, usbep_t ep) {

  (void)usbp;
  (void)ep;
  volatile uint8_t *rxctlr_reg;

  switch (ep & 0x7f)
  {
  case 0:    rxctlr_reg = &(CH32_USB_DEVICE->UEP0_RX_CTRL);    break;  
  case 1:    rxctlr_reg = &(CH32_USB_DEVICE->UEP1_RX_CTRL);    break;  
  case 2:    rxctlr_reg = &(CH32_USB_DEVICE->UEP2_RX_CTRL);    break;  
  case 3:    rxctlr_reg = &(CH32_USB_DEVICE->UEP3_RX_CTRL);    break;  
  case 4:    rxctlr_reg = &(CH32_USB_DEVICE->UEP4_RX_CTRL);    break;  
  case 5:    rxctlr_reg = &(CH32_USB_DEVICE->UEP5_RX_CTRL);    break;  
  case 6:    rxctlr_reg = &(CH32_USB_DEVICE->UEP6_RX_CTRL);    break;  
  case 7:    rxctlr_reg = &(CH32_USB_DEVICE->UEP7_RX_CTRL);    break;  
  default:
  return;
    break;
  }

  *rxctlr_reg = (*rxctlr_reg & ~USBHS_UEP_R_RES_MASK) | USBHS_UEP_R_RES_STALL;
}

/**
 * @brief   Brings an IN endpoint in the stalled state.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_stall_in(USBDriver *usbp, usbep_t ep) {

  (void)usbp;
  (void)ep;
  volatile uint8_t *txctlr_reg;

  switch (ep & 0x7f)
  {
  case 0:    txctlr_reg = &(CH32_USB_DEVICE->UEP0_TX_CTRL);    break;  
  case 1:    txctlr_reg = &(CH32_USB_DEVICE->UEP1_TX_CTRL);    break;  
  case 2:    txctlr_reg = &(CH32_USB_DEVICE->UEP2_TX_CTRL);    break;  
  case 3:    txctlr_reg = &(CH32_USB_DEVICE->UEP3_TX_CTRL);    break;  
  case 4:    txctlr_reg = &(CH32_USB_DEVICE->UEP4_TX_CTRL);    break;  
  case 5:    txctlr_reg = &(CH32_USB_DEVICE->UEP5_TX_CTRL);    break;  
  case 6:    txctlr_reg = &(CH32_USB_DEVICE->UEP6_TX_CTRL);    break;  
  case 7:    txctlr_reg = &(CH32_USB_DEVICE->UEP7_TX_CTRL);    break;  
  default:
  return;
    break;
  }
  
  *txctlr_reg = (*txctlr_reg & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_STALL;
}

/**
 * @brief   Brings an OUT endpoint in the active state.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_clear_out(USBDriver *usbp, usbep_t ep) {

  (void)usbp;
  (void)ep;
  volatile uint8_t *rxctlr_reg;

  switch (ep & 0x7f)
  {
  case 0:    rxctlr_reg = &(CH32_USB_DEVICE->UEP0_RX_CTRL);    break;  
  case 1:    rxctlr_reg = &(CH32_USB_DEVICE->UEP1_RX_CTRL);    break;  
  case 2:    rxctlr_reg = &(CH32_USB_DEVICE->UEP2_RX_CTRL);    break;  
  case 3:    rxctlr_reg = &(CH32_USB_DEVICE->UEP3_RX_CTRL);    break;  
  case 4:    rxctlr_reg = &(CH32_USB_DEVICE->UEP4_RX_CTRL);    break;  
  case 5:    rxctlr_reg = &(CH32_USB_DEVICE->UEP5_RX_CTRL);    break;  
  case 6:    rxctlr_reg = &(CH32_USB_DEVICE->UEP6_RX_CTRL);    break;  
  case 7:    rxctlr_reg = &(CH32_USB_DEVICE->UEP7_RX_CTRL);    break;  
  default:
  return;
    break;
  }

  *rxctlr_reg &= ~(USBHS_UEP_R_DONE | USBHS_UEP_R_RES_ACK);
}

/**
 * @brief   Brings an IN endpoint in the active state.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_clear_in(USBDriver *usbp, usbep_t ep) {

  (void)usbp;
  (void)ep;
  volatile uint8_t *txctlr_reg;

  switch (ep & 0x7f)
  {
  case 0:    txctlr_reg = &(CH32_USB_DEVICE->UEP0_TX_CTRL);    break;  
  case 1:    txctlr_reg = &(CH32_USB_DEVICE->UEP1_TX_CTRL);    break;  
  case 2:    txctlr_reg = &(CH32_USB_DEVICE->UEP2_TX_CTRL);    break;  
  case 3:    txctlr_reg = &(CH32_USB_DEVICE->UEP3_TX_CTRL);    break;  
  case 4:    txctlr_reg = &(CH32_USB_DEVICE->UEP4_TX_CTRL);    break;  
  case 5:    txctlr_reg = &(CH32_USB_DEVICE->UEP5_TX_CTRL);    break;  
  case 6:    txctlr_reg = &(CH32_USB_DEVICE->UEP6_TX_CTRL);    break;  
  case 7:    txctlr_reg = &(CH32_USB_DEVICE->UEP7_TX_CTRL);    break;  
  default:
  return;
    break;
  }
  *txctlr_reg &= ~(USBHS_UEP_T_DONE | USBHS_UEP_T_RES_ACK);
}

#endif /* HAL_USE_USB == TRUE */

/** @} */
