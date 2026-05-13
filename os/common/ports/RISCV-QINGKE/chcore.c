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

#define STORE_REGISTER(r, offset)                                                                                      \
    asm volatile("sw "                                                                                                 \
                 "x" __CH_STRINGIFY(r) ","                                                                             \
                                       "%0(sp)" ::"i"(offset)                                                          \
                 : "memory");
#define LOAD_REGISTER(r, offset)                                                                                       \
    asm volatile("lw "                                                                                                 \
                 "x" __CH_STRINGIFY(r) ","                                                                             \
                                       "%0(sp)" ::"i"(offset)                                                          \
                 : "x" __CH_STRINGIFY(r));
#define STORE_FLOAT_REGISTER(r, offset)                                                                                \
    asm volatile("fsw "                                                                                                \
                 "f" __CH_STRINGIFY(r) ","                                                                             \
                                       "%0(sp)" ::"i"(offset)                                                          \
                 : "memory");
#define LOAD_FLOAT_REGISTER(r, offset)                                                                                 \
    asm volatile("flw "                                                                                                \
                 "f" __CH_STRINGIFY(r) ","                                                                             \
                                       "%0(sp)" ::"i"(offset)                                                          \
                 : "f" __CH_STRINGIFY(r));

#define STORE_CSR(r, offset)                                                                                           \
    asm volatile("csrr t0, " __CH_STRINGIFY(r));                                                                       \
    asm volatile("sw t0, %0(sp)" ::"i"(offset) :);
#define LOAD_CSR(r, offset)                                                                                            \
    asm volatile("lw t0, %0(sp)" ::"i"(offset) :);                                                                     \
    asm volatile("csrw " __CH_STRINGIFY(r) ","                                                                         \
                                           "t0"                                                                        \
                 :);

#define STORE_SP(reg, offset) asm volatile("sw sp, %0(" __CH_STRINGIFY(reg) ")" ::"i"(offset) :)
#define LOAD_SP(reg, offset) asm volatile("lw sp, %0(" __CH_STRINGIFY(reg) ")" ::"i"(offset) :)

#define RELOAD_SP(offset)                                                                                              \
    asm volatile("addi "                                                                                               \
                 "sp, sp, %0" ::"i"(offset)                                                                            \
                 :);
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
/* Module local functions.                                                   */
/*===========================================================================*/
__attribute__((always_inline)) inline void save_context(void)
{
    RELOAD_SP(-sizeof(struct port_intctx));
    STORE_REGISTER(1, offsetof(struct port_intctx, x1));
    STORE_REGISTER(5, offsetof(struct port_intctx, x5));
    STORE_REGISTER(6, offsetof(struct port_intctx, x6));
    STORE_REGISTER(7, offsetof(struct port_intctx, x7));
    STORE_REGISTER(8, offsetof(struct port_intctx, x8));
    STORE_REGISTER(9, offsetof(struct port_intctx, x9));
    STORE_REGISTER(10, offsetof(struct port_intctx, x10));
    STORE_REGISTER(11, offsetof(struct port_intctx, x11));
    STORE_REGISTER(12, offsetof(struct port_intctx, x12));
    STORE_REGISTER(13, offsetof(struct port_intctx, x13));
    STORE_REGISTER(14, offsetof(struct port_intctx, x14));
    STORE_REGISTER(15, offsetof(struct port_intctx, x15));
    STORE_REGISTER(16, offsetof(struct port_intctx, x16));
    STORE_REGISTER(17, offsetof(struct port_intctx, x17));
    STORE_REGISTER(18, offsetof(struct port_intctx, x18));
    STORE_REGISTER(19, offsetof(struct port_intctx, x19));
    STORE_REGISTER(20, offsetof(struct port_intctx, x20));
    STORE_REGISTER(21, offsetof(struct port_intctx, x21));
    STORE_REGISTER(22, offsetof(struct port_intctx, x22));
    STORE_REGISTER(23, offsetof(struct port_intctx, x23));
    STORE_REGISTER(24, offsetof(struct port_intctx, x24));
    STORE_REGISTER(25, offsetof(struct port_intctx, x25));
    STORE_REGISTER(26, offsetof(struct port_intctx, x26));
    STORE_REGISTER(27, offsetof(struct port_intctx, x27));
    STORE_REGISTER(28, offsetof(struct port_intctx, x28));
    STORE_REGISTER(29, offsetof(struct port_intctx, x29));
    STORE_REGISTER(30, offsetof(struct port_intctx, x30));
    STORE_REGISTER(31, offsetof(struct port_intctx, x31));
#ifndef __riscv_float_abi_soft
    STORE_FLOAT_REGISTER(0, offsetof(struct port_intctx, f0));
    STORE_FLOAT_REGISTER(1, offsetof(struct port_intctx, f1));
    STORE_FLOAT_REGISTER(2, offsetof(struct port_intctx, f2));
    STORE_FLOAT_REGISTER(3, offsetof(struct port_intctx, f3));
    STORE_FLOAT_REGISTER(4, offsetof(struct port_intctx, f4));
    STORE_FLOAT_REGISTER(5, offsetof(struct port_intctx, f5));
    STORE_FLOAT_REGISTER(6, offsetof(struct port_intctx, f6));
    STORE_FLOAT_REGISTER(7, offsetof(struct port_intctx, f7));
    STORE_FLOAT_REGISTER(8, offsetof(struct port_intctx, f8));
    STORE_FLOAT_REGISTER(9, offsetof(struct port_intctx, f9));
    STORE_FLOAT_REGISTER(10, offsetof(struct port_intctx, f10));
    STORE_FLOAT_REGISTER(11, offsetof(struct port_intctx, f11));
    STORE_FLOAT_REGISTER(12, offsetof(struct port_intctx, f12));
    STORE_FLOAT_REGISTER(13, offsetof(struct port_intctx, f13));
    STORE_FLOAT_REGISTER(14, offsetof(struct port_intctx, f14));
    STORE_FLOAT_REGISTER(15, offsetof(struct port_intctx, f15));
    STORE_FLOAT_REGISTER(16, offsetof(struct port_intctx, f16));
    STORE_FLOAT_REGISTER(17, offsetof(struct port_intctx, f17));
    STORE_FLOAT_REGISTER(18, offsetof(struct port_intctx, f18));
    STORE_FLOAT_REGISTER(19, offsetof(struct port_intctx, f19));
    STORE_FLOAT_REGISTER(20, offsetof(struct port_intctx, f20));
    STORE_FLOAT_REGISTER(21, offsetof(struct port_intctx, f21));
    STORE_FLOAT_REGISTER(22, offsetof(struct port_intctx, f22));
    STORE_FLOAT_REGISTER(23, offsetof(struct port_intctx, f23));
    STORE_FLOAT_REGISTER(24, offsetof(struct port_intctx, f24));
    STORE_FLOAT_REGISTER(25, offsetof(struct port_intctx, f25));
    STORE_FLOAT_REGISTER(26, offsetof(struct port_intctx, f26));
    STORE_FLOAT_REGISTER(27, offsetof(struct port_intctx, f27));
    STORE_FLOAT_REGISTER(28, offsetof(struct port_intctx, f28));
    STORE_FLOAT_REGISTER(29, offsetof(struct port_intctx, f29));
    STORE_FLOAT_REGISTER(30, offsetof(struct port_intctx, f30));
    STORE_FLOAT_REGISTER(31, offsetof(struct port_intctx, f31));
#endif
    STORE_CSR(mstatus, offsetof(struct port_intctx, mstatus));
    STORE_CSR(mepc, offsetof(struct port_intctx, mepc));
}

__attribute__((always_inline)) inline void recover_context(void)
{
    LOAD_REGISTER(1, offsetof(struct port_intctx, x1));
    LOAD_REGISTER(5, offsetof(struct port_intctx, x5));
    LOAD_REGISTER(6, offsetof(struct port_intctx, x6));
    LOAD_REGISTER(7, offsetof(struct port_intctx, x7));
    LOAD_REGISTER(8, offsetof(struct port_intctx, x8));
    LOAD_REGISTER(9, offsetof(struct port_intctx, x9));
    LOAD_REGISTER(10, offsetof(struct port_intctx, x10));
    LOAD_REGISTER(11, offsetof(struct port_intctx, x11));
    LOAD_REGISTER(12, offsetof(struct port_intctx, x12));
    LOAD_REGISTER(13, offsetof(struct port_intctx, x13));
    LOAD_REGISTER(14, offsetof(struct port_intctx, x14));
    LOAD_REGISTER(15, offsetof(struct port_intctx, x15));
    LOAD_REGISTER(16, offsetof(struct port_intctx, x16));
    LOAD_REGISTER(17, offsetof(struct port_intctx, x17));
    LOAD_REGISTER(18, offsetof(struct port_intctx, x18));
    LOAD_REGISTER(19, offsetof(struct port_intctx, x19));
    LOAD_REGISTER(20, offsetof(struct port_intctx, x20));
    LOAD_REGISTER(21, offsetof(struct port_intctx, x21));
    LOAD_REGISTER(22, offsetof(struct port_intctx, x22));
    LOAD_REGISTER(23, offsetof(struct port_intctx, x23));
    LOAD_REGISTER(24, offsetof(struct port_intctx, x24));
    LOAD_REGISTER(25, offsetof(struct port_intctx, x25));
    LOAD_REGISTER(26, offsetof(struct port_intctx, x26));
    LOAD_REGISTER(27, offsetof(struct port_intctx, x27));
    LOAD_REGISTER(28, offsetof(struct port_intctx, x28));
    LOAD_REGISTER(29, offsetof(struct port_intctx, x29));
    LOAD_REGISTER(30, offsetof(struct port_intctx, x30));
    LOAD_REGISTER(31, offsetof(struct port_intctx, x31));
#ifndef __riscv_float_abi_soft
    LOAD_FLOAT_REGISTER(0, offsetof(struct port_intctx, f0));
    LOAD_FLOAT_REGISTER(1, offsetof(struct port_intctx, f1));
    LOAD_FLOAT_REGISTER(2, offsetof(struct port_intctx, f2));
    LOAD_FLOAT_REGISTER(3, offsetof(struct port_intctx, f3));
    LOAD_FLOAT_REGISTER(4, offsetof(struct port_intctx, f4));
    LOAD_FLOAT_REGISTER(5, offsetof(struct port_intctx, f5));
    LOAD_FLOAT_REGISTER(6, offsetof(struct port_intctx, f6));
    LOAD_FLOAT_REGISTER(7, offsetof(struct port_intctx, f7));
    LOAD_FLOAT_REGISTER(8, offsetof(struct port_intctx, f8));
    LOAD_FLOAT_REGISTER(9, offsetof(struct port_intctx, f9));
    LOAD_FLOAT_REGISTER(10, offsetof(struct port_intctx, f10));
    LOAD_FLOAT_REGISTER(11, offsetof(struct port_intctx, f11));
    LOAD_FLOAT_REGISTER(12, offsetof(struct port_intctx, f12));
    LOAD_FLOAT_REGISTER(13, offsetof(struct port_intctx, f13));
    LOAD_FLOAT_REGISTER(14, offsetof(struct port_intctx, f14));
    LOAD_FLOAT_REGISTER(15, offsetof(struct port_intctx, f15));
    LOAD_FLOAT_REGISTER(16, offsetof(struct port_intctx, f16));
    LOAD_FLOAT_REGISTER(17, offsetof(struct port_intctx, f17));
    LOAD_FLOAT_REGISTER(18, offsetof(struct port_intctx, f18));
    LOAD_FLOAT_REGISTER(19, offsetof(struct port_intctx, f19));
    LOAD_FLOAT_REGISTER(20, offsetof(struct port_intctx, f20));
    LOAD_FLOAT_REGISTER(21, offsetof(struct port_intctx, f21));
    LOAD_FLOAT_REGISTER(22, offsetof(struct port_intctx, f22));
    LOAD_FLOAT_REGISTER(23, offsetof(struct port_intctx, f23));
    LOAD_FLOAT_REGISTER(24, offsetof(struct port_intctx, f24));
    LOAD_FLOAT_REGISTER(25, offsetof(struct port_intctx, f25));
    LOAD_FLOAT_REGISTER(26, offsetof(struct port_intctx, f26));
    LOAD_FLOAT_REGISTER(27, offsetof(struct port_intctx, f27));
    LOAD_FLOAT_REGISTER(28, offsetof(struct port_intctx, f28));
    LOAD_FLOAT_REGISTER(29, offsetof(struct port_intctx, f29));
    LOAD_FLOAT_REGISTER(30, offsetof(struct port_intctx, f30));
    LOAD_FLOAT_REGISTER(31, offsetof(struct port_intctx, f31));
#endif
    LOAD_CSR(mstatus, offsetof(struct port_intctx, mstatus));
    LOAD_CSR(mepc, offsetof(struct port_intctx, mepc));
    RELOAD_SP(sizeof(struct port_intctx));
}

__attribute__((always_inline)) inline void save_context_irq(void)
{
    RELOAD_SP(-sizeof(struct port_extctx));
    STORE_REGISTER(1, offsetof(struct port_extctx, x1));
    STORE_REGISTER(5, offsetof(struct port_extctx, x5));
    STORE_REGISTER(6, offsetof(struct port_extctx, x6));
    STORE_REGISTER(7, offsetof(struct port_extctx, x7));
    STORE_REGISTER(8, offsetof(struct port_extctx, x8));
    STORE_REGISTER(9, offsetof(struct port_extctx, x9));
    STORE_REGISTER(10, offsetof(struct port_extctx, x10));
    STORE_REGISTER(11, offsetof(struct port_extctx, x11));
    STORE_REGISTER(12, offsetof(struct port_extctx, x12));
    STORE_REGISTER(13, offsetof(struct port_extctx, x13));
    STORE_REGISTER(14, offsetof(struct port_extctx, x14));
    STORE_REGISTER(15, offsetof(struct port_extctx, x15));
    STORE_REGISTER(16, offsetof(struct port_extctx, x16));
    STORE_REGISTER(17, offsetof(struct port_extctx, x17));
    STORE_REGISTER(18, offsetof(struct port_extctx, x18));
    STORE_REGISTER(19, offsetof(struct port_extctx, x19));
    STORE_REGISTER(20, offsetof(struct port_extctx, x20));
    STORE_REGISTER(21, offsetof(struct port_extctx, x21));
    STORE_REGISTER(22, offsetof(struct port_extctx, x22));
    STORE_REGISTER(23, offsetof(struct port_extctx, x23));
    STORE_REGISTER(24, offsetof(struct port_extctx, x24));
    STORE_REGISTER(25, offsetof(struct port_extctx, x25));
    STORE_REGISTER(26, offsetof(struct port_extctx, x26));
    STORE_REGISTER(27, offsetof(struct port_extctx, x27));
    STORE_REGISTER(28, offsetof(struct port_extctx, x28));
    STORE_REGISTER(29, offsetof(struct port_extctx, x29));
    STORE_REGISTER(30, offsetof(struct port_extctx, x30));
    STORE_REGISTER(31, offsetof(struct port_extctx, x31));
#ifndef __riscv_float_abi_soft
    STORE_FLOAT_REGISTER(0, offsetof(struct port_extctx, f0));
    STORE_FLOAT_REGISTER(1, offsetof(struct port_extctx, f1));
    STORE_FLOAT_REGISTER(2, offsetof(struct port_extctx, f2));
    STORE_FLOAT_REGISTER(3, offsetof(struct port_extctx, f3));
    STORE_FLOAT_REGISTER(4, offsetof(struct port_extctx, f4));
    STORE_FLOAT_REGISTER(5, offsetof(struct port_extctx, f5));
    STORE_FLOAT_REGISTER(6, offsetof(struct port_extctx, f6));
    STORE_FLOAT_REGISTER(7, offsetof(struct port_extctx, f7));
    STORE_FLOAT_REGISTER(8, offsetof(struct port_extctx, f8));
    STORE_FLOAT_REGISTER(9, offsetof(struct port_extctx, f9));
    STORE_FLOAT_REGISTER(10, offsetof(struct port_extctx, f10));
    STORE_FLOAT_REGISTER(11, offsetof(struct port_extctx, f11));
    STORE_FLOAT_REGISTER(12, offsetof(struct port_extctx, f12));
    STORE_FLOAT_REGISTER(13, offsetof(struct port_extctx, f13));
    STORE_FLOAT_REGISTER(14, offsetof(struct port_extctx, f14));
    STORE_FLOAT_REGISTER(15, offsetof(struct port_extctx, f15));
    STORE_FLOAT_REGISTER(16, offsetof(struct port_extctx, f16));
    STORE_FLOAT_REGISTER(17, offsetof(struct port_extctx, f17));
    STORE_FLOAT_REGISTER(18, offsetof(struct port_extctx, f18));
    STORE_FLOAT_REGISTER(19, offsetof(struct port_extctx, f19));
    STORE_FLOAT_REGISTER(20, offsetof(struct port_extctx, f20));
    STORE_FLOAT_REGISTER(21, offsetof(struct port_extctx, f21));
    STORE_FLOAT_REGISTER(22, offsetof(struct port_extctx, f22));
    STORE_FLOAT_REGISTER(23, offsetof(struct port_extctx, f23));
    STORE_FLOAT_REGISTER(24, offsetof(struct port_extctx, f24));
    STORE_FLOAT_REGISTER(25, offsetof(struct port_extctx, f25));
    STORE_FLOAT_REGISTER(26, offsetof(struct port_extctx, f26));
    STORE_FLOAT_REGISTER(27, offsetof(struct port_extctx, f27));
    STORE_FLOAT_REGISTER(28, offsetof(struct port_extctx, f28));
    STORE_FLOAT_REGISTER(29, offsetof(struct port_extctx, f29));
    STORE_FLOAT_REGISTER(30, offsetof(struct port_extctx, f30));
    STORE_FLOAT_REGISTER(31, offsetof(struct port_extctx, f31));
#endif
}

__attribute__((always_inline)) inline void recover_context_irq(void)
{
    LOAD_REGISTER(1, offsetof(struct port_extctx, x1));
    LOAD_REGISTER(5, offsetof(struct port_extctx, x5));
    LOAD_REGISTER(6, offsetof(struct port_extctx, x6));
    LOAD_REGISTER(7, offsetof(struct port_extctx, x7));
    LOAD_REGISTER(8, offsetof(struct port_extctx, x8));
    LOAD_REGISTER(9, offsetof(struct port_extctx, x9));
    LOAD_REGISTER(10, offsetof(struct port_extctx, x10));
    LOAD_REGISTER(11, offsetof(struct port_extctx, x11));
    LOAD_REGISTER(12, offsetof(struct port_extctx, x12));
    LOAD_REGISTER(13, offsetof(struct port_extctx, x13));
    LOAD_REGISTER(14, offsetof(struct port_extctx, x14));
    LOAD_REGISTER(15, offsetof(struct port_extctx, x15));
    LOAD_REGISTER(16, offsetof(struct port_extctx, x16));
    LOAD_REGISTER(17, offsetof(struct port_extctx, x17));
    LOAD_REGISTER(18, offsetof(struct port_extctx, x18));
    LOAD_REGISTER(19, offsetof(struct port_extctx, x19));
    LOAD_REGISTER(20, offsetof(struct port_extctx, x20));
    LOAD_REGISTER(21, offsetof(struct port_extctx, x21));
    LOAD_REGISTER(22, offsetof(struct port_extctx, x22));
    LOAD_REGISTER(23, offsetof(struct port_extctx, x23));
    LOAD_REGISTER(24, offsetof(struct port_extctx, x24));
    LOAD_REGISTER(25, offsetof(struct port_extctx, x25));
    LOAD_REGISTER(26, offsetof(struct port_extctx, x26));
    LOAD_REGISTER(27, offsetof(struct port_extctx, x27));
    LOAD_REGISTER(28, offsetof(struct port_extctx, x28));
    LOAD_REGISTER(29, offsetof(struct port_extctx, x29));
    LOAD_REGISTER(30, offsetof(struct port_extctx, x30));
    LOAD_REGISTER(31, offsetof(struct port_extctx, x31));
#ifndef __riscv_float_abi_soft
    LOAD_FLOAT_REGISTER(0, offsetof(struct port_extctx, f0));
    LOAD_FLOAT_REGISTER(1, offsetof(struct port_extctx, f1));
    LOAD_FLOAT_REGISTER(2, offsetof(struct port_extctx, f2));
    LOAD_FLOAT_REGISTER(3, offsetof(struct port_extctx, f3));
    LOAD_FLOAT_REGISTER(4, offsetof(struct port_extctx, f4));
    LOAD_FLOAT_REGISTER(5, offsetof(struct port_extctx, f5));
    LOAD_FLOAT_REGISTER(6, offsetof(struct port_extctx, f6));
    LOAD_FLOAT_REGISTER(7, offsetof(struct port_extctx, f7));
    LOAD_FLOAT_REGISTER(8, offsetof(struct port_extctx, f8));
    LOAD_FLOAT_REGISTER(9, offsetof(struct port_extctx, f9));
    LOAD_FLOAT_REGISTER(10, offsetof(struct port_extctx, f10));
    LOAD_FLOAT_REGISTER(11, offsetof(struct port_extctx, f11));
    LOAD_FLOAT_REGISTER(12, offsetof(struct port_extctx, f12));
    LOAD_FLOAT_REGISTER(13, offsetof(struct port_extctx, f13));
    LOAD_FLOAT_REGISTER(14, offsetof(struct port_extctx, f14));
    LOAD_FLOAT_REGISTER(15, offsetof(struct port_extctx, f15));
    LOAD_FLOAT_REGISTER(16, offsetof(struct port_extctx, f16));
    LOAD_FLOAT_REGISTER(17, offsetof(struct port_extctx, f17));
    LOAD_FLOAT_REGISTER(18, offsetof(struct port_extctx, f18));
    LOAD_FLOAT_REGISTER(19, offsetof(struct port_extctx, f19));
    LOAD_FLOAT_REGISTER(20, offsetof(struct port_extctx, f20));
    LOAD_FLOAT_REGISTER(21, offsetof(struct port_extctx, f21));
    LOAD_FLOAT_REGISTER(22, offsetof(struct port_extctx, f22));
    LOAD_FLOAT_REGISTER(23, offsetof(struct port_extctx, f23));
    LOAD_FLOAT_REGISTER(24, offsetof(struct port_extctx, f24));
    LOAD_FLOAT_REGISTER(25, offsetof(struct port_extctx, f25));
    LOAD_FLOAT_REGISTER(26, offsetof(struct port_extctx, f26));
    LOAD_FLOAT_REGISTER(27, offsetof(struct port_extctx, f27));
    LOAD_FLOAT_REGISTER(28, offsetof(struct port_extctx, f28));
    LOAD_FLOAT_REGISTER(29, offsetof(struct port_extctx, f29));
    LOAD_FLOAT_REGISTER(30, offsetof(struct port_extctx, f30));
    LOAD_FLOAT_REGISTER(31, offsetof(struct port_extctx, f31));
#endif
    RELOAD_SP(sizeof(struct port_extctx));
}
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

// __attribute__((naked)) void __port_exit_from_isr(void)
// {
//     recover_context_irq();
// }

// __attribute__((naked)) void __port_switch_from_isr(void)
// {
//     chSysLockFromISR();
//     chSchDoPreemption();
//     chSysUnlockFromISR();
// }

__attribute__((always_inline)) inline void _port_irq_prologue(void)
{
    save_context_irq();
}

__attribute__((always_inline)) inline void _port_irq_epilogue(void)
{
    recover_context_irq();
    if (chSchIsPreemptionRequired())
    {
        chSysLock();
        chSchDoPreemption();
        chSysUnlock();
    }

    asm volatile("mret");
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
    asm volatile("zombie: j zombie");
}

/** @} */
