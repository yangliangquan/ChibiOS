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
    palSetPadMode(GPIOB, GPIO_PIN4, PAL_CH32_ALTERNATE_PUSHPULL(2));
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

/*===========================================================================*/
/* I2C driver related.                                                       */
/*===========================================================================*/

/*
 * I2C1 test configuration.
 * Standard mode (100kHz), 7-bit addressing.
 */
static const I2CConfig i2c1_cfg = {
  .clock_speed = 100000,
  .duty_cycle  = FAST_DUTY_CYCLE_2,
  .op_mode     = OPMODE_I2C,
};

/*
 * I2C slave address for testing (e.g., EEPROM or sensor).
 * 0x50 is a common EEPROM address (e.g., AT24Cxx series).
 */
#define I2C_TEST_SLAVE_ADDR             0x50
#define I2C_EEPROM_MEM_ADDR             0x0000

static THD_WORKING_AREA(waI2CThread, 1024);
static THD_FUNCTION(I2CThread, arg) {
  (void)arg;
  chRegSetThreadName("I2CThread");

  /* Configure I2C1 pins: PB6(SCL), PB7(SDA), AF4 for I2C1 on CH32H417.*/
  palSetPadMode(GPIOB, GPIO_PIN6, PAL_CH32_ALTERNATE_OPENDRAIN(4));
  palSetPadMode(GPIOB, GPIO_PIN7, PAL_CH32_ALTERNATE_OPENDRAIN(4));

  /* Start I2C1.*/
  i2cStart(&I2CD1, &i2c1_cfg);

  /*
   * I2C test buffer.
   * Simulate a write to an I2C EEPROM:
   *   - First two bytes: memory address (0x0000)
   *   - Following bytes: data to write
   */
  uint8_t write_data[4] = {0xAA, 0xBB, 0xCC, 0xDD};
  uint8_t read_back[4];
  msg_t status;
  uint32_t pass_count = 0;
  uint32_t fail_count = 0;

  while (true) {
    /* Build transmit buffer: [addr_hi][addr_lo][data...].*/
    uint8_t tx_buf[6];
    tx_buf[0] = (uint8_t)(I2C_EEPROM_MEM_ADDR >> 8);
    tx_buf[1] = (uint8_t)(I2C_EEPROM_MEM_ADDR & 0xFF);
    tx_buf[2] = write_data[0];
    tx_buf[3] = write_data[1];
    tx_buf[4] = write_data[2];
    tx_buf[5] = write_data[3];

    /* Perform write: send memory address + data to EEPROM.*/
    status = i2cMasterTransmitTimeout(&I2CD1, I2C_TEST_SLAVE_ADDR,
                                       tx_buf, 6,
                                       NULL, 0,
                                       TIME_MS2I(100));

    if (status == MSG_OK) {
      /* Write succeeded, wait for EEPROM write cycle.*/
      chThdSleepMilliseconds(10);

      /* Read back by writing memory address, then reading data.*/
      status = i2cMasterTransmitTimeout(&I2CD1, I2C_TEST_SLAVE_ADDR,
                                         tx_buf, 2,   /* addr only */
                                         read_back, 4, /* read 4 bytes */
                                         TIME_MS2I(100));
      if (status == MSG_OK) {
        /* Verify read-back data matches what was written.*/
        if ((read_back[0] == write_data[0]) &&
            (read_back[1] == write_data[1]) &&
            (read_back[2] == write_data[2]) &&
            (read_back[3] == write_data[3])) {

          pass_count++;
          /* Toggle PD4 (orange LED) fast on success.*/
          palTogglePad(GPIOD, GPIO_PIN4);
        }
        else {
          fail_count++;
          /* Data mismatch - set PD4 steady on error.*/
          palSetPad(GPIOD, GPIO_PIN4);
        }
      }
      else {
        fail_count++;
      }
    }
    else {
      fail_count++;
      /* Write failed (no ACK) - likely no device on bus.*/
      palSetPad(GPIOD, GPIO_PIN4);
    }

    chThdSleepMilliseconds(2000);
  }
}

  
/*===========================================================================*/
/* SPI driver related.                                                       */
/*===========================================================================*/

/*
 * SPI1 configuration structure.
 * Master mode, 8-bit data, CPOL=0, CPHA=0, baud rate = APB2 / 8.
 * CH32 SPI1 is on HB2 bus (APB2-like), clock = HCLK / HPRE.
 * With default HPRE_DIV4: 96MHz / 4 = 24MHz on APB2.
 * BR_1 (bit 4) gives divider of 8: SPI clock = 24MHz / 8 = 3MHz.
 */
static const SPIConfig spi1_cfg = {
  .cr1   = SPI_CTLR1_BR_1,                     /* 3MHz @ 24MHz APB2 */
  .cr2   = 0U,                                  /* No extra features */
};

static THD_WORKING_AREA(waSPIThread, 1024);
static THD_FUNCTION(SPIThread, arg) {
  (void)arg;
  chRegSetThreadName("SPIThread");

  /* Configure SPI1 pins: PA5(SCK), PA6(MISO), PA7(MOSI), AF5 on CH32H417.*/
  palSetPadMode(GPIOA, GPIO_PIN5, PAL_CH32_ALTERNATE_PUSHPULL(5));
  palSetPadMode(GPIOA, GPIO_PIN6, PAL_CH32_ALTERNATE_INPUT(5));
  palSetPadMode(GPIOA, GPIO_PIN7, PAL_CH32_ALTERNATE_PUSHPULL(5));

  /* Start SPI1.*/
  spiStart(&SPID1, &spi1_cfg);

  // uint16_t tx_val = 0xA5;
  // uint16_t rx_val;
  // uint32_t xfer_count = 0;

  while (true) {
    // /* Perform polled exchange. For a self-test without external wiring,
    //    connect PA6 (MISO) to PA7 (MOSI) with a jumper on the EVT board. */
    // rx_val = spiPolledExchange(&SPID1, tx_val);

    // /* Simple check: in loopback mode, received value should match.*/
    // if (rx_val == tx_val) {
    //   /* SPI OK - toggle PD4 LED (orange) slowly.*/
    //   palTogglePad(GPIOD, GPIO_PIN4);
    //   xfer_count++;
    // }

    // /* Increment test pattern.*/
    // tx_val++;

    uint8_t tx_buf[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    uint8_t rx_buf[16];
    spiExchange(&SPID1, sizeof(tx_buf), tx_buf, rx_buf);

    chThdSleepMilliseconds(2000);
  }
}

/*===========================================================================*/
/* I2S driver related.                                                       */
/*===========================================================================*/

/*
 * I2S1 test: master TX, 16-bit data, 44.1kHz sample rate.
 * I2SDIV = 2, ODD = 0, MCKOE = 0.
 * For I2S master mode: I2SxCLK = (HCLK/HPRE) / (I2SDIV*2 + ODD) / 2
 * With HPRE_DIV4 and 96MHz: APB2 = 24MHz.
 * I2S clock = 24MHz / (2*2) / 2 = 3MHz bit clock.
 * Sample rate = bit clock / (16*2) = 3000000 / 32 = 93750 Hz.
 * Using I2SDIV=8, ODD=0: 24MHz / 16 / 2 = 750kHz bit clock.
 * Sample rate = 750000 / 32 = 23437 Hz.
 */
#define I2S_SAMPLE_RATE             24000
#define I2S_BUFFER_SIZE             256

static uint16_t i2s_tx_buf[I2S_BUFFER_SIZE];

static const I2SConfig i2s2_cfg = {
  .tx_buffer  = i2s_tx_buf,
  .rx_buffer  = NULL,
  .size       = I2S_BUFFER_SIZE,
  .end_cb     = NULL,
  .i2scfgr    = SPI_I2SCFGR_DATLEN_0,      /* 24-bit data, CHLEN=0 -> 32-bit frame */
  .i2spr      = (8 << 0) |                  /* I2SDIV = 8 */
                (0 << 8) |                  /* ODD = 0 */
                (0 << 9),                   /* MCKOE = 0 */
};

static THD_WORKING_AREA(waI2SThread, 1024);
static THD_FUNCTION(I2SThread, arg) {
  (void)arg;
  chRegSetThreadName("I2SThread");

  /* Configure I2S1 pins: PA5(CK), PA7(SD), PA4(WS), AF5 on CH32H417.*/
  palSetPadMode(GPIOB, GPIO_PIN13, PAL_CH32_ALTERNATE_PUSHPULL(5));
  palSetPadMode(GPIOB, GPIO_PIN15, PAL_CH32_ALTERNATE_PUSHPULL(5));
  palSetPadMode(GPIOB, GPIO_PIN12, PAL_CH32_ALTERNATE_PUSHPULL(5));

  /* Generate a simple sine wave pattern for testing.*/
  for (uint32_t i = 0; i < I2S_BUFFER_SIZE; i++) {
    i2s_tx_buf[i] = (uint16_t)(0x5000 + ((i * 200) & 0x7FFF));
  }

  /* Start I2S1.*/
  i2sStart(&I2SD2, &i2s2_cfg);

  /* Start continuous exchange (circular DMA).*/
  i2sStartExchange(&I2SD2);

  while (true) {
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
    chThdCreateStatic(waSPIThread, sizeof(waSPIThread), NORMALPRIO, SPIThread, NULL);
    // chThdCreateStatic(waI2CThread, sizeof(waI2CThread), NORMALPRIO, I2CThread, NULL);
    chThdCreateStatic(waI2SThread, sizeof(waI2SThread), NORMALPRIO, I2SThread, NULL);
    chThdCreateStatic(waDACThread, sizeof(waDACThread), NORMALPRIO, DACThread, NULL);

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
