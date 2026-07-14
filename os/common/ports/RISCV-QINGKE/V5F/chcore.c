/*
    ChibiOS - Copyright (C) 2006-2026 Giovanni Di Sirio.

    This file is part of ChibiOS.

    ChibiOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation version 3 of the License.

    ChibiOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    templates/chcore.c
 * @brief   Port related template code.
 *
 * @addtogroup port_core
 * @details Non portable code templates.
 * @{
 */

#include "ch.h"
/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

#define MACHINE_WORD_SIZE __riscv_xlen

#define STORE_SP(reg, offset) asm volatile("sw sp, %0(" __CH_STRINGIFY(reg) ")" ::"i"(offset) : "memory")
#define LOAD_SP(reg, offset) asm volatile("lw sp, %0(" __CH_STRINGIFY(reg) ")" ::"i"(offset) :)
/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Port-related initialization code.
 * @note    This function is usually empty.
 */
void _port_init(void)
{
}

/**
 * @brief   Performs a context switch between two threads.
 * @details This is the most critical code in any port, this function
 *          is responsible for the context switch between 2 threads.
 * @note    The implementation of this code affects <b>directly</b> the context
 *          switch performance so optimize here as much as you can.
 *
 * @param[in] ntp       the thread to be switched in
 * @param[in] otp       the thread to be switched out
 */
__attribute__((naked)) void _port_switch(thread_t *ntp, thread_t *otp)
{
    (void)otp;
    (void)ntp;

    save_context();

    STORE_SP(a1, offsetof(thread_t, ctx));

    LOAD_SP(a0, offsetof(thread_t, ctx));

    recover_context();

    asm volatile("ret");
}


extern uint32_t vector_start[];
extern void HardFault_Handler(void);
#include "core_riscv.h"
__attribute__((naked, section(".irq_entry"))) void _port_irq_handler(void)
{
    CH_IRQ_PROLOGUE();
    void (*_handler)(void);

    uint32_t irq_reason = __get_MCAUSE();

    if ((irq_reason & 0x80000000) != 0)
    {
        uint32_t irq_id = irq_reason & 0x7fffffff;
        uint32_t vectoraddr = vector_start[irq_id];

        _handler = (void (*)(void))vectoraddr;
    }
    else
    {
        _handler = HardFault_Handler;
    }

    _handler();

    CH_IRQ_EPILOGUE();
}

__attribute__((naked)) void _port_thread_start()
{
    chSysUnlock();
    asm volatile("mv a0, x9");
    asm volatile("jalr ra, x8");
    asm volatile("li a0, 0");
    asm volatile("jal ra, chThdExit");
    asm volatile("1: j 1b");
}

/** @} */
