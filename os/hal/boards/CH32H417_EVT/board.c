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

#include "hal.h"

const PALConfig pal_default_config =
{
    {
#if (CH32_HAS_GPIOA == TRUE)
  {0x00000000, 0x44444444, 0x44444444},
#endif
#if (CH32_HAS_GPIOB == TRUE)
  {0x00000000, 0x44444444, 0x44444444},
#endif
#if (CH32_HAS_GPIOC == TRUE)
  {0x00000000, 0x44444444, 0x44444444},
#endif
#if (CH32_HAS_GPIOD == TRUE)
  {0x00000000, 0x44444444, 0x44444444},
#endif
#if (CH32_HAS_GPIOE == TRUE)
  {0x00000000, 0x44444444, 0x44444444},
#endif
#if (CH32_HAS_GPIOF == TRUE)
  {0x00000000, 0x44444444, 0x44444444},
#endif
    }
};

void __early_init(void) {
    
}

/**
 * @brief   Board-specific initialization code.
 * @note    You can add your board-specific code here.
 */
void boardInit(void) {
}

/**
 * @brief   Card insertion detection.
 * @note    The CH32H417 EVT board may not have a dedicated card detect
 *          pin. Override this function if card detection is available.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @return              The operation status.
 * @retval TRUE         card is inserted.
 * @retval FALSE        card is not inserted.
 */
bool sdc_lld_is_card_inserted(SDCDriver *sdcp) {

  (void)sdcp;

  /* Return TRUE to always indicate card present.
     Override with proper GPIO detection if available.*/
  return true;
}

/**
 * @brief   Write protection detection.
 * @note    The CH32H417 EVT board does not have a write protect switch.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @return              The operation status.
 * @retval TRUE         card is write-protected.
 * @retval FALSE        card is not write-protected.
 */
bool sdc_lld_is_write_protected(SDCDriver *sdcp) {

  (void)sdcp;

  /* Return FALSE to indicate no write protection.*/
  return false;
}
