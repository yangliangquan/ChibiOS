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

#include <string.h>
#include <time.h>

#include "ch.h"
#include "hal.h"
#include "usbcfg.h"
#include "chprintf.h"
#include "ff.h"

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
    sioStart(&SIOD2, &sio_cfg);
    while(true){
        sioAsyncWrite(&SIOD2, (const uint8_t *)"Hello from SIOThread!\r\n", 24);
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
  .callback = gpt_default_config_cb,
  .dier = TIM_UIE,
  .cr2 = 0
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
  palSetPadMode(GPIOA, GPIO_PIN9, PAL_CH32_ALTERNATE_PUSHPULL(7));
  palSetPadMode(GPIOA, GPIO_PIN3, PAL_CH32_ALTERNATE_INPUT(7));

  /* Start Serial driver on USART2.*/
  sdStart(&SD1, &serial_cfg);

  /* Use SD2 as a BaseSequentialStream for chprintf.*/
  BaseSequentialStream *chp = (BaseSequentialStream *)&SD1;

  while (true) {
    // chprintf(chp, "Hello from SerialThread! (SD1 USART1)\r\n");
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

/*===========================================================================*/
/* DAC driver related.                                                       */
/*===========================================================================*/

/*
 * DAC configuration.
 * 12-bit right-aligned, software trigger, output buffer enabled.
 *
 * cr = 0 means:
 * - BOFF=0   -> output buffer enabled
 * - TEN=0    -> software trigger (no external trigger)
 * - TSEL=0   -> trigger selection (don't care since TEN=0)
 * - WAVE=0   -> no noise/triangle wave generation
 * - MAMP=0   -> mask/amplitude (don't care since WAVE=0)
 */
static const DACConfig dac1_cfg = {
  .init      = 0U,
  .datamode  = DAC_DHRM_12BIT_RIGHT,
  .cr        = 0b011U<<3
};

static const DACConversionGroup dac1_grpcfg = {
  .num_channels = 1,
  .end_cb = NULL,
  .error_cb = NULL,
  .dummy = 0
};

const GPTConfig dac_gpt_cfg = {
  .frequency = 10000,
  .callback = NULL,
  .dier = TIM_UIE,
  .cr2 = 0b010<<4
};

static THD_WORKING_AREA(waDACThread, 2048);
static THD_FUNCTION(DACThread, arg) {
  (void)arg;
  chRegSetThreadName("DACThread");

  /* Configure DAC1_OUT1 pin (PA4) as analog input (floating). */
  palSetPadMode(GPIOA, GPIO_PIN4, PAL_MODE_INPUT_ANALOG);

  /* Start DAC1 CH1. */
  uint16_t dac_samples[1024];

  for(uint32_t i = 0; i < 1024; i++) {
    dac_samples[i] = (i * 4095) / 1023; /* Ramp from 0 to 4095 */
  }

  gptStart(&GPTD5, &dac_gpt_cfg);
  gptStartContinuous(&GPTD5, 100);

  dacStart(&DACD1, &dac1_cfg);
  dacStartConversion(&DACD1,&dac1_grpcfg, dac_samples, sizeof(dac_samples)/sizeof(*dac_samples));
  /* Generate a sawtooth wave on DAC1 CH1. */
  uint32_t val = 0;
  while (true) {
    /* Output 12-bit right-aligned sample. */

    /* Increment and wrap at 12-bit max value. */
    val = (val + 16) & 0xFFF;

    chThdSleepMilliseconds(2000);
  }
}

/*===========================================================================*/
/* CAN driver related.                                                       */
/*===========================================================================*/

/*
 * CAN1 configuration.
 * Bit timing for 125kbps @ 48MHz CAN clock (APB1 = HCLK/HPRE).
 * With HPRE_DIV1: HCLK = 96MHz, but CAN clock is from HB1 (APB1-like).
 * CAN clock = PCLK1 = HCLK / HPRE.
 * With HPRE_DIV4: HCLK = 96MHz, PCLK1 = 96MHz.
 * For standard bxCAN: CAN clock = PCLK1 (max 48MHz on typical STM32).
 * CH32H417 may support higher. Using PCLK1 = 96MHz.
 *
 * Bit timing for 125kbps with 96MHz CAN clock:
 * tq = (BRP + 1) / CAN_clock
 * Nominal bit time = (tq * (TS1 + TS2 + 3))  (SJW + TS1 + TS2 + 1)
 * Let BRP = 23 (24 tq prescaler), CAN_CLK = 96MHz / 24 = 4MHz -> tq = 250ns
 * Target 125kbps -> bit time = 8us = 32 tq
 * Sync = 1 tq, TS1 = 22 tq, TS2 = 9 tq -> total 32 tq
 * SJW = 4 tq
 */
#define CAN_TEST_BAUD_RATE           125000
#define CAN_TEST_TX_ID               0x42A

static const CANConfig can1_cfg = {
  .mcr = CAN_CTLR_ABOM | CAN_CTLR_TXFP,
  .btr = CAN_BTIMR_BRP(23)   |         /* BRP = 23 -> prescaler 24 */
         CAN_BTIMR_TS1(22)   |         /* TS1 = 22 tq */
         CAN_BTIMR_TS2(9)    |         /* TS2 = 9 tq */
         CAN_BTIMR_SJW(4)              /* SJW = 4 tq */|CAN_BTIMR_LBKM
};

static THD_WORKING_AREA(waCANThread, 1024);
static THD_FUNCTION(CANThread, arg) {
  (void)arg;
  uint32_t tx_count = 0;
  uint32_t rx_count = 0;
  uint32_t err_count = 0;

  chRegSetThreadName("CANThread");

  /* Configure CAN1 pins: PD0(RX), PD1(TX), AF9 for CAN1 on CH32H417. */
  palSetPadMode(GPIOD, GPIO_PIN0, PAL_CH32_ALTERNATE_INPUT(9));
  palSetPadMode(GPIOD, GPIO_PIN1, PAL_CH32_ALTERNATE_PUSHPULL(9));

  /* Start CAN1. */
  canStart(&CAND1, &can1_cfg);

  /* Initialize a CAN Tx frame. Standard ID 0x42A, 8 data bytes. */
  CANTxFrame tx_frame;
  tx_frame.IDE = CAN_IDE_STD;
  tx_frame.RTR = CAN_RTR_DATA;
  tx_frame.SID = CAN_TEST_TX_ID;
  tx_frame.DLC = 8;

  /* Set initial data. */
  tx_frame.data32[0] = 0;
  tx_frame.data32[1] = 0;

  /* Set up Rx frame buffer. */
  CANRxFrame rx_frame;
  msg_t status;
  systime_t time;

  while (true) {
    /* Update data bytes with a counter. */
    tx_frame.data8[0] = (uint8_t)(tx_count >> 0);
    tx_frame.data8[1] = (uint8_t)(tx_count >> 8);
    tx_frame.data8[2] = (uint8_t)(tx_count >> 16);
    tx_frame.data8[3] = (uint8_t)(tx_count >> 24);
    tx_frame.data8[4] = 0xAA;
    tx_frame.data8[5] = 0xBB;
    tx_frame.data8[6] = 0xCC;
    tx_frame.data8[7] = 0xDD;

    /* Transmit a CAN frame with 100ms timeout. */
    status = canTransmitTimeout(&CAND1, CAN_ANY_MAILBOX, &tx_frame,
                                TIME_MS2I(100));

    if (status == MSG_OK) {
      tx_count++;
      /* Toggle PD4 (orange LED) on successful transmit. */
      palTogglePad(GPIOD, GPIO_PIN4);
    }
    else {
      err_count++;
      /* Set PD4 steady on error. */
      palSetPad(GPIOD, GPIO_PIN4);
    }

    /* Try to receive any pending frames (loopback or from other node). */
    time = TIME_MS2I(10);
    status = canReceiveTimeout(&CAND1, CAN_ANY_MAILBOX, &rx_frame, time);

    if (status == MSG_OK) {
      rx_count++;
    }

    /* Every ~50 transmissions, toggle PD5 (green LED). */
    if ((tx_count % 50) == 0) {
      palTogglePad(GPIOD, GPIO_PIN5);
    }

    chThdSleepMilliseconds(500);
  }
}

/*===========================================================================*/
/* SDC driver (SD Card) related.                                             */
/*===========================================================================*/

/*
 * SDC / FatFS test selection.
 * Define CH32_DEMO_USE_FATFS to use the FatFS test thread instead of the
 * raw SDC read/write test thread.
 */
#define CH32_DEMO_USE_FATFS           1

/*
 * SDIO vs SDMMC peripheral selection.
 * Define CH32_DEMO_USE_SDIO to use the SDIO peripheral test,
 * otherwise the SDMMC peripheral test is used.
 */
/* #define CH32_DEMO_USE_SDIO */

/*===========================================================================*/
/* SDC raw read/write test thread.                                          */
/*===========================================================================*/
#if !CH32_DEMO_USE_FATFS || defined(__DOXYGEN__)
/*
 * SDC test thread.
 * Tests SD card with comprehensive read/write verification:
 *   - Read block 0 (MBR) for basic read test
 *   - Write/read-back verification with various data patterns
 *   - Multi-block read/write test
 *   - Walking-bit and incrementing-byte pattern tests
 *
 * Use SDC_TEST_WRITE_BLK as the target block for write tests.
 * WARNING: write tests are destructive - data on the target block
 * will be overwritten. Pick a block that does not contain useful data.
 *
 * Default SDMMC pin mapping (AFIO_PCFR1_SDMMC_REMAP = 0):
 *   SDCK  = PC12, SDCMD = PD2
 *   SDD0  = PC8,  SDD1  = PC9
 *   SDD2  = PC10, SDD3  = PC11
 *
 * SDMMC alternate function: AF12 on CH32H417.
 */

/* Target block for write/verify tests (must not overlap MBR/filesystem). */
#define SDC_TEST_WRITE_BLK           0U

/* Number of blocks for multi-block read/write test. */
#define SDC_TEST_MULTI_BLKS          1U

/* Enable(=1) or disable(=0) write tests (destructive to test block). */
#define SDC_ENABLE_WRITE_TEST        1

static THD_WORKING_AREA(waSDCThread, 4096 * 2);
static THD_FUNCTION(SDCThread, arg) {
  (void)arg;
  uint32_t blk_cnt   = 0;
  uint32_t rd_cnt    = 0;
  uint32_t wr_cnt    = 0;
  uint32_t vfy_cnt   = 0;
  uint32_t err_cnt   = 0;
  BlockDeviceInfo info;
  __attribute__((aligned(16))) uint8_t  buf[MMCSD_BLOCK_SIZE];
  __attribute__((aligned(16))) uint8_t  ref[MMCSD_BLOCK_SIZE];
  uint32_t i;
  msg_t    result;

  chRegSetThreadName("SDCThread");

  /* Configure SDMMC GPIO pins, AF12 for SDMMC on CH32H417. */
  palSetPadMode(GPIOC, GPIO_PIN12, PAL_MODE_CH32_ALTERNATE_PUSHPULL); /* SDCK  */
  palSetPadMode(GPIOD, GPIO_PIN2,  PAL_MODE_CH32_ALTERNATE_PUSHPULL); /* SDCMD */
  palSetPadMode(GPIOC, GPIO_PIN8,  PAL_MODE_CH32_ALTERNATE_PUSHPULL); /* SDD0  */
  palSetPadMode(GPIOC, GPIO_PIN9,  PAL_MODE_CH32_ALTERNATE_PUSHPULL); /* SDD1  */
  palSetPadMode(GPIOC, GPIO_PIN10, PAL_MODE_CH32_ALTERNATE_PUSHPULL); /* SDD2  */
  palSetPadMode(GPIOC, GPIO_PIN11, PAL_MODE_CH32_ALTERNATE_PUSHPULL); /* SDD3  */

  enableHB1(RCC_PWREN);
  enableHB1(RCC_SWPMIEN);

  SWPMI->OR |= 1 << 0;

  PWR->CTLR &= ~PWR_CTLR_VIO_SWCR;
  PWR->CTLR |= 0x00000200;

  PWR->CTLR &= ~PWR_CTLR_VSEL_VIO18;
  PWR->CTLR |= 0x00000C00;

  /* Start the SDC driver with default configuration (4-bit mode). */
  chprintf((BaseSequentialStream *)&SD1, "SDC: Start SDC driver\r\n");
  sdcStart(&SDCD1, NULL);

  /* Short delay for card power-up. */
  chThdSleepMilliseconds(100);

  while (true) {
    chprintf((BaseSequentialStream *)&SD1, "SDC: Entry\r\n");

    /*------------------------------------------------------------------------*/
    /* 1. Check card insertion.                                              */
    /*------------------------------------------------------------------------*/
    if (!sdcIsCardInserted(&SDCD1)) {
      chprintf((BaseSequentialStream *)&SD1,
               "SDC: No card inserted\r\n");
      chThdSleepMilliseconds(2000);
      continue;
    }

    /*------------------------------------------------------------------------*/
    /* 2. Connect to the SD card.                                            */
    /*------------------------------------------------------------------------*/
    result = sdcConnect(&SDCD1);
    if (result != HAL_SUCCESS) {

      chprintf((BaseSequentialStream *)&SD1,
               "SDC: Connect failed, errors=0x%08lx\r\n",
               (unsigned long)SDCD1.errors);
      
      chThdSleepMilliseconds(2000);
      continue;
    }

    /*------------------------------------------------------------------------*/
    /* 3. Get card info.                                                     */
    /*------------------------------------------------------------------------*/
    if (sdcGetInfo(&SDCD1, &info) == HAL_SUCCESS) {
      chprintf((BaseSequentialStream *)&SD1,
               "SDC: Capacity: %lu blocks, %u bytes/block, total %lu KB\r\n",
               (unsigned long)info.blk_num, info.blk_size,
               (unsigned long long)((uint64_t)info.blk_num *
                                    (uint64_t)info.blk_size / 1024));
    }

    /*------------------------------------------------------------------------*/
    /* 4. Read block 0 (MBR).                                               */
    /*------------------------------------------------------------------------*/
    chprintf((BaseSequentialStream *)&SD1, "SDC: Test 1 - Read block 0 (MBR)\r\n");

    if (sdcRead(&SDCD1, 0, buf, 1) == HAL_SUCCESS) {
      chprintf((BaseSequentialStream *)&SD1,
               "SDC:   OK [%02x %02x %02x %02x %02x %02x %02x %02x "
                      "%02x %02x %02x %02x %02x %02x %02x %02x ...]\r\n",
               buf[0],  buf[1],  buf[2],  buf[3],
               buf[4],  buf[5],  buf[6],  buf[7],
               buf[8],  buf[9],  buf[10], buf[11],
               buf[12], buf[13], buf[14], buf[15]);
      rd_cnt++;
      blk_cnt++;
    }
    else {
      sdcflags_t err = sdcGetAndClearErrors(&SDCD1);
      chprintf((BaseSequentialStream *)&SD1,
               "SDC:   FAILED, errors=0x%08lx\r\n", (unsigned long)err);
      err_cnt++;
      goto sdc_cleanup;
    }

#if SDC_ENABLE_WRITE_TEST
    /*----------------------------------------------------------------------*/
    /* 5. Write pattern to test block and read back to verify.             */
    /*    Pattern: incrementing byte values 0x00..0xFF, repeated.          */
    /*----------------------------------------------------------------------*/
    chprintf((BaseSequentialStream *)&SD1,
             "SDC: Test 2 - Write incrementing-byte pattern to block %u\r\n",
             SDC_TEST_WRITE_BLK);

    for (i = 0; i < MMCSD_BLOCK_SIZE; i++) {
      buf[i] = (uint8_t)i;
    }

    if (sdcWrite(&SDCD1, SDC_TEST_WRITE_BLK, buf, 1) != HAL_SUCCESS) {
      sdcflags_t err = sdcGetAndClearErrors(&SDCD1);
      chprintf((BaseSequentialStream *)&SD1,
               "SDC:   Write FAILED, errors=0x%08lx\r\n", (unsigned long)err);
      err_cnt++;
      goto sdc_cleanup;
    }
    wr_cnt++;

    /* Read back and verify. */
    memset(ref, 0, MMCSD_BLOCK_SIZE);
    if (sdcRead(&SDCD1, SDC_TEST_WRITE_BLK, ref, 1) != HAL_SUCCESS) {
      sdcflags_t err = sdcGetAndClearErrors(&SDCD1);
      chprintf((BaseSequentialStream *)&SD1,
               "SDC:   Read-back FAILED, errors=0x%08lx\r\n",
               (unsigned long)err);
      err_cnt++;
      goto sdc_cleanup;
    }
    rd_cnt++;

    if (memcmp(buf, ref, MMCSD_BLOCK_SIZE) == 0) {
      chprintf((BaseSequentialStream *)&SD1,
               "SDC:   Verify PASSED (%u bytes match)\r\n",
               MMCSD_BLOCK_SIZE);
      vfy_cnt++;
    }
    else {
      /* Find first mismatch. */
      for (i = 0; i < MMCSD_BLOCK_SIZE; i++) {
        if (buf[i] != ref[i]) {
          break;
        }
      }
      chprintf((BaseSequentialStream *)&SD1,
               "SDC:   Verify FAILED at offset %u "
               "(expected 0x%02x, got 0x%02x)\r\n",
               i, buf[i], ref[i]);
      err_cnt++;
      goto sdc_cleanup;
    }
    blk_cnt++;

    /*--------------------------------------------------------------------*/
    /* 6. Walking-bit pattern test.                                      */
    /*--------------------------------------------------------------------*/
    chprintf((BaseSequentialStream *)&SD1,
             "SDC: Test 3 - Walking-bit pattern (block %u)\r\n",
             SDC_TEST_WRITE_BLK + 1);

    for (i = 0; i < MMCSD_BLOCK_SIZE; i++) {
      buf[i] = (uint8_t)(1UL << (i & 7));
    }

    if (sdcWrite(&SDCD1, SDC_TEST_WRITE_BLK + 1, buf, 1) != HAL_SUCCESS) {
      sdcflags_t err = sdcGetAndClearErrors(&SDCD1);
      chprintf((BaseSequentialStream *)&SD1,
               "SDC:   Write FAILED, errors=0x%08lx\r\n", (unsigned long)err);
      err_cnt++;
      goto sdc_cleanup;
    }
    wr_cnt++;

    memset(ref, 0, MMCSD_BLOCK_SIZE);
    if (sdcRead(&SDCD1, SDC_TEST_WRITE_BLK + 1, ref, 1) != HAL_SUCCESS) {
      sdcflags_t err = sdcGetAndClearErrors(&SDCD1);
      chprintf((BaseSequentialStream *)&SD1,
               "SDC:   Read-back FAILED, errors=0x%08lx\r\n",
               (unsigned long)err);
      err_cnt++;
      goto sdc_cleanup;
    }
    rd_cnt++;

    if (memcmp(buf, ref, MMCSD_BLOCK_SIZE) == 0) {
      chprintf((BaseSequentialStream *)&SD1,
               "SDC:   Verify PASSED (%u bytes match)\r\n",
               MMCSD_BLOCK_SIZE);
      vfy_cnt++;
    }
    else {
      for (i = 0; i < MMCSD_BLOCK_SIZE; i++) {
        if (buf[i] != ref[i]) {
          break;
        }
      }
      chprintf((BaseSequentialStream *)&SD1,
               "SDC:   Verify FAILED at offset %u "
               "(expected 0x%02x, got 0x%02x)\r\n",
               i, buf[i], ref[i]);
      err_cnt++;
      goto sdc_cleanup;
    }
    blk_cnt++;

    /*--------------------------------------------------------------------*/
    /* 7. Multi-block read/write test (SDC_TEST_MULTI_BLKS blocks).       */
    /*--------------------------------------------------------------------*/
    chprintf((BaseSequentialStream *)&SD1,
             "SDC: Test 4 - Multi-block (%u blocks) starting at block %u\r\n",
             SDC_TEST_MULTI_BLKS, SDC_TEST_WRITE_BLK + 2);

    /* Prepare reference data: each block filled with its block-number byte. */
    {
      uint32_t b;
      for (b = 0; b < SDC_TEST_MULTI_BLKS; b++) {
        memset(buf, (uint8_t)(b & 0xFF), MMCSD_BLOCK_SIZE);
        if (sdcWrite(&SDCD1, SDC_TEST_WRITE_BLK + 2 + b, buf, 1)
            != HAL_SUCCESS) {
          sdcflags_t err = sdcGetAndClearErrors(&SDCD1);
          chprintf((BaseSequentialStream *)&SD1,
                   "SDC:   Write block %u FAILED, errors=0x%08lx\r\n",
                   SDC_TEST_WRITE_BLK + 2 + b, (unsigned long)err);
          err_cnt++;
          goto sdc_cleanup;
        }
        wr_cnt++;
      }
    }

    /* Read back all blocks at once and verify individually. */
    {
      uint8_t multi_buf[MMCSD_BLOCK_SIZE * SDC_TEST_MULTI_BLKS];
      uint32_t b;

      if (sdcRead(&SDCD1, SDC_TEST_WRITE_BLK + 2,
                  multi_buf, SDC_TEST_MULTI_BLKS) != HAL_SUCCESS) {
        sdcflags_t err = sdcGetAndClearErrors(&SDCD1);
        chprintf((BaseSequentialStream *)&SD1,
                 "SDC:   Multi-block read FAILED, errors=0x%08lx\r\n",
                 (unsigned long)err);
        err_cnt++;
        goto sdc_cleanup;
      }
      rd_cnt++;

      for (b = 0; b < SDC_TEST_MULTI_BLKS; b++) {
        const uint8_t *blk = multi_buf + (b * MMCSD_BLOCK_SIZE);
        uint8_t expected = (uint8_t)(b & 0xFF);
        bool ok = true;

        for (i = 0; i < MMCSD_BLOCK_SIZE; i++) {
          if (blk[i] != expected) {
            ok = false;
            break;
          }
        }

        if (ok) {
          vfy_cnt++;
        }
        else {
          chprintf((BaseSequentialStream *)&SD1,
                   "SDC:   Block %u verify FAILED at offset %u "
                   "(expected 0x%02x, got 0x%02x)\r\n",
                   SDC_TEST_WRITE_BLK + 2 + b, i, expected, blk[i]);
          err_cnt++;
          goto sdc_cleanup;
        }
      }

      chprintf((BaseSequentialStream *)&SD1,
               "SDC:   Multi-block verify PASSED (%u blocks)\r\n",
               SDC_TEST_MULTI_BLKS);
      blk_cnt += SDC_TEST_MULTI_BLKS;
    }
#endif /* SDC_ENABLE_WRITE_TEST */

    /*------------------------------------------------------------------------*/
    /* 8. Summary.                                                           */
    /*------------------------------------------------------------------------*/
    palTogglePad(GPIOD, GPIO_PIN4);

sdc_cleanup:
    sdcDisconnect(&SDCD1);

    chprintf((BaseSequentialStream *)&SD1,
             "SDC: === Summary: %lu blk xfers, "
             "%lu reads, %lu writes, %lu verifies, %lu errors ===\r\n",
             (unsigned long)blk_cnt,
             (unsigned long)rd_cnt,
             (unsigned long)wr_cnt,
             (unsigned long)vfy_cnt,
             (unsigned long)err_cnt);

    chThdSleepMilliseconds(3000);
  }
}
#endif /* !CH32_DEMO_USE_FATFS */

/*===========================================================================*/
/* FatFS test thread.                                                        */
/*===========================================================================*/
#if CH32_DEMO_USE_FATFS || defined(__DOXYGEN__)

static FATFS SDC_FS;
__attribute__((aligned(4))) uint8_t work[512];
static THD_WORKING_AREA(waFatFSThread, 4096 * 2);
static THD_FUNCTION(FatFSThread, arg) {
  (void)arg;
  FRESULT err;
  
  chRegSetThreadName("FatFSThread");

  /* Configure SDMMC GPIO pins, AF12 for SDMMC on CH32H417. */
  palSetPadMode(GPIOC, GPIO_PIN12, PAL_MODE_CH32_ALTERNATE_PUSHPULL); /* SDCK  */
  palSetPadMode(GPIOD, GPIO_PIN2,  PAL_MODE_CH32_ALTERNATE_PUSHPULL); /* SDCMD */
  palSetPadMode(GPIOC, GPIO_PIN8,  PAL_MODE_CH32_ALTERNATE_PUSHPULL); /* SDD0  */
  palSetPadMode(GPIOC, GPIO_PIN9,  PAL_MODE_CH32_ALTERNATE_PUSHPULL); /* SDD1  */
  palSetPadMode(GPIOC, GPIO_PIN10, PAL_MODE_CH32_ALTERNATE_PUSHPULL); /* SDD2  */
  palSetPadMode(GPIOC, GPIO_PIN11, PAL_MODE_CH32_ALTERNATE_PUSHPULL); /* SDD3  */

  enableHB1(RCC_PWREN);
  enableHB1(RCC_SWPMIEN);

  SWPMI->OR |= 1 << 0;

  PWR->CTLR &= ~PWR_CTLR_VIO_SWCR;
  PWR->CTLR |= 0x00000200;

  PWR->CTLR &= ~PWR_CTLR_VSEL_VIO18;
  PWR->CTLR |= 0x00000C00;

  /* Start the SDC driver with default configuration (4-bit mode). */
  chprintf((BaseSequentialStream *)&SD1, "FatFS: Start SDC driver\r\n");
  sdcStart(&SDCD1, NULL);

  /* Short delay for card power-up. */
  chThdSleepMilliseconds(100);

  while (true) {
    chprintf((BaseSequentialStream *)&SD1, "FatFS: Entry\r\n");

    /*------------------------------------------------------------------------*/
    /* 1. Check card insertion.                                              */
    /*------------------------------------------------------------------------*/
    if (!sdcIsCardInserted(&SDCD1)) {
      chprintf((BaseSequentialStream *)&SD1,
               "FatFS: No card inserted\r\n");
      chThdSleepMilliseconds(2000);
      continue;
    }

    /*------------------------------------------------------------------------*/
    /* 2. Connect to the SD card.                                            */
    /*------------------------------------------------------------------------*/
    if (sdcConnect(&SDCD1) != HAL_SUCCESS) {
      chprintf((BaseSequentialStream *)&SD1,
               "FatFS: Connect failed, errors=0x%08lx\r\n",
               (unsigned long)SDCD1.errors);
      chThdSleepMilliseconds(2000);
      continue;
    }

    /*------------------------------------------------------------------------*/
    /* 3. Mount FatFS.                                                       */
    /*------------------------------------------------------------------------*/
    chprintf((BaseSequentialStream *)&SD1, "FatFS: Mounting...\r\n");
    err = f_mount(&SDC_FS, "/", 1);

    if (err == FR_NO_FILESYSTEM) {
      chprintf((BaseSequentialStream *)&SD1,
               "FatFS: No filesystem, formatting...\r\n");
      /* Format the card as FAT32 with auto cluster size. */
      MKFS_PARM mkfs_opt = {
        .fmt     = FM_FAT32,
        .n_fat   = 0,    /* auto */
        .align   = 0,    /* auto */
        .n_root  = 0,    /* auto */
        .au_size = 0,    /* auto */
      };
      err = f_mkfs("/", &mkfs_opt, work, sizeof work);
      if (err != FR_OK) {
        chprintf((BaseSequentialStream *)&SD1,
                 "FatFS: Format failed (%d)\r\n", err);
        sdcDisconnect(&SDCD1);
        chThdSleepMilliseconds(3000);
        continue;
      }
      chprintf((BaseSequentialStream *)&SD1,
               "FatFS: Format OK, remounting...\r\n");
      err = f_mount(&SDC_FS, "0:", 1);
    }

    if (err != FR_OK) {
      chprintf((BaseSequentialStream *)&SD1,
               "FatFS: Mount failed (%d)\r\n", err);
      sdcDisconnect(&SDCD1);
      chThdSleepMilliseconds(3000);
      continue;
    }
    chprintf((BaseSequentialStream *)&SD1, "FatFS: Mount OK\r\n");

    /*------------------------------------------------------------------------*/
    /* 4. Create/open a test file and write data.                             */
    /*------------------------------------------------------------------------*/
    {
      uint8_t write_buf[256];
      uint8_t read_buf[256];
      UINT bw, br;
      uint32_t i;

      for (i = 0; i < sizeof(write_buf); i++) {
        write_buf[i] = (uint8_t)i;
      }

      chprintf((BaseSequentialStream *)&SD1,
               "FatFS: Writing test file 'fatfs_test.bin'...\r\n");

      FIL fil;

      err = f_open(&fil, "fatfs_test.bin", FA_CREATE_ALWAYS | FA_WRITE);
      if (err != FR_OK) {
        chprintf((BaseSequentialStream *)&SD1,
                 "FatFS: f_open(fatfs_test.bin) failed (%d)\r\n", err);
        goto fatfs_cleanup;
      }

      err = f_write(&fil, write_buf, sizeof(write_buf), &bw);
      if ((err != FR_OK) || (bw != sizeof(write_buf))) {
        chprintf((BaseSequentialStream *)&SD1,
                 "FatFS: f_write failed (%d, bw=%u)\r\n", err, bw);
        f_close(&fil);
        goto fatfs_cleanup;
      }
      chprintf((BaseSequentialStream *)&SD1,
               "FatFS: Wrote %u bytes OK\r\n", bw);

      f_close(&fil);

      /*--------------------------------------------------------------------*/
      /* 5. Read back and verify.                                           */
      /*--------------------------------------------------------------------*/
      err = f_open(&fil, "fatfs_test.bin", FA_READ);
      if (err != FR_OK) {
        chprintf((BaseSequentialStream *)&SD1,
                 "FatFS: f_open(fatfs_test.bin) for read failed (%d)\r\n",
                 err);
        goto fatfs_cleanup;
      }

      memset(read_buf, 0, sizeof(read_buf));
      err = f_read(&fil, read_buf, sizeof(read_buf), &br);
      if ((err != FR_OK) || (br != sizeof(read_buf))) {
        chprintf((BaseSequentialStream *)&SD1,
                 "FatFS: f_read failed (%d, br=%u)\r\n", err, br);
        f_close(&fil);
        goto fatfs_cleanup;
      }
      f_close(&fil);

      if (memcmp(write_buf, read_buf, sizeof(write_buf)) == 0) {
        palTogglePad(GPIOD, GPIO_PIN4);
        chprintf((BaseSequentialStream *)&SD1,
                 "FatFS: Verify PASSED (%u bytes match)\r\n",
                 sizeof(write_buf));
      }
      else {
        for (i = 0; i < sizeof(write_buf); i++) {
          if (write_buf[i] != read_buf[i]) {
            break;
          }
        }
        chprintf((BaseSequentialStream *)&SD1,
                 "FatFS: Verify FAILED at offset %u "
                 "(expected 0x%02x, got 0x%02x)\r\n",
                 i, write_buf[i], read_buf[i]);
        goto fatfs_cleanup;
      }
    }

    /*------------------------------------------------------------------------*/
    /* 6. List root directory.                                               */
    /*------------------------------------------------------------------------*/
    {
      DIR dir;
      FILINFO fno;
      FRESULT res;

      chprintf((BaseSequentialStream *)&SD1,
               "FatFS: Listing root directory:\r\n");

      res = f_opendir(&dir, "0:");
      if (res == FR_OK) {
        while (1) {
          res = f_readdir(&dir, &fno);
          if ((res != FR_OK) || (fno.fname[0] == 0)) {
            break;
          }
          if (fno.fattrib & AM_DIR) {
            chprintf((BaseSequentialStream *)&SD1,
                     "  [DIR]  %s\r\n", fno.fname);
          }
          else {
            chprintf((BaseSequentialStream *)&SD1,
                     "  [FILE] %-12s %lu bytes\r\n",
                     fno.fname, (unsigned long)fno.fsize);
          }
        }
        f_closedir(&dir);
      }
    }

    /*------------------------------------------------------------------------*/
    /* 7. Get free space.                                                    */
    /*------------------------------------------------------------------------*/
    {
      uint32_t fre_clust;
      FATFS *fsp;

      err = f_getfree("0:", &fre_clust, &fsp);
      if (err == FR_OK) {
        uint32_t bytes_free = fre_clust * (uint32_t)fsp->csize *
                              MMCSD_BLOCK_SIZE;
        chprintf((BaseSequentialStream *)&SD1,
                 "FatFS: Free: %lu clusters, %lu KB\r\n",
                 (unsigned long)fre_clust,
                 (unsigned long)(bytes_free / 1024));
      }
    }

fatfs_cleanup:
    /*------------------------------------------------------------------------*/
    /* 8. Unmount and disconnect.                                            */
    /*------------------------------------------------------------------------*/
    f_mount(NULL, "0:", 0);
    sdcDisconnect(&SDCD1);

    chprintf((BaseSequentialStream *)&SD1,
             "FatFS: ============ Test cycle done ============\r\n");

    chThdSleepMilliseconds(5000);
  }
}
#endif /* CH32_DEMO_USE_FATFS */

/*===========================================================================*/
/* SDIO test thread (when CH32_DEMO_USE_SDIO is defined).                   */
/*===========================================================================*/
#if CH32_DEMO_USE_SDIO || defined(__DOXYGEN__)

static THD_WORKING_AREA(waSDIOThread, 4096 * 2);
static THD_FUNCTION(SDIOThread, arg) {
  (void)arg;
  uint32_t blk_cnt   = 0;
  uint32_t rd_cnt    = 0;
  uint32_t wr_cnt    = 0;
  uint32_t vfy_cnt   = 0;
  uint32_t err_cnt   = 0;
  BlockDeviceInfo info;
  __attribute__((aligned(16))) uint8_t  buf[MMCSD_BLOCK_SIZE];
  __attribute__((aligned(16))) uint8_t  ref[MMCSD_BLOCK_SIZE];
  uint32_t i;
  msg_t    result;

  chRegSetThreadName("SDIOThread");

  /* Configure SDIO GPIO pins, AF8 for SDIO on CH32H417.
   * SDIO pin mapping (EVT board):
   *   SDCLK = PB10, SDCMD = PB11
   *   SDD0  = PE8,  SDD1  = PE9
   *   SDD2  = PE10, SDD3  = PE11 */
  palSetPadMode(GPIOB, GPIO_PIN10, PAL_CH32_ALTERNATE_PUSHPULL(8));  /* SDCLK  */
  palSetPadMode(GPIOB, GPIO_PIN11, PAL_CH32_ALTERNATE_PUSHPULL(8));  /* SDCMD  */
  palSetPadMode(GPIOE, GPIO_PIN8,  PAL_CH32_ALTERNATE_PUSHPULL(8));  /* SDD0   */
  palSetPadMode(GPIOE, GPIO_PIN9,  PAL_CH32_ALTERNATE_PUSHPULL(8));  /* SDD1   */
  palSetPadMode(GPIOE, GPIO_PIN10, PAL_CH32_ALTERNATE_PUSHPULL(8));  /* SDD2   */
  palSetPadMode(GPIOE, GPIO_PIN11, PAL_CH32_ALTERNATE_PUSHPULL(8));  /* SDD3   */

  /* Start the SDC driver with default configuration (4-bit mode). */
  chprintf((BaseSequentialStream *)&SD1, "SDIO: Start SDC driver\r\n");
  sdcStart(&SDCD1, NULL);

  /* Short delay for card power-up. */
  chThdSleepMilliseconds(100);

  while (true) {
    chprintf((BaseSequentialStream *)&SD1, "SDIO: Entry\r\n");

    /*------------------------------------------------------------------------*/
    /* 1. Check card insertion.                                              */
    /*------------------------------------------------------------------------*/
    if (!sdcIsCardInserted(&SDCD1)) {
      chprintf((BaseSequentialStream *)&SD1,
               "SDIO: No card inserted\r\n");
      chThdSleepMilliseconds(2000);
      continue;
    }

    /*------------------------------------------------------------------------*/
    /* 2. Connect to the SD card.                                            */
    /*------------------------------------------------------------------------*/
    result = sdcConnect(&SDCD1);
    if (result != HAL_SUCCESS) {

      chprintf((BaseSequentialStream *)&SD1,
               "SDIO: Connect failed, errors=0x%08lx\r\n",
               (unsigned long)SDCD1.errors);
      
      chThdSleepMilliseconds(2000);
      continue;
    }

    /*------------------------------------------------------------------------*/
    /* 3. Get card info.                                                     */
    /*------------------------------------------------------------------------*/
    if (sdcGetInfo(&SDCD1, &info) == HAL_SUCCESS) {
      chprintf((BaseSequentialStream *)&SD1,
               "SDIO: Capacity: %lu blocks, %u bytes/block, total %lu KB\r\n",
               (unsigned long)info.blk_num, info.blk_size,
               (unsigned long long)((uint64_t)info.blk_num *
                                    (uint64_t)info.blk_size / 1024));
    }

    /*------------------------------------------------------------------------*/
    /* 4. Read block 0 (MBR).                                               */
    /*------------------------------------------------------------------------*/
    chprintf((BaseSequentialStream *)&SD1, "SDIO: Test 1 - Read block 0 (MBR)\r\n");

    if (sdcRead(&SDCD1, 0, buf, 1) == HAL_SUCCESS) {
      chprintf((BaseSequentialStream *)&SD1,
               "SDIO:   OK [%02x %02x %02x %02x %02x %02x %02x %02x "
                       "%02x %02x %02x %02x %02x %02x %02x %02x ...]\r\n",
               buf[0],  buf[1],  buf[2],  buf[3],
               buf[4],  buf[5],  buf[6],  buf[7],
               buf[8],  buf[9],  buf[10], buf[11],
               buf[12], buf[13], buf[14], buf[15]);
      rd_cnt++;
      blk_cnt++;
    }
    else {
      sdcflags_t err = sdcGetAndClearErrors(&SDCD1);
      chprintf((BaseSequentialStream *)&SD1,
               "SDIO:   FAILED, errors=0x%08lx\r\n", (unsigned long)err);
      err_cnt++;
      goto sdio_cleanup;
    }

    /*------------------------------------------------------------------------*/
    /* 5. Write pattern to test block and read back to verify.               */
    /*------------------------------------------------------------------------*/
    chprintf((BaseSequentialStream *)&SD1,
             "SDIO: Test 2 - Write incrementing-byte pattern to block 0\r\n");

    for (i = 0; i < MMCSD_BLOCK_SIZE; i++) {
      buf[i] = (uint8_t)i;
    }

    if (sdcWrite(&SDCD1, 0, buf, 1) != HAL_SUCCESS) {
      sdcflags_t err = sdcGetAndClearErrors(&SDCD1);
      chprintf((BaseSequentialStream *)&SD1,
               "SDIO:   Write FAILED, errors=0x%08lx\r\n", (unsigned long)err);
      err_cnt++;
      goto sdio_cleanup;
    }
    wr_cnt++;

    /* Read back and verify. */
    memset(ref, 0, MMCSD_BLOCK_SIZE);
    if (sdcRead(&SDCD1, 0, ref, 1) != HAL_SUCCESS) {
      sdcflags_t err = sdcGetAndClearErrors(&SDCD1);
      chprintf((BaseSequentialStream *)&SD1,
               "SDIO:   Read-back FAILED, errors=0x%08lx\r\n",
               (unsigned long)err);
      err_cnt++;
      goto sdio_cleanup;
    }
    rd_cnt++;

    if (memcmp(buf, ref, MMCSD_BLOCK_SIZE) == 0) {
      chprintf((BaseSequentialStream *)&SD1,
               "SDIO:   Verify PASSED (%u bytes match)\r\n",
               MMCSD_BLOCK_SIZE);
      vfy_cnt++;
    }
    else {
      for (i = 0; i < MMCSD_BLOCK_SIZE; i++) {
        if (buf[i] != ref[i]) {
          break;
        }
      }
      chprintf((BaseSequentialStream *)&SD1,
               "SDIO:   Verify FAILED at offset %u "
               "(expected 0x%02x, got 0x%02x)\r\n",
               i, buf[i], ref[i]);
      err_cnt++;
      goto sdio_cleanup;
    }
    blk_cnt++;

    palTogglePad(GPIOD, GPIO_PIN4);

sdio_cleanup:
    sdcDisconnect(&SDCD1);

    chprintf((BaseSequentialStream *)&SD1,
             "SDIO: === Summary: %lu blk xfers, "
             "%lu reads, %lu writes, %lu verifies, %lu errors ===\r\n",
             (unsigned long)blk_cnt,
             (unsigned long)rd_cnt,
             (unsigned long)wr_cnt,
             (unsigned long)vfy_cnt,
             (unsigned long)err_cnt);

    chThdSleepMilliseconds(3000);
  }
}
#endif /* CH32_DEMO_USE_SDIO */

/*===========================================================================*/
/* RTC driver test thread.                                                   */
/*===========================================================================*/

static THD_WORKING_AREA(waRTCThread, 1024);
static THD_FUNCTION(RTCThread, arg) {
  (void)arg;
  RTCDateTime timespec;
  RTCAlarm alarmspec;
  struct tm timp;
  uint32_t tv_msec;

  chRegSetThreadName("RTCThread");

  /* Initialize RTC driver.*/
  rtcInit();

  /* Set prescaler for LSI clock (~40 kHz). 40000-1 = 39999 => 1 Hz.*/
  rtc_lld_set_prescaler(CH32_RTC_CLOCK_FREQ - 1);

  /* Set initial time: 2025-01-01 00:00:00 UTC (Wednesday).*/
  timp.tm_year = 125;   /* years since 1900 */
  timp.tm_mon  = 0;     /* January (0-based) */
  timp.tm_mday = 1;
  timp.tm_hour = 0;
  timp.tm_min  = 0;
  timp.tm_sec  = 0;
  timp.tm_isdst = 0;
  rtcConvertStructTmToDateTime(&timp, 0, &timespec);
  rtcSetTime(&RTCD1, &timespec);

  chprintf((BaseSequentialStream *)&SD1,
           "RTC: Time set, starting periodic readout\r\n");

  /* Set alarm for 10 seconds from now.*/
  rtcGetTime(&RTCD1, &timespec);
  rtcConvertDateTimeToStructTm(&timespec, &timp, &tv_msec);
  alarmspec.tv_sec = mktime(&timp) + 10;
  rtcSetAlarm(&RTCD1, 0, &alarmspec);
  chprintf((BaseSequentialStream *)&SD1,
           "RTC: Alarm set for +10 seconds\r\n");

  while (true) {
    rtcGetTime(&RTCD1, &timespec);
    rtcConvertDateTimeToStructTm(&timespec, &timp, &tv_msec);

    chprintf((BaseSequentialStream *)&SD1,
             "RTC: %04d-%02d-%02d %02d:%02d:%02d.%03lu\r\n",
             timp.tm_year + 1900, timp.tm_mon + 1, timp.tm_mday,
             timp.tm_hour, timp.tm_min, timp.tm_sec,
             (unsigned long)tv_msec);

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

#define SHELL_WA_SIZE THD_WORKING_AREA_SIZE(2048)

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
    // chThdSleepMilliseconds(1500);
    usbStart(serusbcfg.usbp, &usbcfg);
    usbConnectBus(serusbcfg.usbp);

    /*
     * Creates the example thread.
     */
    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
    // chThdCreateStatic(waADCThread, sizeof(waADCThread), NORMALPRIO, ADCThread, NULL);
    // chThdCreateStatic(waGPTThread, sizeof(waGPTThread), NORMALPRIO, GPTThread, NULL);
    // chThdCreateStatic(waPWMThread, sizeof(waPWMThread), NORMALPRIO, PWMThread, NULL);
    // chThdCreateStatic(waICUThread, sizeof(waICUThread), NORMALPRIO, ICUThread, NULL);
    // chThdCreateStatic(waSIOThread, sizeof(waSIOThread), NORMALPRIO, SIOThread, NULL);
    chThdCreateStatic(waSerialThread, sizeof(waSerialThread), NORMALPRIO, SerialThread, NULL);
    // chThdCreateStatic(waUARTThread, sizeof(waUARTThread), NORMALPRIO, UARTThread, NULL);
    // chThdCreateStatic(waSPIThread, sizeof(waSPIThread), NORMALPRIO, SPIThread, NULL);
    // chThdCreateStatic(waI2CThread, sizeof(waI2CThread), NORMALPRIO, I2CThread, NULL);
    // chThdCreateStatic(waI2SThread, sizeof(waI2SThread), NORMALPRIO, I2SThread, NULL);
    // chThdCreateStatic(waDACThread, sizeof(waDACThread), NORMALPRIO, DACThread, NULL);
    // chThdCreateStatic(waCANThread, sizeof(waCANThread), NORMALPRIO, CANThread, NULL);
    chThdCreateStatic(waRTCThread, sizeof(waRTCThread), NORMALPRIO, RTCThread, NULL);
#if CH32_DEMO_USE_SDIO
    chThdCreateStatic(waSDIOThread, sizeof(waSDIOThread),
                      NORMALPRIO - 1, SDIOThread, NULL);
#elif CH32_DEMO_USE_FATFS
    chThdCreateStatic(waFatFSThread, sizeof(waFatFSThread),
                      NORMALPRIO - 1, FatFSThread, NULL);
#else
    chThdCreateStatic(waSDCThread, sizeof(waSDCThread),
                      NORMALPRIO - 1, SDCThread, NULL);
#endif

    /*
     * Normal main() thread activity, in this demo it does nothing except
     * sleeping in a loop and check the button state.
     */
    while (true)
    {
        if (SDU1.config->usbp->state == USB_ACTIVE)
        {
            thread_t *shelltp =
                chThdCreateFromHeap(NULL, SHELL_WA_SIZE, "shell", NORMALPRIO + 1, shellThread, (void *)&shell_cfg);
            chThdWait(shelltp); /* Waiting termination.             */
        }
        chThdSleepMilliseconds(500);
    }
}
