/********************************** (C) COPYRIGHT *******************************
* File Name          : crt0.S
* Description        : SKELETON - CH32H417 V5F startup file.
*                      Implement V5F-specific reset sequence here.
********************************************************************************/
/* TODO: Implement V5F crt0.S */
/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

#include "riscvparams.h"

#if !defined(FALSE) || defined(__DOXYGEN__)
#define FALSE                               0
#endif

#if !defined(TRUE) || defined(__DOXYGEN__)
#define TRUE                                1
#endif

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Core initialization switch.
 */
#if !defined(CRT0_INIT_CORE) || defined(__DOXYGEN__)
#define CRT0_INIT_CORE                      TRUE
#endif

/**
 * @brief   Stack segments initialization switch.
 */
#if !defined(CRT0_STACKS_FILL_PATTERN) || defined(__DOXYGEN__)
#define CRT0_STACKS_FILL_PATTERN            0x55555555
#endif

/**
 * @brief   Stack segments initialization switch.
 */
#if !defined(CRT0_INIT_STACKS) || defined(__DOXYGEN__)
#define CRT0_INIT_STACKS                    TRUE
#endif

/**
 * @brief   DATA segment initialization switch.
 */
#if !defined(CRT0_INIT_DATA) || defined(__DOXYGEN__)
#define CRT0_INIT_DATA                      TRUE
#endif

/**
 * @brief   BSS segment initialization switch.
 */
#if !defined(CRT0_INIT_BSS) || defined(__DOXYGEN__)
#define CRT0_INIT_BSS                       TRUE
#endif

/**
 * @brief   RAM areas initialization switch.
 */
#if !defined(CRT0_INIT_RAM_AREAS) || defined(__DOXYGEN__)
#define CRT0_INIT_RAM_AREAS                 TRUE
#endif

/**
 * @brief   Constructors invocation switch.
 */
#if !defined(CRT0_CALL_CONSTRUCTORS) || defined(__DOXYGEN__)
#define CRT0_CALL_CONSTRUCTORS              TRUE
#endif

/**
 * @brief   Destructors invocation switch.
 */
#if !defined(CRT0_CALL_DESTRUCTORS) || defined(__DOXYGEN__)
#define CRT0_CALL_DESTRUCTORS               TRUE
#endif

	.section	.init,"ax",@progbits
	.global	_start
	.align	2
_start:
    li t0, 0x88
    csrc 0x800, t0
    fence.i

#ifdef HARDWARE_STACK
    li t0, 0x07
    csrw 0x804, t0
    fence.i
#endif
    
    la a0, _loadcode_lma
    la a1, _loadcode_vma_start
    la a2, _loadcode_vma_end
    bgeu a1, a2, 2f

1:
    lw t0, (a0)
    sw t0, (a1)
    addi a0, a0, 4
    addi a1, a1, 4
    bltu a1, a2, 1b
2:

    la t0, handle_reset
    jr t0

	.section	.handle_reset,"ax",@progbits
	.weak	handle_reset
	.align	1
    
swicth_clock_pll70m:

    lui  s1 ,0x40021
    li  a1, 0x40E 
    sw a1 ,8(s1)

    /*enable pll*/
    lw t0,0(s1)
    li t1,0x1000000
    or t0,t0,t1
    sw t0,0(s1)

1:
    lw t2,0(s1)   
    li t3,0x2000000
    and t2,t2,t3
    beqz t2 ,1b

    /*enable pll gate*/
    lw t4,8(s1)
    li t5,0x80000000
    or t4,t4,t5
    sw t4,8(s1)

    /*choose pll */
    li  t6, 0x2
    sw t6 ,4(s1)

1:
    lw s2, 4(s1)
    li s3 ,0x8
    and s2,s2,s3
    beqz s2, 1b
    ret


.section	.handle_reset,"ax",@progbits
.option push
handle_reset: 
.option	norelax 
	la gp, __global_pointer$
.option	pop 

    la sp, _eusrstack 

    /* Configure Prefetch */
    li t0, 0x123703E1
    csrw 0xBC0, t0
    /* Enable floating point and global interrupt, configure privileged mode */
    li t0, 0x7888           
   	csrw mstatus, t0
    /* Configure the interrupt vector table recognition mode and entry address mode */
    .extern _port_irq_handler
    la t0, _port_irq_handler
    # ori t0, t0, 3           
	csrw mtvec, t0

	call swicth_clock_pll70m
    call loadcode2vma
#if CRT0_INIT_CORE == TRUE
    call __core_init
#endif

    call __early_init
#if CRT0_INIT_STACKS
    call colorize_stack
#endif

2:  
#if CRT0_INIT_DATA
    call loaddata2vma
#endif

#if CRT0_INIT_BSS
    call clearbss
#endif

#if CRT0_CALL_CONSTRUCTORS
    call call_constructors
#endif

#if CRT0_INIT_RAM_AREAS == TRUE
    /* RAM areas initialization.*/
    call __init_ram_areas
#endif

    call __late_init

la t0, _main
	csrw mepc, t0
	mret

_main:
    call main
#if CRT0_CALL_DESTRUCTORS
    call call_destructors
#endif

