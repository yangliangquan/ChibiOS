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
 * @file    templates/chcore.h
 * @brief   Port related template macros and structures.
 * @details This file is a template of the system driver macros provided by
 *          a port.
 *
 * @addtogroup port_core
 * @{
 */

#ifndef CHCORE_H
#define CHCORE_H

/* Inclusion of the RISC-V V3F implementation specific parameters.*/
#include "riscvparams.h"

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/**
 * @name    Port Capabilities and Constants
 * @{
 */
/**
 * @brief   This port supports a realtime counter.
 */
#define PORT_SUPPORTS_RT                TRUE

/**
 * @brief   Natural alignment constant.
 * @note    It is the minimum alignment for pointer-size variables.
 */
#define PORT_NATURAL_ALIGN              sizeof (void *)

/**
 * @brief   Stack initial alignment constant.
 * @note    It is the alignment required for the stack pointer.
 */
#define PORT_STACK_ALIGN                4U

/**
 * @brief   Working Areas alignment constant.
 * @note    It is the alignment to be enforced for thread working areas.
 */
#define PORT_WORKING_AREA_ALIGN         4U
/** @} */

/**
 * @name    Priority Ranges
 * @{
 */

/**
 * @brief   Lowest priority level.
 */
#define PORT_LOWEST_PRIORITY            255

/**
 * @brief   Highest priority level.
 */
#define PORT_HIGHEST_PRIORITY           0

/**
 * @brief   Register number of the interrupt vector register.
 */
#define PORT_INT_VECTOR_REG             8
/** @} */

/**
 * @name    Architecture and Compiler
 * @{
 */
/**
 * @brief   Macro defining an RISCV architecture.
 */
#define PORT_ARCHITECTURE_RISCV

/**
 * @brief   Macro defining the specific RISCV architecture.
 */
#define PORT_ARCHITECTURE_RISCV_QINGKE

/**
 * @brief   Name of the implemented architecture.
 */
#define PORT_ARCHITECTURE_NAME          "RISC-V"

/**
 * @brief   Name of the architecture variant.
 */
#define PORT_CORE_VARIANT_NAME          "RV32IMAC"


/**
 * @brief   Compiler name and version.
 */
#if defined(__GNUC__) || defined(__DOXYGEN__)
#define PORT_COMPILER_NAME              "GCC " __VERSION__

#else
#error "unsupported compiler"
#endif

/**
 * @brief   Port-specific information string.
 */
#define PORT_INFO                       "RV32IMAC - QingKe"
/** @} */

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Stack size for the system idle thread.
 * @details This size depends on the idle thread implementation, usually
 *          the idle thread should take no more space than those reserved
 *          by @p PORT_INT_REQUIRED_STACK.
 */
#if !defined(PORT_IDLE_THREAD_STACK_SIZE) || defined(__DOXYGEN__)
#define PORT_IDLE_THREAD_STACK_SIZE     64
#endif

/**
 * @brief   Per-thread stack overhead for interrupts servicing.
 * @details This constant is used in the calculation of the correct working
 *          area size.
 */
#if !defined(PORT_INT_REQUIRED_STACK) || defined(__DOXYGEN__)
#define PORT_INT_REQUIRED_STACK         256
#endif

/**
 * @brief   Enables a "wait for interrupt" instruction in the idle loop.
 */
#if !defined(PORT_XXX_WFI_SLEEP_IDLE) || defined(__DOXYGEN__)
#define PORT_ENABLE_WFI_IDLE        FALSE
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/* The following code is not processed when the file is included from an
   asm module.*/
#if !defined(_FROM_ASM_)
#include "chtypes.h"
/**
 * @brief   Interrupt saved context.
 * @details This structure represents the stack frame saved during a
 *          preemption-capable interrupt handler.
 * @note    R2 and R13 are not saved because those are assumed to be immutable
 *          during the system life cycle.
 */
struct port_extctx
{
    uint32_t x1;      /* ra */
    uint32_t x5;      /* t0 */
    uint32_t x6;      /* t1 */
    uint32_t x7;      /* t2 */
    uint32_t x8;      /* s0/fp */
    uint32_t x9;      /* s1 */
    uint32_t x10;     /* a0 */
    uint32_t x11;     /* a1 */
    uint32_t x12;     /* a2 */
    uint32_t x13;     /* a3 */
    uint32_t x14;     /* a4 */
    uint32_t x15;     /* a5 */
    uint32_t x16;     /* a6 */
    uint32_t x17;     /* a7 */
    uint32_t x18;     /* s2 */
    uint32_t x19;     /* s3 */
    uint32_t x20;     /* s4 */
    uint32_t x21;     /* s5 */
    uint32_t x22;     /* s6 */
    uint32_t x23;     /* s7 */
    uint32_t x24;     /* s8 */
    uint32_t x25;     /* s9 */
    uint32_t x26;     /* s10 */
    uint32_t x27;     /* s11 */
    uint32_t x28;     /* t3 */
    uint32_t x29;     /* t4 */
    uint32_t x30;     /* t5 */
    uint32_t x31;     /* t6 */
#ifndef __riscv_float_abi_soft 
    float f0;         /* ft0 */
    float f1;         /* ft1 */
    float f2;         /* ft2 */
    float f3;         /* ft3 */
    float f4;         /* ft4 */
    float f5;         /* ft5 */
    float f6;         /* ft6 */
    float f7;         /* ft7 */
    float f8;         /* fs0 */
    float f9;         /* fs1 */
    float f10;        /* fa0 */
    float f11;        /* fa1 */
    float f12;        /* fa2 */
    float f13;        /* fa3 */
    float f14;        /* fa4 */
    float f15;        /* fa5 */
    float f16;        /* fa6 */
    float f17;        /* fa7 */
    float f18;        /* fs2 */
    float f19;        /* fs3 */
    float f20;        /* fs4 */
    float f21;        /* fs5 */
    float f22;        /* fs6 */
    float f23;        /* fs7 */
    float f24;        /* fs8 */
    float f25;        /* fs9 */
    float f26;        /* fs10 */
    float f27;        /* fs11 */
    float f28;        /* ft8 */
    float f29;        /* ft9 */
    float f30;        /* ft10 */
    float f31;        /* ft11 */
#endif
};

/**
 * @brief   System saved context.
 * @details This structure represents the inner stack frame during a context
 *          switching.
 * @note    R2 and R13 are not saved because those are assumed to be immutable
 *          during the system life cycle.
 * @note    LR is stored in the caller context so it is not present in this
 *          structure.
 */
struct port_intctx
{
    uint32_t x1;      /* ra */
    uint32_t x5;      /* t0 */
    uint32_t x6;      /* t1 */
    uint32_t x7;      /* t2 */
    uint32_t x8;      /* s0/fp */
    uint32_t x9;      /* s1 */
    uint32_t x10;     /* a0 */
    uint32_t x11;     /* a1 */
    uint32_t x12;     /* a2 */
    uint32_t x13;     /* a3 */
    uint32_t x14;     /* a4 */
    uint32_t x15;     /* a5 */
    uint32_t x16;     /* a6 */
    uint32_t x17;     /* a7 */
    uint32_t x18;     /* s2 */
    uint32_t x19;     /* s3 */
    uint32_t x20;     /* s4 */
    uint32_t x21;     /* s5 */
    uint32_t x22;     /* s6 */
    uint32_t x23;     /* s7 */
    uint32_t x24;     /* s8 */
    uint32_t x25;     /* s9 */
    uint32_t x26;     /* s10 */
    uint32_t x27;     /* s11 */
    uint32_t x28;     /* t3 */
    uint32_t x29;     /* t4 */
    uint32_t x30;     /* t5 */
    uint32_t x31;     /* t6 */
#ifndef __riscv_float_abi_soft 
    float f0;         /* ft0 */
    float f1;         /* ft1 */
    float f2;         /* ft2 */
    float f3;         /* ft3 */
    float f4;         /* ft4 */
    float f5;         /* ft5 */
    float f6;         /* ft6 */
    float f7;         /* ft7 */
    float f8;         /* fs0 */
    float f9;         /* fs1 */
    float f10;        /* fa0 */
    float f11;        /* fa1 */
    float f12;        /* fa2 */
    float f13;        /* fa3 */
    float f14;        /* fa4 */
    float f15;        /* fa5 */
    float f16;        /* fa6 */
    float f17;        /* fa7 */
    float f18;        /* fs2 */
    float f19;        /* fs3 */
    float f20;        /* fs4 */
    float f21;        /* fs5 */
    float f22;        /* fs6 */
    float f23;        /* fs7 */
    float f24;        /* fs8 */
    float f25;        /* fs9 */
    float f26;        /* fs10 */
    float f27;        /* fs11 */
    float f28;        /* ft8 */
    float f29;        /* ft9 */
    float f30;        /* ft10 */
    float f31;        /* ft11 */
#endif
    uint32_t mstatus; /* Machine status register */
    uint32_t mepc;
};

/**
 * @brief   Platform dependent part of the @p thread_t structure.
 * @details This structure usually contains just the saved stack pointer
 *          defined as a pointer to a @p port_intctx structure.
 */
struct port_context {
  struct port_intctx *sp;
};

#endif /* !defined(_FROM_ASM_) */

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Optimized thread function declaration macro.
 */
#define PORT_THD_FUNCTION(tname, arg) void tname(void *arg)

/**
 * @brief   Platform dependent part of the @p chThdCreateI() API.
 * @details This code usually setup the context switching frame represented
 *          by an @p port_intctx structure.
 */
#define PORT_SETUP_CONTEXT(tp, wbase, wtop, pf, arg)                                                                   \
    {                                                                                                                  \
        (tp)->ctx.sp = (struct port_intctx *)((uint8_t *)(wtop) - sizeof(struct port_intctx));                         \
        (tp)->ctx.sp->x1 = (uint32_t)_port_thread_start;                                                               \
        (tp)->ctx.sp->x8 = (uint32_t)(pf);                                                                             \
        (tp)->ctx.sp->x9 = (uint32_t)(arg);                                                                            \
    }

/**
 * @brief   Computes the thread working area global size.
 * @note    There is no need to perform alignments in this macro.
 */
#define PORT_WA_SIZE(n) (sizeof(struct port_intctx) +                       \
                         sizeof(struct port_extctx) +                       \
                         ((size_t)(n)) + ((size_t)(PORT_INT_REQUIRED_STACK)))

/**
 * @brief   Priority level verification macro.
 */
#define PORT_IRQ_IS_VALID_PRIORITY(n) (((n) > 0) && ((n) <= (PORT_LOWEST_PRIORITY - PORT_HIGHEST_PRIORITY)))

/**
 * @brief   Priority level verification macro.
 */
#define PORT_IRQ_IS_VALID_KERNEL_PRIORITY(n) PORT_IRQ_IS_VALID_PRIORITY(n)

/**
 * @brief   IRQ prologue code.
 * @details This macro must be inserted at the start of all IRQ handlers
 *          enabled to invoke system APIs.
 */
#define PORT_IRQ_PROLOGUE() _port_irq_prologue()

/**
 * @brief   IRQ epilogue code.
 * @details This macro must be inserted at the end of all IRQ handlers
 *          enabled to invoke system APIs.
 */
#define PORT_IRQ_EPILOGUE() _port_irq_epilogue()

/**
 * @brief   IRQ handler function declaration.
 * @note    @p id can be a function name or a vector number depending on the
 *          port implementation.
 */
#ifdef __cplusplus
#define PORT_IRQ_HANDLER(id) extern "C"  void id(void)
#else
#define PORT_IRQ_HANDLER(id)  void id(void)
#endif

/**
 * @brief   Fast IRQ handler function declaration.
 * @note    @p id can be a function name or a vector number depending on the
 *          port implementation.
 */
#ifdef __cplusplus
#define PORT_FAST_IRQ_HANDLER(id) extern "C" __attribute__((interrupt)) void id(void)
#else
#define PORT_FAST_IRQ_HANDLER(id) __attribute__((interrupt)) void id(void)
#endif

/**
 * @name    Context Save/Restore Macros
 * @{
 */
#define STORE_REGISTER(r, offset)                                                                                      \
    __asm volatile("sw "                                                                                                 \
                 "x" __CH_STRINGIFY(r) ","                                                                             \
                                       "%0(sp)" ::"i"(offset)                                                          \
                 : "memory");
#define LOAD_REGISTER(r, offset)                                                                                       \
    __asm volatile("lw "                                                                                                 \
                 "x" __CH_STRINGIFY(r) ","                                                                             \
                                       "%0(sp)" ::"i"(offset)                                                          \
                 : "x" __CH_STRINGIFY(r));
#define STORE_FLOAT_REGISTER(r, offset)                                                                                \
    __asm volatile("fsw "                                                                                                \
                 "f" __CH_STRINGIFY(r) ","                                                                             \
                                       "%0(sp)" ::"i"(offset)                                                          \
                 : "memory");
#define LOAD_FLOAT_REGISTER(r, offset)                                                                                 \
    __asm volatile("flw "                                                                                                \
                 "f" __CH_STRINGIFY(r) ","                                                                             \
                                       "%0(sp)" ::"i"(offset)                                                          \
                 : "f" __CH_STRINGIFY(r));

#define STORE_CSR(r, offset)                                                                                           \
    __asm volatile("csrr t0, " __CH_STRINGIFY(r));                                                                       \
    __asm volatile("sw t0, %0(sp)" ::"i"(offset) :);
#define LOAD_CSR(r, offset)                                                                                            \
    __asm volatile("lw t0, %0(sp)" ::"i"(offset) :);                                                                     \
    __asm volatile("csrw " __CH_STRINGIFY(r) ","                                                                         \
                                           "t0"                                                                        \
                 :);

#define RELOAD_SP(offset)                                                                                              \
    __asm volatile("addi "                                                                                               \
                 "sp, sp, %0" ::"i"(offset)                                                                            \
                 :);
/** @} */

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
#if !CH_DBG_ENABLE_STACK_CHECK || defined(__DOXYGEN__)
#define port_switch(ntp, otp) _port_switch(ntp, otp)
#else
#define port_switch(ntp, otp) {                                             \
  register struct port_intctx *sp asm ("sp");                              \
  if ((stkline_t *)(sp - 1) < otp->wabase)                                  \
    chSysHalt("stack overflow");                                            \
  _port_switch(ntp, otp);                                                   \
}
#endif

/**
 * @brief   Returns a word representing a critical section status.
 *
 * @return              The critical section status.
 */
#define port_get_lock_status() 0U

/**
 * @brief   Determines if in a critical section.
 *
 * @param[in] sts       status word returned by @p port_get_lock_status()
 * @return              The current status.
 * @retval false        if running outside a critical section.
 * @retval true         if running within a critical section.
 */
#define port_is_locked(sts) ((sts) != 0U)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

/* The following code is not processed when the file is included from an
   asm module.*/
#if !defined(_FROM_ASM_)

#ifdef __cplusplus
extern "C" {
#endif
#define port_init(x) _port_init();
  void _port_init(void);
  __attribute__((naked)) void _port_switch(thread_t *ntp, thread_t *otp);
  __attribute__((naked)) void _port_thread_start(void);
#ifdef __cplusplus
}
#endif

#endif /* !defined(_FROM_ASM_) */

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

/* The following code is not processed when the file is included from an
   asm module.*/
#if !defined(_FROM_ASM_)
#include "ch32h417.h"
/**
 * @brief   Determines the current execution context.
 *
 * @return              The execution context.
 * @retval false        not running in ISR mode.
 * @retval true         running in ISR mode.
 */
static inline bool port_is_isr_context(void) {
  for (size_t i = 0; i < PORT_INT_VECTOR_REG; i++)
  {
    if(NVIC->IACTR[i] != 0)
    {
      return true;
    }
  }
  return false;
}

/**
 * @brief   Kernel-lock action.
 * @details Usually this function just disables interrupts but may perform more
 *          actions.
 */
static inline void port_lock(void) {
  __disable_irq();
}

/**
 * @brief   Kernel-unlock action.
 * @details Usually this function just enables interrupts but may perform more
 *          actions.
 */
static inline void port_unlock(void) {
  __enable_irq();
}

/**
 * @brief   Kernel-lock action from an interrupt handler.
 * @details This function is invoked before invoking I-class APIs from
 *          interrupt handlers. The implementation is architecture dependent,
 *          in its simplest form it is void.
 */
static inline void port_lock_from_isr(void) {
  port_lock();
}

/**
 * @brief   Kernel-unlock action from an interrupt handler.
 * @details This function is invoked after invoking I-class APIs from interrupt
 *          handlers. The implementation is architecture dependent, in its
 *          simplest form it is void.
 */
static inline void port_unlock_from_isr(void) {
  port_unlock();
}

/**
 * @brief   Disables all the interrupt sources.
 * @note    Of course non-maskable interrupt sources are not included.
 */
static inline void port_disable(void) {
  __disable_irq();
}

/**
 * @brief   Disables the interrupt sources below kernel-level priority.
 * @note    Interrupt sources above kernel level remains enabled.
 */
static inline void port_suspend(void) {
  __disable_irq();
}

/**
 * @brief   Enables all the interrupt sources.
 */
static inline void port_enable(void) {
  __enable_irq();
}

/**
 * @brief   Enters an architecture-dependent IRQ-waiting mode.
 * @details The function is meant to return when an interrupt becomes pending.
 *          The simplest implementation is an empty function or macro but this
 *          would not take advantage of architecture-specific power saving
 *          modes.
 */
static inline void port_wait_for_interrupt(void) {

#if PORT_ENABLE_WFI_IDLE
  // __WFI();
#endif
}

/**
 * @brief   Returns the current value of the realtime counter.
 *
 * @return              The realtime counter value.
 */
static inline rtcnt_t port_rt_get_counter_value(void) {

  return 0;
}

static __attribute__((always_inline)) inline void save_context(void)
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

static __attribute__((always_inline)) inline void recover_context(void)
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

static __attribute__((always_inline)) inline void save_context_irq(void)
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

static __attribute__((always_inline)) inline void recover_context_irq(void)
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

static __attribute__((always_inline)) inline void _port_irq_prologue(void)
{
    save_context_irq();
}

/* Forward declarations for kernel functions used by _port_irq_epilogue.
   These are declared in chschd.h, included later via ch.h -> chsys.h -> chschd.h.
   Must use C linkage since they are defined in C code. */
#ifdef __cplusplus
extern "C" {
#endif
extern bool chSchIsPreemptionRequired(void);
extern void chSchDoPreemption(void);
#ifdef __cplusplus
}
#endif

static __attribute__((always_inline)) inline void _port_irq_epilogue(void)
{

    if (chSchIsPreemptionRequired())
    {
        port_lock();
        chSchDoPreemption();
        port_unlock();
    }
    recover_context_irq();

    __asm volatile("mret");
}

#endif /* !defined(_FROM_ASM_) */

/*===========================================================================*/
/* Module late inclusions.                                                   */
/*===========================================================================*/

#if !defined(_FROM_ASM_)

#if CH_CFG_ST_TIMEDELTA > 0
#include "chcore_timer.h"
#endif /* CH_CFG_ST_TIMEDELTA > 0 */

#endif /* !defined(_FROM_ASM_) */

#endif /* CHCORE_H */

/** @} */
