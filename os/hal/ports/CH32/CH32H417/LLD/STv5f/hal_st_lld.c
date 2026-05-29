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
 * @file    STv5f/hal_st_lld.c
 * @brief   CH32H417 V5F core ST subsystem low level driver source.
 *
 * @warning This is a SKELETON file for the V5F core.
 *          Implement the V5F-specific system timer initialization here.
 *
 * @addtogroup ST
 * @{
 */

#include "hal.h"

#if (OSAL_ST_MODE != OSAL_ST_MODE_NONE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#if OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING
/* TODO: Define V5F free running timer configuration */
#error "V5F freerunning ST mode not yet implemented"

#elif OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC
/* TODO: Define V5F periodic tick configuration */
#error "V5F periodic ST mode not yet implemented"

#endif

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/* TODO: Add V5F core interrupt handler definition */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level ST driver initialization for V5F core.
 *
 * @notapi
 */
void st_lld_init(void) {

  /* TODO: Implement V5F core system timer initialization */
}

/**
 * @brief   ST interrupt service routine for V5F core.
 */
void st_lld_serve_interrupt(void) {

  /* TODO: Implement V5F core interrupt service routine */
}

#endif /* OSAL_ST_MODE != OSAL_ST_MODE_NONE */

/** @} */
