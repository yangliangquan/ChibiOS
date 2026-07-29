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
#include "shell.h"
#include "hal_uart.h"

#define SHELL_WA_SIZE (1024 * 32)

/* SDIO test write block (avoid MBR). */
#define SDIO_TEST_WRITE_BLK    1024U

/*===========================================================================*/
/* SDIO + FatFS shell command.                                                */
/*===========================================================================*/

static FATFS sdio_fs;

static void cmd_sdio(BaseSequentialStream *chp, int argc, char *argv[]) {
  BlockDeviceInfo info;
  __attribute__((aligned(16))) uint8_t buf[MMCSD_BLOCK_SIZE];
  __attribute__((aligned(16))) uint8_t ref[MMCSD_BLOCK_SIZE];
  FRESULT fres;
  uint32_t i;
  msg_t result;
  (void)argc;
  (void)argv;

  enableHB2(RCC_AFIOEN);
  /*----------------------------------------------------------------------*/
  /* 1. Configure SDIO GPIO pins.                                         */
  /*    SDIO pin mapping (CH32H417, default AF8):                          */
  /*      SDIO_CK  = PB11 (AF8), SDIO_CMD = PB10 (AF8)                   */
  /*      SDIO_D0  = PE8  (AF8), SDIO_D1  = PE9  (AF8)                   */
  /*      SDIO_D2  = PE10 (AF8), SDIO_D3  = PE11 (AF8)                   */
  /*----------------------------------------------------------------------*/
  palSetPadMode(GPIOB, 11, PAL_CH32_ALTERNATE_PUSHPULL(8));  /* SDIO_CK  */
  palSetPadMode(GPIOB, 10, PAL_CH32_ALTERNATE_PUSHPULL(8));  /* SDIO_CMD */
  palSetPadMode(GPIOE,  8, PAL_CH32_ALTERNATE_PUSHPULL(8));  /* SDIO_D0  */
  palSetPadMode(GPIOE,  9, PAL_CH32_ALTERNATE_PUSHPULL(8));  /* SDIO_D1  */
  palSetPadMode(GPIOB,  3, PAL_CH32_ALTERNATE_PUSHPULL(9));  /* SDIO_D2  */
  palSetPadMode(GPIOB,  4, PAL_CH32_ALTERNATE_PUSHPULL(9));  /* SDIO_D3  */

  /*----------------------------------------------------------------------*/
  /* 2. Start SDC driver (4-bit mode) and wait for card power-up.        */
  /*----------------------------------------------------------------------*/
  chprintf(chp, "sdio: starting SDC driver (SDIO backend)...\r\n");
  sdcStart(&SDCD1, NULL);
  chThdSleepMilliseconds(200);

  /*----------------------------------------------------------------------*/
  /* 3. Check card insertion.                                             */
  /*----------------------------------------------------------------------*/
  if (!sdcIsCardInserted(&SDCD1)) {
    chprintf(chp, "sdio: no card inserted, aborting\r\n");
    sdcStop(&SDCD1);
    return;
  }
  chprintf(chp, "sdio: card detected\r\n");

  /*----------------------------------------------------------------------*/
  /* 4. Connect to the SD card.                                           */
  /*----------------------------------------------------------------------*/
  result = sdcConnect(&SDCD1);
  if (result != HAL_SUCCESS) {
    chprintf(chp, "sdio: connect failed, errors=0x%08lx\r\n",
             (unsigned long)SDCD1.errors);
    sdcStop(&SDCD1);
    return;
  }
  chprintf(chp, "sdio: card connected\r\n");

  /*----------------------------------------------------------------------*/
  /* 5. Get card info.                                                    */
  /*----------------------------------------------------------------------*/
  if (sdcGetInfo(&SDCD1, &info) == HAL_SUCCESS) {
    chprintf(chp,
             "sdio: capacity=%lu blocks, %u bytes/block, total=%lu KB\r\n",
             (unsigned long)info.blk_num, info.blk_size,
             (unsigned long)((uint64_t)info.blk_num *
                              (uint64_t)info.blk_size / 1024));
  }

  /*----------------------------------------------------------------------*/
  /* 6. SDC raw test: read block 0 (MBR).                                */
  /*----------------------------------------------------------------------*/
  chprintf(chp, "sdio: [SDC] read block 0 (MBR)...\r\n");
  if (sdcRead(&SDCD1, 0, buf, 1) == HAL_SUCCESS) {
    chprintf(chp,
             "sdio:   MBR=[%02x %02x %02x %02x %02x %02x %02x %02x"
             " %02x %02x %02x %02x %02x %02x %02x %02x...]\r\n",
             buf[0], buf[1], buf[2], buf[3],
             buf[4], buf[5], buf[6], buf[7],
             buf[8], buf[9], buf[10], buf[11],
             buf[12], buf[13], buf[14], buf[15]);
  }
  else {
    sdcflags_t err = sdcGetAndClearErrors(&SDCD1);
    chprintf(chp, "sdio:   read FAILED, errors=0x%08lx\r\n",
             (unsigned long)err);
    goto sdio_done;
  }

  /*----------------------------------------------------------------------*/
  /* 7. SDC raw test: write block and verify.                            */
  /*----------------------------------------------------------------------*/
  chprintf(chp, "sdio: [SDC] write+verify block %u...\r\n",
           SDIO_TEST_WRITE_BLK);

  for (i = 0; i < MMCSD_BLOCK_SIZE; i++) {
    buf[i] = (uint8_t)i;
  }

  if (sdcWrite(&SDCD1, SDIO_TEST_WRITE_BLK, buf, 1) != HAL_SUCCESS) {
    sdcflags_t err = sdcGetAndClearErrors(&SDCD1);
    chprintf(chp, "sdio:   write FAILED, errors=0x%08lx\r\n",
             (unsigned long)err);
    goto sdio_done;
  }

  memset(ref, 0, MMCSD_BLOCK_SIZE);
  if (sdcRead(&SDCD1, SDIO_TEST_WRITE_BLK, ref, 1) != HAL_SUCCESS) {
    sdcflags_t err = sdcGetAndClearErrors(&SDCD1);
    chprintf(chp, "sdio:   read-back FAILED, errors=0x%08lx\r\n",
             (unsigned long)err);
    goto sdio_done;
  }

  if (memcmp(buf, ref, MMCSD_BLOCK_SIZE) == 0) {
    chprintf(chp, "sdio:   SDC verify PASSED (%u bytes match)\r\n",
             MMCSD_BLOCK_SIZE);
  }
  else {
    for (i = 0; i < MMCSD_BLOCK_SIZE; i++) {
      if (buf[i] != ref[i]) {
        break;
      }
    }
    chprintf(chp, "sdio:   SDC verify FAILED at offset %u"
             " (expected 0x%02x, got 0x%02x)\r\n",
             i, buf[i], ref[i]);
    goto sdio_done;
  }

  /*----------------------------------------------------------------------*/
  /* 8. FatFS: mount.                                                     */
  /*----------------------------------------------------------------------*/
  chprintf(chp, "sdio: [FATFS] mounting...\r\n");
  fres = f_mount(&sdio_fs, "/", 1);

  if (fres == FR_NO_FILESYSTEM) {
    chprintf(chp, "sdio:   no filesystem, formatting (FAT32)...\r\n");
    MKFS_PARM mkfs_opt = {
      .fmt     = FM_FAT32,
      .n_fat   = 0,
      .align   = 0,
      .n_root  = 0,
      .au_size = 0,
    };
    uint8_t work[512];
    fres = f_mkfs("/", &mkfs_opt, work, sizeof(work));
    if (fres != FR_OK) {
      chprintf(chp, "sdio:   format FAILED (%d)\r\n", fres);
      goto sdio_done;
    }
    chprintf(chp, "sdio:   format OK, remounting...\r\n");
    fres = f_mount(&sdio_fs, "0:", 1);
  }

  if (fres != FR_OK) {
    chprintf(chp, "sdio:   mount FAILED (%d)\r\n", fres);
    goto sdio_done;
  }
  chprintf(chp, "sdio:   mount OK\r\n");

  /*----------------------------------------------------------------------*/
  /* 9. FatFS: write test file.                                           */
  /*----------------------------------------------------------------------*/
  {
    static const char *test_payload =
      "Hello from CH32H417 SDIO + FatFS test!\r\n"
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n"
      "abcdefghijklmnopqrstuvwxyz\r\n";
    uint32_t payload_len = (uint32_t)strlen(test_payload);
    uint8_t read_buf[128];
    UINT bw, br;
    FIL fil;

    chprintf(chp, "sdio: [FATFS] write 'sdio_test.txt'...\r\n");
    fres = f_open(&fil, "0:/sdio_test.txt",
                  FA_CREATE_ALWAYS | FA_WRITE);
    if (fres != FR_OK) {
      chprintf(chp, "sdio:   f_open(write) FAILED (%d)\r\n", fres);
      goto sdio_fatfs_done;
    }
    fres = f_write(&fil, test_payload, payload_len, &bw);
    f_close(&fil);
    if (fres != FR_OK || bw != payload_len) {
      chprintf(chp, "sdio:   f_write FAILED (%d, bw=%u)\r\n", fres, bw);
      goto sdio_fatfs_done;
    }
    chprintf(chp, "sdio:   wrote %u bytes\r\n", bw);

    /*--------------------------------------------------------------*/
    /* 10. FatFS: read back and verify.                             */
    /*--------------------------------------------------------------*/
    chprintf(chp, "sdio: [FATFS] read 'sdio_test.txt'...\r\n");
    fres = f_open(&fil, "0:/sdio_test.txt", FA_READ);
    if (fres != FR_OK) {
      chprintf(chp, "sdio:   f_open(read) FAILED (%d)\r\n", fres);
      goto sdio_fatfs_done;
    }
    memset(read_buf, 0, sizeof(read_buf));
    fres = f_read(&fil, read_buf, sizeof(read_buf) - 1, &br);
    f_close(&fil);
    if (fres != FR_OK) {
      chprintf(chp, "sdio:   f_read FAILED (%d)\r\n", fres);
      goto sdio_fatfs_done;
    }
    read_buf[br] = '\0';
    chprintf(chp, "sdio:   read %u bytes: \"%s\"\r\n", br, read_buf);

    if (br == payload_len &&
        memcmp(read_buf, test_payload, payload_len) == 0) {
      chprintf(chp, "sdio:   FatFS verify PASSED (%u bytes match)\r\n",
               br);
    }
    else {
      chprintf(chp, "sdio:   FatFS verify FAILED\r\n");
    }

    /*--------------------------------------------------------------*/
    /* 11. FatFS: list root directory.                              */
    /*--------------------------------------------------------------*/
    chprintf(chp, "sdio: [FATFS] list root dir:\r\n");
    {
      DIR dir;
      FILINFO fno;
      fres = f_opendir(&dir, "0:");
      if (fres == FR_OK) {
        while (1) {
          fres = f_readdir(&dir, &fno);
          if (fres != FR_OK || fno.fname[0] == '\0') {
            break;
          }
          if (fno.fattrib & AM_DIR) {
            chprintf(chp, "sdio:   [DIR]  %s\r\n", fno.fname);
          }
          else {
            chprintf(chp, "sdio:   [FILE] %-12s %lu bytes\r\n",
                     fno.fname, (unsigned long)fno.fsize);
          }
        }
        f_closedir(&dir);
      }
    }

    /*--------------------------------------------------------------*/
    /* 12. FatFS: free space.                                       */
    /*--------------------------------------------------------------*/
    {
      uint32_t fre_clust;
      FATFS *fsp;
      fres = f_getfree("0:", &fre_clust, &fsp);
      if (fres == FR_OK) {
        uint32_t bytes_free = fre_clust * (uint32_t)fsp->csize *
                              MMCSD_BLOCK_SIZE;
        chprintf(chp, "sdio:   free=%lu clusters, %lu KB\r\n",
                 (unsigned long)fre_clust,
                 (unsigned long)(bytes_free / 1024));
      }
    }
  }

sdio_fatfs_done:
  f_mount(NULL, "0:", 0);
  chprintf(chp, "sdio:   FATFS unmounted\r\n");

sdio_done:
  sdcDisconnect(&SDCD1);
  sdcStop(&SDCD1);
  chprintf(chp, "sdio: all tests complete\r\n");
}

/*===========================================================================*/
/* Shell command table and configuration.                                     */
/*===========================================================================*/

/*===========================================================================*/
/* WSPI (QSPI2) + flash shell command.                                       */
/*===========================================================================*/

/* QSPI Flash commands (W25Qxx series). */
#define QSPI_CMD_JEDEC_ID         0x9FU
#define QSPI_CMD_READ_SR1         0x05U
#define QSPI_CMD_WRITE_ENABLE     0x06U
#define QSPI_CMD_WRITE_DISABLE    0x04U
#define QSPI_CMD_SECTOR_ERASE     0x20U
#define QSPI_CMD_PAGE_PROGRAM     0x02U
#define QSPI_CMD_FAST_READ        0x0BU
#define QSPI_CMD_FAST_READ_QUAD   0xEBU
#define QSPI_CMD_ENABLE_RESET     0x66U
#define QSPI_CMD_RESET_DEVICE     0x99U

/*
 * WSPI2 configuration for dual QSPI flash on CH32H417 board:
 *
 * Primary flash (SCK/CS/IO0-IO3):
 *   SCK  = PF6  (AF4)    CS   = PF1  (AF5)
 *   IO0  = PF2  (AF5)    IO1  = PF9  (AF4)
 *   IO2  = PF10 (AF4)    IO3  = PE15 (AF7)
 *
 * Secondary flash (SCSXN/IOX0-IOX3):
 *   CSXN = PC1  (AF10)   IOX0 = PC2  (AF10)
 *   IOX1 = PC3  (AF10)   IOX2 = PB13 (AF11)
 *   IOX3 = PB14 (AF11)
 */
/* Primary flash: W25Qxx on NCS0. */
static const WSPIConfig wspi2_pri_cfg = {
  .end_cb         = NULL,
  .error_cb       = NULL,
  .prescaler      = 1,
  .ckmode         = 0,
  .cshtime        = 7,
  .fsize          = 19,   /* 2^(22+1) = 1MB per flash */
  .fifo_threshold = 0,
  .fselect        = 0,    /* NCS0 - primary flash */
  .dfm            = 0     /* single-flash mode */
};

/* Secondary flash: W25Qxx on NCS1. */
static const WSPIConfig wspi2_sec_cfg = {
  .end_cb         = NULL,
  .error_cb       = NULL,
  .prescaler      = 1,
  .ckmode         = 0,
  .cshtime        = 7,
  .fsize          = 19,   /* 2^(22+1) = 1MB per flash */
  .fifo_threshold = 0,
  .fselect        = 1,    /* NCS1 - secondary flash */
  .dfm            = 0     /* single-flash mode */
};

static void wspi_test_one_flash(BaseSequentialStream *chp,
                              const WSPIConfig *cfg,
                              const char *label) {
  wspi_command_t cmd;

  chprintf(chp, "wspi: --- %s ---\r\n", label);

  /* Start driver. */
  wspiStart(&WSPID2, cfg);
  chprintf(chp, "wspi: driver started (fselect=%u)\r\n",
           (unsigned)cfg->fselect);

  /* Reset flash. */
  chprintf(chp, "wspi: resetting flash...\r\n");
  cmd.cfg   = WSPI_CFG_CMD_MODE_ONE_LINE |
              WSPI_CFG_ADDR_MODE_NONE |
              WSPI_CFG_DATA_MODE_NONE;
  cmd.cmd   = QSPI_CMD_ENABLE_RESET;
  cmd.addr  = 0U;
  cmd.alt   = 0U;
  cmd.dummy = 0U;
  wspiCommand(&WSPID2, &cmd);
  cmd.cmd = QSPI_CMD_RESET_DEVICE;
  wspiCommand(&WSPID2, &cmd);
  chThdSleepMilliseconds(10);

  /* Read JEDEC ID. */
  {
    uint8_t id_buf[3] = {0, 0, 0};
    cmd.cfg   = WSPI_CFG_CMD_MODE_ONE_LINE |
                WSPI_CFG_ADDR_MODE_NONE |
                WSPI_CFG_DATA_MODE_ONE_LINE;
    cmd.cmd   = QSPI_CMD_JEDEC_ID;
    cmd.addr  = 0U;
    cmd.alt   = 0U;
    cmd.dummy = 0U;
    wspiReceive(&WSPID2, &cmd, 3, id_buf);
    chprintf(chp, "wspi: JEDEC ID = %02x %02x %02x\r\n",
             id_buf[0], id_buf[1], id_buf[2]);
    if (id_buf[0] == 0x00 || id_buf[0] == 0xFF) {
      chprintf(chp, "wspi: WARNING - no flash detected\r\n");
    }
  }

  /* Enable Quad mode (SR1 bit 1 = QE). */
  {
    uint8_t sr1 = 0;
    cmd.cfg   = WSPI_CFG_CMD_MODE_ONE_LINE |
                WSPI_CFG_ADDR_MODE_NONE |
                WSPI_CFG_DATA_MODE_ONE_LINE;
    cmd.cmd   = 0x35;
    cmd.addr  = 0U;
    cmd.alt   = 0U;
    cmd.dummy = 0U;
    wspiReceive(&WSPID2, &cmd, 1, &sr1);
    chprintf(chp, "wspi: SR1 = 0x%02x\r\n", sr1);

    cmd.cfg   = WSPI_CFG_CMD_MODE_ONE_LINE |
                WSPI_CFG_ADDR_MODE_NONE |
                WSPI_CFG_DATA_MODE_NONE;
    cmd.cmd   = QSPI_CMD_WRITE_ENABLE;
    cmd.addr  = 0U;
    cmd.alt   = 0U;
    cmd.dummy = 0U;
    wspiCommand(&WSPID2, &cmd);

    sr1 |= 0x02U;
    cmd.cfg   = WSPI_CFG_CMD_MODE_ONE_LINE |
                WSPI_CFG_ADDR_MODE_NONE |
                WSPI_CFG_DATA_MODE_ONE_LINE;
    cmd.cmd   = 0x31U;  /* Write Status Register 1 */
    cmd.addr  = 0U;
    cmd.alt   = 0U;
    cmd.dummy = 0U;
    {
      uint8_t wr = sr1;
      wspiSend(&WSPID2, &cmd, 1, &wr);
    }
    chprintf(chp, "wspi: SR1 written = 0x%02x\r\n", sr1);
  }

  /* 1-line fast read. */
  {
    uint8_t read_buf[16];
    uint32_t i;
    memset(read_buf, 0, sizeof(read_buf));
    cmd.cfg   = WSPI_CFG_CMD_MODE_ONE_LINE |
                WSPI_CFG_ADDR_MODE_ONE_LINE |
                WSPI_CFG_DATA_MODE_ONE_LINE |
                WSPI_CFG_ADDR_SIZE_24;
    cmd.cmd   = QSPI_CMD_FAST_READ;
    cmd.addr  = 0x000000U;
    cmd.alt   = 0U;
    cmd.dummy = 8U;
    wspiReceive(&WSPID2, &cmd, sizeof(read_buf), read_buf);
    chprintf(chp, "wspi: [1-line] read:");
    for (i = 0; i < sizeof(read_buf); i++) {
      chprintf(chp, " %02x", read_buf[i]);
    }
    chprintf(chp, "\r\n");
  }

  /* 4-line quad read. */
  {
    uint8_t quad_buf[16];
    uint32_t i;
    memset(quad_buf, 0, sizeof(quad_buf));
    cmd.cfg   = WSPI_CFG_CMD_MODE_ONE_LINE |
                WSPI_CFG_ADDR_MODE_FOUR_LINES |
                WSPI_CFG_DATA_MODE_FOUR_LINES |
                WSPI_CFG_ADDR_SIZE_24;
    cmd.cmd   = QSPI_CMD_FAST_READ_QUAD;
    cmd.addr  = 0x000000U;
    cmd.alt   = 0U;
    cmd.dummy = 6U;
    wspiReceive(&WSPID2, &cmd, sizeof(quad_buf), quad_buf);
    chprintf(chp, "wspi: [4-line] read:");
    for (i = 0; i < sizeof(quad_buf); i++) {
      chprintf(chp, " %02x", quad_buf[i]);
    }
    chprintf(chp, "\r\n");
  }

  /* 4-line quad write + verify. */
  {
    #define QSPI_TEST_ADDR   0x000000U
    #define QSPI_TEST_LEN    16U
    uint8_t wr_buf[QSPI_TEST_LEN];
    uint8_t rd_buf[QSPI_TEST_LEN];
    uint32_t i;

    /* Prepare data pattern. */
    for (i = 0; i < QSPI_TEST_LEN; i++) {
      wr_buf[i] = (uint8_t)(0xA0U + i + cfg->fselect);
    }

    /* Write Enable. */
    cmd.cfg   = WSPI_CFG_CMD_MODE_ONE_LINE |
                WSPI_CFG_ADDR_MODE_NONE |
                WSPI_CFG_DATA_MODE_NONE;
    cmd.cmd   = QSPI_CMD_WRITE_ENABLE;
    cmd.addr  = 0U;
    cmd.alt   = 0U;
    cmd.dummy = 0U;
    wspiCommand(&WSPID2, &cmd);

    /* Sector erase (4KB) at test address before writing. */
    chprintf(chp, "wspi: [4-line] sector erase at 0x%06x...\r\n",
             QSPI_TEST_ADDR);
    cmd.cfg   = WSPI_CFG_CMD_MODE_ONE_LINE |
                WSPI_CFG_ADDR_MODE_ONE_LINE |
                WSPI_CFG_DATA_MODE_NONE |
                WSPI_CFG_ADDR_SIZE_24;
    cmd.cmd   = QSPI_CMD_SECTOR_ERASE;
    cmd.addr  = QSPI_TEST_ADDR;
    cmd.alt   = 0U;
    cmd.dummy = 0U;
    wspiCommand(&WSPID2, &cmd);
    chThdSleepMilliseconds(50);

    /* Write Enable. */
    cmd.cfg   = WSPI_CFG_CMD_MODE_ONE_LINE |
                WSPI_CFG_ADDR_MODE_NONE |
                WSPI_CFG_DATA_MODE_NONE;
    cmd.cmd   = QSPI_CMD_WRITE_ENABLE;
    cmd.addr  = 0U;
    cmd.alt   = 0U;
    cmd.dummy = 0U;
    wspiCommand(&WSPID2, &cmd);

    /* Quad Page Program (0x32): cmd=1-line, addr=4-line, data=4-line. */
    chprintf(chp, "wspi: [4-line] writing %u bytes...\r\n", QSPI_TEST_LEN);
    cmd.cfg   = WSPI_CFG_CMD_MODE_ONE_LINE |
                WSPI_CFG_ADDR_MODE_ONE_LINE |
                WSPI_CFG_DATA_MODE_FOUR_LINES |
                WSPI_CFG_ADDR_SIZE_24;
    cmd.cmd   = 0x32U;  /* Quad Page Program */
    cmd.addr  = QSPI_TEST_ADDR;
    cmd.alt   = 0U;
    cmd.dummy = 0U;
    wspiSend(&WSPID2, &cmd, QSPI_TEST_LEN, wr_buf);
    chThdSleepMilliseconds(50);

    /* Read back with 4-line quad read to verify. */
    memset(rd_buf, 0, sizeof(rd_buf));
    cmd.cfg   = WSPI_CFG_CMD_MODE_ONE_LINE |
                WSPI_CFG_ADDR_MODE_FOUR_LINES |
                WSPI_CFG_DATA_MODE_FOUR_LINES |
                WSPI_CFG_ADDR_SIZE_24;
    cmd.cmd   = QSPI_CMD_FAST_READ_QUAD;
    cmd.addr  = QSPI_TEST_ADDR;
    cmd.alt   = 0U;
    cmd.dummy = 6U;
    wspiReceive(&WSPID2, &cmd, QSPI_TEST_LEN, rd_buf);

    /* Verify. */
    chprintf(chp, "wspi: [4-line] verify:");
    for (i = 0; i < QSPI_TEST_LEN; i++) {
      chprintf(chp, " %02x", rd_buf[i]);
    }
    chprintf(chp, "\r\n");

    if (memcmp(wr_buf, rd_buf, QSPI_TEST_LEN) == 0) {
      chprintf(chp, "wspi: [4-line] write+verify PASSED (%u bytes match)\r\n",
               QSPI_TEST_LEN);
    }
    else {
      for (i = 0; i < QSPI_TEST_LEN; i++) {
        if (wr_buf[i] != rd_buf[i]) {
          break;
        }
      }
      chprintf(chp, "wspi: [4-line] write+verify FAILED at offset %u"
               " (expected 0x%02x, got 0x%02x)\r\n",
               i, wr_buf[i], rd_buf[i]);
    }
    #undef QSPI_TEST_ADDR
    #undef QSPI_TEST_LEN
  }

  /* Memory-mapped read. */
  {
    uint8_t *map_addr = NULL;
    uint8_t  map_buf[16];
    uint32_t i;
    cmd.cfg   = WSPI_CFG_CMD_MODE_ONE_LINE |
                WSPI_CFG_ADDR_MODE_FOUR_LINES |
                WSPI_CFG_DATA_MODE_FOUR_LINES |
                WSPI_CFG_ADDR_SIZE_24;
    cmd.cmd   = QSPI_CMD_FAST_READ_QUAD;
    cmd.addr  = 0x000000U;
    cmd.alt   = 0U;
    cmd.dummy = 6U;
    wspiMapFlash(&WSPID2, &cmd, &map_addr);
    chprintf(chp, "wspi: mapped at 0x%p\r\n", (void *)map_addr);
    for (i = 0; i < sizeof(map_buf); i++) {
      map_buf[i] = map_addr[i];
      // chprintf(chp, "%x ",*(uint8_t *)(0x90000000+i));
    }
    chprintf(chp, "\n");
    wspiUnmapFlash(&WSPID2);
    chprintf(chp, "wspi: [mmap] read:");
    for (i = 0; i < sizeof(map_buf); i++) {
      chprintf(chp, " %02x", map_buf[i]);
    }
    chprintf(chp, "\r\n");
  }

  wspiStop(&WSPID2);
  chprintf(chp, "wspi: driver stopped\r\n");
}

static void cmd_wspi(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  /*----------------------------------------------------------------------*/
  /* 1. Configure QSPI2 GPIO pins (primary + secondary flash).           */
  /*    Primary:   SCK=PF6(AF4)  CS=PF1(AF5)  IO0=PF2(AF5)             */
  /*               IO1=PF9(AF4)  IO2=PF10(AF4) IO3=PE15(AF7)           */
  /*    Secondary: SCK shared,  CSXN=PC1(AF10)                           */
  /*               IOX0=PC2(AF10) IOX1=PC3(AF10)                        */
  /*               IOX2=PB13(AF11) IOX3=PB14(AF11)                      */
  /*----------------------------------------------------------------------*/
  /* Primary flash pins. */
  palSetPadMode(GPIOF,  6, PAL_CH32_ALTERNATE_PUSHPULL(4));   /* SCK   */
  palSetPadMode(GPIOF,  1, PAL_CH32_ALTERNATE_PUSHPULL(5));   /* CS    */
  palSetPadMode(GPIOF,  2, PAL_CH32_ALTERNATE_PUSHPULL(5));   /* IO0   */
  palSetPadMode(GPIOF,  9, PAL_CH32_ALTERNATE_PUSHPULL(4));   /* IO1   */
  palSetPadMode(GPIOF, 10, PAL_CH32_ALTERNATE_PUSHPULL(4));   /* IO2   */
  palSetPadMode(GPIOE, 15, PAL_CH32_ALTERNATE_PUSHPULL(7));   /* IO3   */
  /* Secondary flash pins. */
  palSetPadMode(GPIOC,  1, PAL_CH32_ALTERNATE_PUSHPULL(10));  /* CSXN  */
  palSetPadMode(GPIOC,  2, PAL_CH32_ALTERNATE_PUSHPULL(10));  /* IOX0  */
  palSetPadMode(GPIOC,  3, PAL_CH32_ALTERNATE_PUSHPULL(10));  /* IOX1  */
  palSetPadMode(GPIOB, 13, PAL_CH32_ALTERNATE_PUSHPULL(11));  /* IOX2  */
  palSetPadMode(GPIOB, 14, PAL_CH32_ALTERNATE_PUSHPULL(11));  /* IOX3  */

  /*----------------------------------------------------------------------*/
  /* 2. Test primary flash (NCS0).                                        */
  /*----------------------------------------------------------------------*/
  wspi_test_one_flash(chp, &wspi2_pri_cfg, "PRIMARY flash (NCS0)");

  /*----------------------------------------------------------------------*/
  /* 3. Test secondary flash (NCS1).                                      */
  /*----------------------------------------------------------------------*/
  wspi_test_one_flash(chp, &wspi2_sec_cfg, "SECONDARY flash (NCS1)");

  chprintf(chp, "wspi: all tests complete\r\n");
}

/*===========================================================================*/
/* Software reset shell command.                                              */
/*===========================================================================*/

static void cmd_reset(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  chprintf(chp, "reset: rebooting...\r\n");
  chThdSleepMilliseconds(100);
  NVIC_SystemReset();
}

static const ShellCommand shell_commands[] = {
  {"sdio", cmd_sdio},
  {"wspi", cmd_wspi},
  {"reset", cmd_reset},
  {NULL, NULL}
};

static const ShellConfig shell_cfg = {
  (BaseSequentialStream *)&SDU1,
  shell_commands
};


int main(void)
{

    /*
     * System initializations.
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
     * Normal main() thread activity, in this demo it spawns a shell
     * when USB is active.
     */
    while (true)
    {
        if (SDU1.config->usbp->state == USB_ACTIVE)
        {
            thread_t *shelltp =
                chThdCreateFromHeap(NULL, SHELL_WA_SIZE, "shell",
                                    NORMALPRIO + 1, shellThread,
                                    (void *)&shell_cfg);
            chThdWait(shelltp);
        }

        chThdSleepMilliseconds(500);
    }
}
