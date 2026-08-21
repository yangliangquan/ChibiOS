# List of all the CH32H417 V3F platform files.

# Configuration files directory
ifeq ($(CONFDIR),)
  CONFDIR = .
endif

HALCONF := $(strip $(shell cat $(CONFDIR)/halconf.h | egrep -e "\#define"))
MCUCONF := $(strip $(shell cat $(CONFDIR)/mcuconf.h | egrep -e "\#define"))

# Required include directories.

# Select SDMMC or SDIO LLD include path
ifneq ($(findstring CH32_SDC_USE_SDIO TRUE,$(MCUCONF)),)
CH32_SDC_LLDINC = ${CHIBIOS}/os/hal/ports/CH32/LLD/SDIOv1
else
CH32_SDC_LLDINC = ${CHIBIOS}/os/hal/ports/CH32/LLD/SDMMCv1
endif

# Select USBHS or OTG LLD include path
ifneq ($(findstring CH32_OTG_USE_USB1 TRUE,$(MCUCONF)),)
CH32_USB_LLDINC = ${CHIBIOS}/os/hal/ports/CH32/LLD/OTGv1
else
CH32_USB_LLDINC = ${CHIBIOS}/os/hal/ports/CH32/LLD/USBv1
endif

PLATFORMINC = ${CHIBIOS}/os/hal/ports/CH32/CH32H417 \
              ${CHIBIOS}/os/hal/ports/CH32/CH32H417/LLD/STv3f \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/DMAv1 \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/ADCv1 \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/CANv1 \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/CRYPv1 \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/DACv1 \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/EFLv1 \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/TIMv1 \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/I2Cv1 \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/SPIv1 \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/MACv1 \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/GPIOv1 \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/RNGv1 \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/RTCv1 \
              $(CH32_SDC_LLDINC) \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/USARTv1 \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/WDGv1 \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/QULDSPIv1 \
              $(CH32_USB_LLDINC)

# Optional or full set of platform files.
ifeq ($(USE_SMART_BUILD),yes)

PLATFORMSRC := ${CHIBIOS}/os/hal/ports/CH32/CH32H417/hal_lld.c
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/CH32H417/LLD/STv3f/hal_st_lld.c
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/DMAv1/ch32_dma.c
ifneq ($(findstring HAL_USE_ADC TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/ADCv1/hal_adc_lld.c
endif
ifneq ($(findstring HAL_USE_CAN TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/CANv1/hal_can_lld.c
endif
ifneq ($(findstring HAL_USE_CRY TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/CRYPv1/hal_crypto_lld.c
endif
ifneq ($(findstring HAL_USE_DAC TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/DACv1/hal_dac_lld.c
endif
ifneq ($(findstring HAL_USE_EFL TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/EFLv1/hal_efl_lld.c
endif
ifneq ($(findstring HAL_USE_GPT TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/TIMv1/hal_gpt_lld.c
endif
ifneq ($(findstring HAL_USE_I2C TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/I2Cv1/hal_i2c_lld.c
endif
ifneq ($(findstring HAL_USE_I2S TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/SPIv1/hal_i2s_lld.c
endif
ifneq ($(findstring HAL_USE_ICU TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/TIMv1/hal_icu_lld.c
endif
ifneq ($(findstring HAL_USE_MAC TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/MACv1/hal_mac_lld.c
endif
ifneq ($(findstring HAL_USE_PAL TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/GPIOv1/hal_pal_lld.c
endif
ifneq ($(findstring HAL_USE_PWM TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/TIMv1/hal_pwm_lld.c
endif
ifneq ($(findstring HAL_USE_RTC TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/RTCv1/hal_rtc_lld.c
endif
ifneq ($(findstring HAL_USE_SDC TRUE,$(HALCONF)),)
ifneq ($(findstring CH32_SDC_USE_SDIO TRUE,$(MCUCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/SDIOv1/hal_sdc_lld.c
else
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/SDMMCv1/hal_sdc_lld.c
endif
endif
ifneq ($(findstring HAL_USE_SERIAL TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/USARTv1/hal_serial_lld.c
endif
ifneq ($(findstring HAL_USE_SIO TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/USARTv1/hal_sio_lld.c
endif
ifneq ($(findstring HAL_USE_SPI TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/SPIv1/hal_spi_lld.c
endif
ifneq ($(findstring HAL_USE_TRNG TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/RNGv1/hal_trng_lld.c
endif
ifneq ($(findstring HAL_USE_UART TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/USARTv1/hal_uart_lld.c
endif
ifneq ($(findstring HAL_USE_USB TRUE,$(HALCONF)),)
ifneq ($(findstring CH32_OTG_USE_USB1 TRUE,$(MCUCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/OTGv1/hal_usb_lld.c
else
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/USBv1/hal_usb_lld.c
endif
endif
ifneq ($(findstring HAL_USE_WDG TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/WDGv1/hal_wdg_lld.c
endif
ifneq ($(findstring HAL_USE_WSPI TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS}/os/hal/ports/CH32/LLD/QULDSPIv1/hal_wspi_lld.c
endif
else
# Select SDMMC or SDIO LLD src path
ifneq ($(findstring CH32_SDC_USE_SDIO TRUE,$(MCUCONF)),)
CH32_SDC_SRC = ${CHIBIOS}/os/hal/ports/CH32/LLD/SDIOv1/hal_sdc_lld.c
else
CH32_SDC_SRC = ${CHIBIOS}/os/hal/ports/CH32/LLD/SDMMCv1/hal_sdc_lld.c
endif

# Select USBHS or OTG LLD src path
ifneq ($(findstring CH32_OTG_USE_USB1 TRUE,$(MCUCONF)),)
CH32_USB_SRC = ${CHIBIOS}/os/hal/ports/CH32/LLD/OTGv1/hal_usb_lld.c
else
CH32_USB_SRC = ${CHIBIOS}/os/hal/ports/CH32/LLD/USBv1/hal_usb_lld.c
endif

PLATFORMSRC = ${CHIBIOS}/os/hal/ports/CH32/CH32H417/hal_lld.c \
              ${CHIBIOS}/os/hal/ports/CH32/CH32H417/LLD/STv3f/hal_st_lld.c \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/DMAv1/ch32_dma.c \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/ADCv1/hal_adc_lld.c \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/CANv1/hal_can_lld.c \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/CRYPv1/hal_crypto_lld.c \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/DACv1/hal_dac_lld.c \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/EFLv1/hal_efl_lld.c \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/TIMv1/hal_gpt_lld.c \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/I2Cv1/hal_i2c_lld.c \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/SPIv1/hal_i2s_lld.c \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/TIMv1/hal_icu_lld.c \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/MACv1/hal_mac_lld.c \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/GPIOv1/hal_pal_lld.c \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/TIMv1/hal_pwm_lld.c \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/RTCv1/hal_rtc_lld.c \
              $(CH32_SDC_SRC)\
              ${CHIBIOS}/os/hal/ports/CH32/LLD/USARTv1/hal_serial_lld.c \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/USARTv1/hal_sio_lld.c \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/SPIv1/hal_spi_lld.c \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/RNGv1/hal_trng_lld.c \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/USARTv1/hal_uart_lld.c \
              $(CH32_USB_SRC)\
              ${CHIBIOS}/os/hal/ports/CH32/LLD/WDGv1/hal_wdg_lld.c \
              ${CHIBIOS}/os/hal/ports/CH32/LLD/QULDSPIv1/hal_wspi_lld.c

endif
# Shared variables
ALLCSRC += $(PLATFORMSRC)
ALLINC  += $(PLATFORMINC)
