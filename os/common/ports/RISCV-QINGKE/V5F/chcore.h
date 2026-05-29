/*
    ChibiOS - Copyright (C) 2006-2026 Giovanni Di Sirio.

    This file is part of ChibiOS.
    ...

/**
 * @file    V5F/chcore_v5f.h
 * @brief   SKELETON - V5F port related macros and structures.
 *
 * @addtogroup port_core
 * @{
 */

#ifndef CHCORE_V5F_H
#define CHCORE_V5F_H

/* TODO: Include V5F-specific parameters */
#include "riscvparams_v5f.h"

#define PORT_SUPPORTS_RT                TRUE
#define PORT_NATURAL_ALIGN              sizeof(void *)
#define PORT_STACK_ALIGN                4U
#define PORT_WORKING_AREA_ALIGN         4U
#define PORT_LOWEST_PRIORITY            0
#define PORT_HIGHEST_PRIORITY           1
#define PORT_ARCHITECTURE_RISCV
#define PORT_ARCHITECTURE_RISCV_QINGKE
#define PORT_ARCHITECTURE_NAME          "RISC-V"
#define PORT_CORE_VARIANT_NAME          "RV32IMAFDC"
#define PORT_INFO                       "RV32IMAFDC - QingKe V5F"

#if !defined(PORT_IDLE_THREAD_STACK_SIZE)
#define PORT_IDLE_THREAD_STACK_SIZE     64
#endif
#if !defined(PORT_INT_REQUIRED_STACK)
#define PORT_INT_REQUIRED_STACK         256
#endif
#if !defined(PORT_ENABLE_WFI_IDLE)
#define PORT_ENABLE_WFI_IDLE            FALSE
#endif

#if !defined(_FROM_ASM_)
#include "chtypes_v5f.h"
struct port_extctx {
  /* TODO: Define V5F interrupt context structure */
  uint32_t dummy;
};
struct port_intctx {
  /* TODO: Define V5F system context structure */
  uint32_t dummy;
  uint32_t mstatus;
  uint32_t mepc;
};
struct port_context {
  struct port_intctx *sp;
};
#endif /* !defined(_FROM_ASM_) */

#define PORT_THD_FUNCTION(tname, arg) void tname(void *arg)
#define PORT_SETUP_CONTEXT(tp, wbase, wtop, pf, arg)
#define PORT_WA_SIZE(n) (sizeof(struct port_intctx) + sizeof(struct port_extctx) + ((size_t)(n)) + ((size_t)(PORT_INT_REQUIRED_STACK)))

#define PORT_IRQ_PROLOGUE()
#define PORT_IRQ_EPILOGUE()
#define PORT_IRQ_HANDLER(id) void id(void)
#define PORT_FAST_IRQ_HANDLER(id) __attribute__((interrupt)) void id(void)
#define port_switch(ntp, otp) _port_switch(ntp, otp)

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
void _port_init(void);
__attribute__((naked)) void _port_switch(thread_t *ntp, thread_t *otp);
__attribute__((naked)) void _port_thread_start(void);
#ifdef __cplusplus
}
#endif

static inline bool port_is_isr_context(void) { return false; }
static inline void port_lock(void) {}
static inline void port_unlock(void) {}
static inline void port_lock_from_isr(void) {}
static inline void port_unlock_from_isr(void) {}
static inline void port_disable(void) {}
static inline void port_suspend(void) {}
static inline void port_enable(void) {}
static inline void port_wait_for_interrupt(void) {}
static inline rtcnt_t port_rt_get_counter_value(void) { return 0; }

#endif /* !defined(_FROM_ASM_) */

#endif /* CHCORE_V5F_H */

/** @} */
