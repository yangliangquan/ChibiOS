/*
    ChibiOS - Copyright (C) 2020 Patrick Seidel
    ChibiOS - Copyright (C) 2021 Stefan Kerkmann

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
 * @file    crt1.c
 * @brief   Startup stub functions.
 *
 * @addtogroup RISCV_ECLIC_STARTUP
 * @{
 */

#include <stdbool.h>
#include <stdint.h>

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Architecture-dependent core initialization.
 * @details This hook is invoked immediately after the stack initialization
 *          and before the DATA and BSS segments initialization.
 * @note    This function is a weak symbol.
 */
#if !defined(__DOXYGEN__)
__attribute__((weak, section(".handle_reset"),optimize("O0")))
#endif
/*lint -save -e9075 [8.4] All symbols are invoked from asm context.*/
void __core_init(void) {}

/**
 * @brief   Early initialization.
 * @details This hook is invoked immediately after the stack initialization
 *          and before the DATA and BSS segments initialization. The
 *          default behavior is to do nothing.
 * @note    This function is a weak symbol.
 */
#if !defined(__DOXYGEN__)
__attribute__((weak, section(".handle_reset"),optimize("O0")))
#endif
/*lint -save -e9075 [8.4] All symbols are invoked from asm context.*/
void __early_init(void) {}
/*lint -restore*/

/**
 * @brief   Late initialization.
 * @details This hook is invoked after the DATA and BSS segments
 *          initialization and before any static constructor. The
 *          default behavior is to do nothing.
 * @note    This function is a weak symbol.
 */
#if !defined(__DOXYGEN__)
__attribute__((weak, section(".handle_reset"),optimize("O0")))
#endif
/*lint -save -e9075 [8.4] All symbols are invoked from asm context.*/
void __late_init(void) {}
/*lint -restore*/

/**
 * @brief   Default @p main() function exit handler.
 * @details This handler is invoked or the @p main() function exit. The
 *          default behavior is to enter an infinite loop.
 * @note    This function is a weak symbol.
 */
#if !defined(__DOXYGEN__)
__attribute__((noreturn, weak, section(".handle_reset"),optimize("O0")))
#endif
/*lint -save -e9075 [8.4] All symbols are invoked from asm context.*/
void __default_exit(void) {
/*lint -restore*/
  while (true) {
  }
}

/**
 * @brief   Performs the initialization of the various RAM areas.
 */
__attribute__((weak, section(".handle_reset"),optimize("O0")))
void __init_ram_areas(void) {
}

__attribute__((weak, section(".handle_reset"),optimize("O0")))
void call_constructors(){
    extern uint32_t __init_array_start[];
    extern uint32_t __init_array_end[];

    for(uint32_t *p = __init_array_start; p < __init_array_end; p++){
        ((void (*)(void))(*p))();
    }
}

__attribute__((weak, section(".handle_reset"),optimize("O0")))
void call_destructors(){
    extern uint32_t __fini_array_start[];
    extern uint32_t __fini_array_end[];

    for(uint32_t *p = __fini_array_start; p < __fini_array_end; p++){
        ((void (*)(void))(*p))();
    }
}

__attribute__((weak, section(".handle_reset"),optimize("O0")))
void clearbss(){
    extern uint32_t _sbss[];
    extern uint32_t _ebss[];
    uint32_t *p = _sbss;
    while(p < _ebss){
        *p = 0;
        p++;
    }
}

__attribute__((weak, section(".handle_reset"),optimize("O0")))
void loaddata2vma(){
    extern uint32_t _data_lma[];
    extern uint32_t _data_vma[];
    extern uint32_t _edata[];

    uint32_t *p = _data_lma;
    uint32_t *q = _data_vma;
    while(q < _edata){
        *q = *p;
        p++;
        q++;
    }
}

__attribute__((weak, section(".handle_reset"),optimize("O0")))
void loadcode2vma(){
    extern uint32_t  _highcode_lma[];
    extern uint32_t  _highcode_vma_start[];
    extern uint32_t  _highcode_vma_end[];

    uint32_t *p = _highcode_lma;
    uint32_t *q = _highcode_vma_start;
    while(q < _highcode_vma_end){
        *q = *p;
        p++;
        q++;
    }

    extern uint32_t _highcode_lma1[];
    extern uint32_t _highcode_vma_start1[];
    extern uint32_t _highcode_vma_end1[];

    p = _highcode_lma1;
    q = _highcode_vma_start1;
    while(q < _highcode_vma_end1){
        *q = *p;
        p++;
        q++;
    }
}

/**
 * @brief   Stack segments initialization switch.
 */
#if !defined(CRT0_STACKS_FILL_PATTERN) || defined(__DOXYGEN__)
#define CRT0_STACKS_FILL_PATTERN            0x55555555
#endif

__attribute__((weak, section(".handle_reset"),optimize("O0")))
void colorize_stack(){
    extern uint32_t __process_stack_base__[];
    extern uint32_t __process_stack_end__[];

    uint32_t *p = __process_stack_base__;
    uint32_t *q = __process_stack_end__;
    while(p < q){
        *p = CRT0_STACKS_FILL_PATTERN;
        p++;
    }
}

/** @} */
