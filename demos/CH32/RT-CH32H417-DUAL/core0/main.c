/*demos/CH32/RT-CH32H417-EVT/Makefile/*
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

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

/*===========================================================================*/
/* LED blinker thread.                                                       */
/*===========================================================================*/

static THD_WORKING_AREA(waBlinkerThread, 256);
static THD_FUNCTION(BlinkerThread, arg)
{
    (void)arg;
    chRegSetThreadName("blinker");

    /* GPIOD PIN4 is the onboard LED on CH32H417-EVT. */
    palSetPadMode(GPIOD, GPIO_PIN4, PAL_MODE_OUTPUT_PUSHPULL);

    while (true)
    {
        palSetPad(GPIOD, GPIO_PIN4);
        chThdSleepMilliseconds(500);
        palClearPad(GPIOD, GPIO_PIN4);
        chThdSleepMilliseconds(500);
    }
}

/*===========================================================================*/
/* Serial output thread (via SD2 / USART2).                                  */
/*===========================================================================*/

static const SerialConfig serial_cfg = {
  .baud = 115200,
  .cr1   = USART_CTLR1_DATA8,
  .cr2   = USART_CTLR2_STOP1_BITS,
  .cr3   = 0U
};

static THD_WORKING_AREA(waSerialThread, 512);
static THD_FUNCTION(SerialThread, arg)
{
    (void)arg;
    chRegSetThreadName("serial");

    /* USART2 TX=PA2, RX=PA3, AF7. */
    palSetPadMode(GPIOA, GPIO_PIN2, PAL_CH32_ALTERNATE_PUSHPULL(7));
    palSetPadMode(GPIOA, GPIO_PIN3, PAL_CH32_ALTERNATE_INPUT(7));

    sdStart(&SD2, &serial_cfg);

    while (true)
    {
        chprintf((BaseSequentialStream *)&SD2,
                 "Core0 (V3F): Hello from ChibiOS/RT!\r\n");
        chThdSleepMilliseconds(1000);
    }
}

/*===========================================================================*/
/* SIO output thread (via USART1).                                           */
/*===========================================================================*/

static const SIOConfig sio_cfg = {
  .baud  = 115200,
  .cr1   = USART_CTLR1_DATA8,
  .cr2   = USART_CTLR2_STOP1_BITS,
  .cr3   = 0U
};

static THD_WORKING_AREA(waSIOThread, 256);
static THD_FUNCTION(SIOThread, arg)
{
    (void)arg;
    chRegSetThreadName("sio");

    /* USART1 TX=PA9, AF7. */
    palSetPadMode(GPIOA, GPIO_PIN9, PAL_CH32_ALTERNATE_PUSHPULL(7));
    sioStart(&SIOD1, &sio_cfg);

    while (true)
    {
        sioAsyncWrite(&SIOD1,
                      (const uint8_t *)"Core0 (V3F): Hello from SIO!\r\n", 33);
        chThdSleepMilliseconds(2000);
    }
}

/*===========================================================================*/
/* GPT periodic callback - toggles an extra pin for scope measurement.       */
/*===========================================================================*/

static void gpt_cb(GPTDriver *gptp)
{
    (void)gptp;
    palTogglePad(GPIOD, GPIO_PIN5);
}

static const GPTConfig gpt_default_config = {
  .frequency = 10000,
  .callback  = gpt_cb
};

static THD_WORKING_AREA(waGPTThread, 256);
static THD_FUNCTION(GPTThread, arg)
{
    (void)arg;
    chRegSetThreadName("gpt");

    palSetPadMode(GPIOD, GPIO_PIN5, PAL_MODE_OUTPUT_PUSHPULL);
    gptStart(&GPTD2, &gpt_default_config);
    gptStartContinuous(&GPTD2, 10000); /* 1 Hz callback. */

    while (true)
    {
        chThdSleepMilliseconds(1000);
    }
}

/*===========================================================================*/
/* Application entry point.                                                  */
/*===========================================================================*/

int main(void)
{
    halInit();
    chSysInit();

    chThdCreateStatic(waBlinkerThread, sizeof(waBlinkerThread),
                      NORMALPRIO, BlinkerThread, NULL);
    chThdCreateStatic(waSerialThread, sizeof(waSerialThread),
                      NORMALPRIO, SerialThread, NULL);
    chThdCreateStatic(waSIOThread, sizeof(waSIOThread),
                      NORMALPRIO, SIOThread, NULL);
    chThdCreateStatic(waGPTThread, sizeof(waGPTThread),
                      NORMALPRIO, GPTThread, NULL);

    /* Main thread: idle loop. */
    while (true)
    {
        chThdSleepMilliseconds(500);
    }
}
