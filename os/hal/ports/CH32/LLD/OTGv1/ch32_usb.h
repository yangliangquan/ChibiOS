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
 * @file    ch32_usb.h
 * @brief   CH32 USBFS (OTGv1) register definitions.
 *
 * @addtogroup USB
 * @{
 */

#ifndef CH32_USB_H
#define CH32_USB_H

#if (HAL_USE_USB == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/* USBFS register base address.*/
#define CH32_USBFS_DEVICE                         ((USBFSD_TypeDef *)USBFS_BASE)
#define CH32_USBFS_HOST                           ((USBFSH_TypeDef *)USBFS_BASE)

/**
 * @brief   Default EP0 maximum packet size for USB Full Speed.
 */
#if !defined(DEF_USBD_UEP0_SIZE) || defined(__DOXYGEN__)
#define DEF_USBD_UEP0_SIZE                        64
#endif

/*===========================================================================*/
/* USBFS BASE_CTRL register bits.                                            */
/*===========================================================================*/
#define USBFS_UC_HOST_MODE          0x80
#define USBFS_UC_LOW_SPEED          0x40
#define USBFS_UC_DEV_PU_EN          0x20
#define USBFS_UC_SYS_CTRL_MASK      0x30
#define USBFS_UC_SYS_CTRL0          0x00
#define USBFS_UC_SYS_CTRL1          0x10
#define USBFS_UC_SYS_CTRL2          0x20
#define USBFS_UC_SYS_CTRL3          0x30
#define USBFS_UC_INT_BUSY           0x08
#define USBFS_UC_RESET_SIE          0x04
#define USBFS_UC_CLR_ALL            0x02
#define USBFS_UC_DMA_EN             0x01

/*===========================================================================*/
/* USBFS INT_EN register bits.                                               */
/*===========================================================================*/
#define USBFS_UIE_DEV_SOF           0x80
#define USBFS_UIE_DEV_NAK           0x40
#define USBFS_U_1WIRE_MODE          0x20
#define USBFS_UIE_FIFO_OV           0x10
#define USBFS_UIE_HST_SOF           0x08
#define USBFS_UIE_SUSPEND           0x04
#define USBFS_UIE_TRANSFER          0x02
#define USBFS_UIE_DETECT            0x01
#define USBFS_UIE_BUS_RST           0x01

/*===========================================================================*/
/* USBFS DEV_ADDR register bits.                                             */
/*===========================================================================*/
#define USBFS_UDA_GP_BIT            0x80
#define USBFS_USB_ADDR_MASK         0x7F

/*===========================================================================*/
/* USBFS MIS_ST register bits.                                               */
/*===========================================================================*/
#define USBFS_UMS_SOF_PRES          0x80
#define USBFS_UMS_SOF_ACT           0x40
#define USBFS_UMS_SIE_FREE          0x20
#define USBFS_UMS_R_FIFO_RDY        0x10
#define USBFS_UMS_BUS_RESET         0x08
#define USBFS_UMS_SUSPEND           0x04
#define USBFS_UMS_DM_LEVEL          0x02
#define USBFS_UMS_DEV_ATTACH        0x01

/*===========================================================================*/
/* USBFS INT_FG register bits.                                               */
/*===========================================================================*/
#define USBFS_U_IS_NAK              0x80
#define USBFS_U_TOG_OK              0x40
#define USBFS_U_SIE_FREE            0x20
#define USBFS_UIF_FIFO_OV           0x10
#define USBFS_UIF_HST_SOF           0x08
#define USBFS_UIF_SUSPEND           0x04
#define USBFS_UIF_TRANSFER          0x02
#define USBFS_UIF_DETECT            0x01
#define USBFS_UIF_BUS_RST           0x01

/*===========================================================================*/
/* USBFS INT_ST register bits.                                               */
/*===========================================================================*/
#define USBFS_UIS_IS_NAK            0x80
#define USBFS_UIS_TOG_OK            0x40
#define USBFS_UIS_TOKEN_MASK        0x30
#define USBFS_UIS_TOKEN_OUT         0x00
#define USBFS_UIS_TOKEN_SOF         0x10
#define USBFS_UIS_TOKEN_IN          0x20
#define USBFS_UIS_TOKEN_SETUP       0x30
#define USBFS_UIS_ENDP_MASK         0x0F
#define USBFS_UIS_H_RES_MASK        0x0F

/*===========================================================================*/
/* USBFS OTG_CR register bits.                                               */
/*===========================================================================*/
#define USBFS_CR_SESS_VTH           0x20
#define USBFS_CR_VBUS_VTH           0x10
#define USBFS_CR_OTG_EN             0x08
#define USBFS_CR_IDPU               0x04
#define USBFS_CR_CHARGE_VBUS        0x02
#define USBFS_CR_DISCHAR_VBUS       0x01

/*===========================================================================*/
/* USBFS OTG_SR register bits.                                               */
/*===========================================================================*/
#define USBFS_SR_ID_DIG             0x08
#define USBFS_SR_SESS_END           0x04
#define USBFS_SR_SESS_VLD           0x02
#define USBFS_SR_VBUS_VLD           0x01

/*===========================================================================*/
/* USBFS UDEV_CTRL register bits.                                            */
/*===========================================================================*/
#define USBFS_UD_PD_DIS             0x80
#define USBFS_UD_DP_PIN             0x20
#define USBFS_UD_DM_PIN             0x10
#define USBFS_UD_LOW_SPEED          0x04
#define USBFS_UD_GP_BIT             0x02
#define USBFS_UD_PORT_EN            0x01

/*===========================================================================*/
/* USBFS UEP4_1_MOD register bits.                                           */
/*===========================================================================*/
#define USBFS_UEP1_RX_EN            0x80
#define USBFS_UEP1_TX_EN            0x40
#define USBFS_UEP1_BUF_MOD          0x10
#define USBFS_UEP4_RX_EN            0x08
#define USBFS_UEP4_TX_EN            0x04
#define USBFS_UEP4_BUF_MOD          0x01

/*===========================================================================*/
/* USBFS UEP2_3_MOD register bits.                                           */
/*===========================================================================*/
#define USBFS_UEP3_RX_EN            0x80
#define USBFS_UEP3_TX_EN            0x40
#define USBFS_UEP3_BUF_MOD          0x10
#define USBFS_UEP2_RX_EN            0x08
#define USBFS_UEP2_TX_EN            0x04
#define USBFS_UEP2_BUF_MOD          0x01

/*===========================================================================*/
/* USBFS UEP5_6_MOD register bits.                                           */
/*===========================================================================*/
#define USBFS_UEP6_RX_EN            0x80
#define USBFS_UEP6_TX_EN            0x40
#define USBFS_UEP6_BUF_MOD          0x10
#define USBFS_UEP5_RX_EN            0x08
#define USBFS_UEP5_TX_EN            0x04
#define USBFS_UEP5_BUF_MOD          0x01

/*===========================================================================*/
/* USBFS UEP7_MOD register bits.                                             */
/*===========================================================================*/
#define USBFS_UEP7_RX_EN            0x08
#define USBFS_UEP7_TX_EN            0x04
#define USBFS_UEP7_BUF_MOD          0x01

/*===========================================================================*/
/* USBFS UEPn_TX_CTRL register bits.                                         */
/*===========================================================================*/
#define USBFS_UEP_T_AUTO_TOG        0x08
#define USBFS_UEP_T_TOG             0x04
#define USBFS_UEP_T_RES_MASK        0x03
#define USBFS_UEP_T_RES_ACK         0x00
#define USBFS_UEP_T_RES_NONE        0x01
#define USBFS_UEP_T_RES_NAK         0x02
#define USBFS_UEP_T_RES_STALL       0x03

/*===========================================================================*/
/* USBFS UEPn_RX_CTRL register bits.                                         */
/*===========================================================================*/
#define USBFS_UEP_R_AUTO_TOG        0x08
#define USBFS_UEP_R_TOG             0x04
#define USBFS_UEP_R_RES_MASK        0x03
#define USBFS_UEP_R_RES_ACK         0x00
#define USBFS_UEP_R_RES_NONE        0x01
#define USBFS_UEP_R_RES_NAK         0x02
#define USBFS_UEP_R_RES_STALL       0x03

/*===========================================================================*/
/* USBFS HOST registers bits.                                                */
/*===========================================================================*/
#define USBFS_UH_PD_DIS             0x80
#define USBFS_UH_DP_PIN             0x20
#define USBFS_UH_DM_PIN             0x10
#define USBFS_UH_LOW_SPEED          0x04
#define USBFS_UH_BUS_RESET          0x02
#define USBFS_UH_PORT_EN            0x01

#define USBFS_UH_EP_TX_EN           0x40
#define USBFS_UH_EP_TBUF_MOD        0x10
#define USBFS_UH_EP_RX_EN           0x08
#define USBFS_UH_EP_RBUF_MOD        0x01

#define USBFS_UH_PRE_PID_EN         0x0400
#define USBFS_UH_SOF_EN             0x0004

#define USBFS_UH_TOKEN_MASK         0xF0
#define USBFS_UH_ENDP_MASK          0x0F

#define USBFS_UH_R_AUTO_TOG         0x08
#define USBFS_UH_R_TOG              0x04
#define USBFS_UH_R_RES              0x01

#define USBFS_UH_T_AUTO_TOG         0x08
#define USBFS_UH_T_TOG              0x04
#define USBFS_UH_T_RES              0x01

#endif /* HAL_USE_USB == TRUE */

#endif /* CH32_USB_H */
