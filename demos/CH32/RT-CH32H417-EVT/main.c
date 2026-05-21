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

#include "ch.h"
#include "hal.h"
#include "usbcfg.h"
#include "chprintf.h"

static const ADCConversionGroup adcgrpcfg = {
  .circular = false,
  .num_channels = 2,
  .end_cb = NULL,
  .error_cb = NULL,
  {ADC_Channel_1, ADC_Channel_2},
  {ADC_SampleTime_CyclesMode7, ADC_SampleTime_CyclesMode7},
  .adcmode = ADC_Mode_Independent,
  .adcoutputbuffer = ADC_OutputBuffer_Disable,
  .adcpga = ADC_Pga_1,
  .adcdataalign = ADC_DataAlign_Right,
  .adcscanmode = true, 
  .adccont = true,              // ← 启用连续转换模式，让ADC自动重复采样
  .adcexttrig = ADC_ExternalTrigConv_None
};
static const ADCConfig adc_default_config = {
  .dmacfg = DMA_DIR_PeripheralSRC | DMA_MemoryInc_Enable | DMA_PeripheralInc_Disable | (adcgrpcfg.circular ? DMA_Mode_Circular : DMA_Mode_Normal) |
                                     DMA_PeripheralDataSize_HalfWord | DMA_MemoryDataSize_HalfWord
};

adcsample_t sample_buffer[16];

static THD_WORKING_AREA(waADCThread, 1024);
static THD_FUNCTION(ADCThread, arg){
    (void)arg;
    chRegSetThreadName("ADCThread");
    pal_lld_setpadmode(GPIOA, GPIO_PIN0, PAL_MODE_INPUT_ANALOG);
    pal_lld_setpadmode(GPIOA, GPIO_PIN1, PAL_MODE_INPUT_ANALOG);

    adcStart(&ADCD1, &adc_default_config);
    
    while(true){
        adcConvert(&ADCD1, &adcgrpcfg, sample_buffer, 8);
        chprintf((BaseSequentialStream *)&SDU1, "%s: ADC: %u %u\r\n", chRegGetThreadNameX(chThdGetSelfX()), sample_buffer[0], sample_buffer[1]);
        chThdSleepMilliseconds(1000);
    }
}

/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */
static THD_WORKING_AREA(waThread1, 1024);
static THD_FUNCTION(Thread1, arg)
{

    (void)arg;
    chRegSetThreadName("blinker");
    palSetPadMode(GPIOD, GPIO_PIN4, PAL_MODE_OUTPUT_PUSHPULL);
    while (true)
    {
        palSetPad(GPIOD, GPIO_PIN4); /* Orange.  */
        chThdSleepMilliseconds(500/2);
        palClearPad(GPIOD, GPIO_PIN4); /* Orange.  */
        chThdSleepMilliseconds(500/2);
    }
}

/*
 * Application entry point.
 */
int main(void)
{

    /*
     * System initializations.
     * - HAL initialization, this also initializes the configured device drivers
     *   and performs the board-specific initializations.
     * - Kernel initialization, the main() function becomes a thread and the
     *   RTOS is active.
     */
    halInit();
    chSysInit();

    /*
     * Initializes a serial-over-USB CDC driver.
     */
    sduObjectInit(&SDU1);
    sduStart(&SDU1, &serusbcfg);

    /*
     * Activates the USB driver and then the USB bus pull-up on D+.
     * Note, a delay is inserted in order to not have to disconnect the cable
     * after a reset.
     */
    usbDisconnectBus(serusbcfg.usbp);
    chThdSleepMilliseconds(1500);
    usbStart(serusbcfg.usbp, &usbcfg);
    usbConnectBus(serusbcfg.usbp);

    /*
     * Creates the example thread.
     */
    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
    chThdCreateStatic(waADCThread, sizeof(waADCThread), NORMALPRIO, ADCThread, NULL);

    /*
     * Normal main() thread activity, in this demo it does nothing except
     * sleeping in a loop and check the button state.
     */
    while (true)
    {
        chprintf((BaseSequentialStream *)&SDU1, "%s: Hello\r\n", chRegGetThreadNameX(chThdGetSelfX()));
        chThdSleepMilliseconds(5000);
    }
}
