/********************************** (C) COPYRIGHT *******************************
* File Name          : core_riscv_v5f.h
* Description        : SKELETON - RISC-V V5F Core Peripheral Access Layer Header
********************************************************************************/
#ifndef __CORE_RISCV_V5F_H__
#define __CORE_RISCV_V5F_H__

#ifdef __cplusplus
extern "C" {
#endif

/* TODO: Implement V5F-specific peripheral definitions:
 *   - PFIC register map (may differ from V3F)
 *   - SysTick register map
 *   - HSEM, IPC structures
 *   - Core ID definitions
 *   - IRQ control functions
 *   - Atomic operation functions
 */

#define Core_ID_V3F   ((uint8_t)0x00)
#define Core_ID_V5F   ((uint8_t)0x01)

extern volatile uint32_t WFE_MASK;

uint32_t __get_MSTATUS(void);
void __set_MSTATUS(uint32_t value);
uint32_t __get_MEPC(void);
void __set_MEPC(uint32_t value);
uint32_t __get_MCAUSE(void);
void __set_MCAUSE(uint32_t value);
uint32_t __get_MTVEC(void);
void __set_MTVEC(uint32_t value);
uint32_t __get_MHARTID(void);
uint32_t __get_SP(void);
void __enable_irq(void);
void __disable_irq(void);

#ifdef __cplusplus
}
#endif

#endif
