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
 * @brief   CH32H417 Core1 (V5F) bare-metal firmware.
 *
 * @details This is a minimal standalone firmware for the secondary core.
 *          No RTOS or HAL is used - the core runs a simple GPIO toggle loop.
 *
 *          When the V5F ChibiOS port is completed, this can be upgraded to
 *          a full RTOS application. For now, it demonstrates that Core1
 *          starts and executes code independently.
 */

#include "ch.h"
#include "hal.h"

/*===========================================================================*/
/* Main entry point.                                                         */
/*===========================================================================*/

int main(void)
{
    halInit();
    chSysInit();

    /* Main thread: idle loop. */
    while (true)
    {
        chThdSleepMilliseconds(500);
    }
}
