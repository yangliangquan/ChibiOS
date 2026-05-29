/*
    ChibiOS - Copyright (C) 2006-2026 Giovanni Di Sirio.
    ...

/**
 * @file    V5F/chtypes_v5f.h
 * @brief   SKELETON - V5F port system types.
 *
 * @addtogroup port_types
 * @{
 */

#ifndef CHTYPES_V5F_H
#define CHTYPES_V5F_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "ccportab.h"

#define PORT_ARCH_SIZEOF_DATA_PTR   4
#define PORT_ARCH_SIZEOF_CODE_PTR   4
#define PORT_ARCH_REGISTERS_WIDTH   32
#define PORT_ARCH_REVERSE_ORDER     1

typedef uint32_t            port_rtcnt_t;
typedef uint64_t            port_rttime_t;
typedef uint32_t            port_syssts_t;
typedef uint64_t            port_stkline_t;

#define PORT_DOES_NOT_PROVIDE_TYPES
#define ROMCONST            CC_ROMCONST
#define NOINLINE            CC_NO_INLINE
#define ALIGNED_VAR(n)      CC_ALIGN_DATA(n)
#define SIZEOF_PTR          PORT_ARCH_SIZEOF_DATA_PTR

#endif /* CHTYPES_V5F_H */

/** @} */
