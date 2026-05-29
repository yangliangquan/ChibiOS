/********************************** (C) COPYRIGHT *******************************
* File Name          : core_riscv_v5f.c
* Description        : SKELETON - RISC-V V5F Core Peripheral Access Layer Source
********************************************************************************/
/* TODO: Implement V5F core peripheral access functions */
#include <stdint.h>

volatile uint32_t WFE_MASK = 0;

uint32_t __get_MSTATUS(void) { return 0; }
void __set_MSTATUS(uint32_t value) { (void)value; }
uint32_t __get_MEPC(void) { return 0; }
void __set_MEPC(uint32_t value) { (void)value; }
uint32_t __get_MCAUSE(void) { return 0; }
void __set_MCAUSE(uint32_t value) { (void)value; }
uint32_t __get_MTVEC(void) { return 0; }
void __set_MTVEC(uint32_t value) { (void)value; }
uint32_t __get_MHARTID(void) { return 0; }
uint32_t __get_SP(void) { return 0; }
void __enable_irq(void) {}
void __disable_irq(void) {}
