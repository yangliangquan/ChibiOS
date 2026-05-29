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

static const SIOConfig sio_cfg = {
  .baud  = 115200,
  .cr1   = USART_CTLR1_DATA8,
  .cr2   = USART_CTLR2_STOP1_BITS,
  .cr3   = 0U
};
static THD_WORKING_AREA(waSIOThread, 1024);
static THD_FUNCTION(SIOThread, arg){
    (void)arg;
    chRegSetThreadName("SIOThread");
    palSetPadMode(GPIOA, GPIO_PIN9, PAL_CH32_ALTERNATE_PUSHPULL(7));
    sioStart(&SIOD1, &sio_cfg);
    while(true){
        sioAsyncWrite(&SIOD1, (const uint8_t *)"Hello from SIOThread!\r\n", 24);
        chThdSleepMilliseconds(500);
    }
}

static const ICUConfig icucfg = {
  .frequency = 100000,
  .arr = 0,
  .mode = ICU_INPUT_ACTIVE_HIGH,
  .width_cb = NULL,
  .period_cb = NULL,
  .overflow_cb = NULL,
  .dma_settings = 0
};

static THD_WORKING_AREA(waICUThread, 1024);
static THD_FUNCTION(ICUThread, arg){
    (void)arg;
    chRegSetThreadName("ICUThread");
    palSetPadMode(GPIOB, GPIO_PIN6, PAL_CH32_ALTERNATE_INPUT(2));
    icuStart(&ICUD4, &icucfg);
    icuStartCapture(&ICUD4);
    while(true){
        chThdSleepMilliseconds(500);
    }
}

static const PWMConfig pwmgrpcfg = {
  .frequency = 10000,
  .period = 1000,
  .callback = NULL,
  .channels = {
    {.mode = PWM_OUTPUT_ACTIVE_HIGH, .callback = NULL},
  }
};

static THD_WORKING_AREA(waPWMThread, 1024);
static THD_FUNCTION(PWMThread, arg){
    (void)arg;
    chRegSetThreadName("PWMThread");
    palSetPadMode(GPIOA, GPIO_PIN6, PAL_CH32_ALTERNATE_PUSHPULL(2));
    pwmStart(&PWMD3, &pwmgrpcfg);
    static uint16_t duty = 0;
    while(true){

        chThdSleepMilliseconds(500);
        pwmEnableChannel(&PWMD3, 0, duty++);
    }
}

void gpt_default_config_cb(GPTDriver *gptp){
    (void)gptp;
    palTogglePad(GPIOD, GPIO_PIN5);
}
static const GPTConfig gpt_default_config = {
  .frequency = 10000,
  .callback = gpt_default_config_cb
};
static THD_WORKING_AREA(waGPTThread, 1024);
static THD_FUNCTION(GPTThread, arg){
    (void)arg;
    palSetPadMode(GPIOD, GPIO_PIN5, PAL_MODE_OUTPUT_PUSHPULL);
    chRegSetThreadName("GPTThread");
    gptStart(&GPTD2, &gpt_default_config);
    gptStartContinuous(&GPTD2, 10000);
    while(true){
        chThdSleepMilliseconds(1000);
    }
}

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
  .adccont = true,
  .adcexttrig = ADC_ExternalTrigConv_None
};
static const ADCConfig adc_default_config = {
  .dmacfg = DMA_DIR_PeripheralSRC | DMA_MemoryInc_Enable | DMA_PeripheralInc_Disable | (adcgrpcfg.circular ? DMA_Mode_Circular : DMA_Mode_Normal) |
                                     DMA_PeripheralDataSize_HalfWord | DMA_MemoryDataSize_HalfWord|(DMA_CFGR1_TCIE | DMA_CFGR1_HTIE | DMA_CFGR1_TEIE)
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
        // chprintf((BaseSequentialStream *)&SDU1, "%s: ADC: %u %u\r\n", chRegGetThreadNameX(chThdGetSelfX()), sample_buffer[0], sample_buffer[1]);
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

#include "shell.h"
#include "hal_uart.h"

/*===========================================================================*/
/* Serial driver related (SD driver).                                        */
/*===========================================================================*/
static const SerialConfig serial_cfg = {
  .baud = 115200,
  .cr1   = USART_CTLR1_DATA8,
  .cr2   = USART_CTLR2_STOP1_BITS,
  .cr3   = 0U
};

static THD_WORKING_AREA(waSerialThread, 1024);
static THD_FUNCTION(SerialThread, arg) {
  (void)arg;
  chRegSetThreadName("SerialThread");

  /* Configure USART2 TX (PA2) and RX (PA3), AF7 for USART2 on CH32H417.*/
  palSetPadMode(GPIOA, GPIO_PIN2, PAL_CH32_ALTERNATE_PUSHPULL(7));
  palSetPadMode(GPIOA, GPIO_PIN3, PAL_CH32_ALTERNATE_INPUT(7));

  /* Start Serial driver on USART2.*/
  sdStart(&SD2, &serial_cfg);

  /* Use SD2 as a BaseSequentialStream for chprintf.*/
  BaseSequentialStream *chp = (BaseSequentialStream *)&SD2;

  while (true) {
    chprintf(chp, "Hello from SerialThread! (SD2 USART2)\r\n");
    chThdSleepMilliseconds(1000);
  }
}

/*===========================================================================*/
/* UART driver related.                                                      */
/*===========================================================================*/

/* UART test statistics */
static uint32_t uart_tx_count = 0;
static uint32_t uart_rx_count = 0;
static uint32_t uart_errors = 0;

static uint8_t uart_tx_buf[] = "Hello from UART3 (CH32H417)!\r\n";
static uint8_t uart_rx_buf[128];

/*
 * This callback is invoked when a transmission buffer has been completely
 * read by the driver.
 */
static void uart_txend1_cb(UARTDriver *uartp) {
  (void)uartp;
  uart_tx_count++;
//   palTogglePad(GPIOD, GPIO_PIN5);  /* Toggle LED on TX complete */
}

/*
 * This callback is invoked when a transmission has physically completed.
 */
static void uart_txend2_cb(UARTDriver *uartp) {
  (void)uartp;
  /* Transmission physically completed.*/
}

/*
 * This callback is invoked when a receive buffer has been completely written.
 */
static void uart_rxend_cb(UARTDriver *uartp) {
  (void)uartp;
  uart_rx_count++;
  /* Restart reception */
  if (uartp == &UARTD3) {
    chSysLockFromISR();
    uartStopReceiveI(&UARTD3);
    chSysUnlockFromISR();
  }
}
uint32_t rx_index = 0;
/*
 * This callback is invoked when a character is received but the application
 * was not ready to receive it, the character is passed as parameter.
 */
static void uart_rxchar_cb(UARTDriver *uartp, uint16_t c) {
  (void)uartp;
  (void)c;
if (uartp == &UARTD3) {
    rx_index++;
  }
  /* Character received while out of UART_RECEIVE state.*/
}

/*
 * This callback is invoked on a receive error, the errors mask is passed
 * as parameter.
 */
static void uart_rxerr_cb(UARTDriver *uartp, uartflags_t e) {
  (void)uartp;
  (void)e;
  uart_errors++;
}

/*
 * This callback is invoked when configured timeout reached.
 */
static void uart_timeout_cb(UARTDriver *uartp) {
  (void)uartp;
  /* Receiver timeout detected.*/
}

/*
 * UART driver configuration structure.
 */
static const UARTConfig uart_cfg = {
  .txend1_cb  = uart_txend1_cb,
  .txend2_cb  = uart_txend2_cb,
  .rxend_cb   = uart_rxend_cb,
  .rxchar_cb  = uart_rxchar_cb,
  .rxerr_cb   = uart_rxerr_cb,
  .timeout_cb = uart_timeout_cb,
  .baud      = 115200,
  .cr1        = USART_CTLR1_DATA8,      /* 8 data bits, no parity */
  .cr2        = USART_CTLR2_STOP1_BITS, /* 1 stop bit */
  .cr3        = 0U
};

static THD_WORKING_AREA(waUARTThread, 1024);
static THD_FUNCTION(UARTThread, arg) {
  (void)arg;
  chRegSetThreadName("UARTThread");

  /* Configure USART3 TX (PB10) and RX (PB11) pins.
     AF7 for USART3 on CH32H417.*/
  palSetPadMode(GPIOB, GPIO_PIN10, PAL_CH32_ALTERNATE_PUSHPULL(7));
  palSetPadMode(GPIOB, GPIO_PIN11, PAL_CH32_ALTERNATE_INPUT(7));

  /* Start UART driver.*/
  uartStart(&UARTD3, &uart_cfg);

  /* Start continuous reception.*/
  uartStartReceive(&UARTD3, sizeof(uart_rx_buf), uart_rx_buf);
uint32_t last_rx_count = 0;

  while (true) {
    if(rx_index != last_rx_count) {
        uartStartSend(&UARTD3, 4, &rx_index);
        last_rx_count = rx_index;
    }
    /* Send test message every second.*/
    // uartStartSend(&UARTD3, 32, uart_tx_buf);
    /* Print statistics every 10 seconds */
    // if ((uart_tx_count % 10) == 0 && uart_tx_count != 0 ) {
    //   chprintf((BaseSequentialStream *)&SDU1, 
    //            "UART Stats: TX=%u, RX=%u, ERR=%u\r\n", 
    //            uart_tx_count, uart_rx_count, uart_errors);
    // }
    
    chThdSleepMilliseconds(1000);
  }
}

static void cmd_hello(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  chprintf(chp, "Hello from ChibiOS Shell!\r\n");
}

static const ShellCommand commands[] = {
  {"hello", cmd_hello},  
  {NULL, NULL}         
};

static const ShellConfig shell_cfg = {
  (BaseSequentialStream *)&SDU1, 
  commands                      
};

#define SHELL_WA_SIZE THD_WORKING_AREA_SIZE(1024)

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
    // sduObjectInit(&SDU1);
    // sduStart(&SDU1, &serusbcfg);

    /*
     * Activates the USB driver and then the USB bus pull-up on D+.
     * Note, a delay is inserted in order to not have to disconnect the cable
     * after a reset.
     */
    // usbDisconnectBus(serusbcfg.usbp);
    // chThdSleepMilliseconds(1500);
    // usbStart(serusbcfg.usbp, &usbcfg);
    // usbConnectBus(serusbcfg.usbp);

    /*
     * Creates the example thread.
     */
    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
    chThdCreateStatic(waADCThread, sizeof(waADCThread), NORMALPRIO, ADCThread, NULL);
    chThdCreateStatic(waGPTThread, sizeof(waGPTThread), NORMALPRIO, GPTThread, NULL);
    chThdCreateStatic(waPWMThread, sizeof(waPWMThread), NORMALPRIO, PWMThread, NULL);
    chThdCreateStatic(waICUThread, sizeof(waICUThread), NORMALPRIO, ICUThread, NULL);
    chThdCreateStatic(waSIOThread, sizeof(waSIOThread), NORMALPRIO, SIOThread, NULL);
    chThdCreateStatic(waSerialThread, sizeof(waSerialThread), NORMALPRIO, SerialThread, NULL);
    chThdCreateStatic(waUARTThread, sizeof(waUARTThread), NORMALPRIO, UARTThread, NULL);

    /*
     * Normal main() thread activity, in this demo it does nothing except
     * sleeping in a loop and check the button state.
     */
    while (true)
    {
        // if (SDU1.config->usbp->state == USB_ACTIVE)
        // {
        //     thread_t *shelltp =
        //         chThdCreateFromHeap(NULL, SHELL_WA_SIZE, "shell", NORMALPRIO + 1, shellThread, (void *)&shell_cfg);
        //     chThdWait(shelltp); /* Waiting termination.             */
        // }
        chThdSleepMilliseconds(500);
    }
}
