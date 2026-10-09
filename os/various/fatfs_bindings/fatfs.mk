# FATFS files.
# NOTE: ff_memalloc/ff_memfree/get_fattime live in the ArduPilot
# hwdef/common tree (malloc.c/stm32_util.c); the ChibiOS
# fatfs_syscall.c variant would clash, so only the diskio shim and the
# FatFS core are built here.

FATFSSRC = $(CHIBIOS)/os/various/fatfs_bindings/fatfs_diskio.c \
           $(CHIBIOS)/ext/fatfs/source/ff.c \
           $(CHIBIOS)/ext/fatfs/source/ffunicode.c

FATFSINC = $(CHIBIOS)/ext/fatfs/source

# Shared variables
ALLCSRC += $(FATFSSRC)
ALLINC  += $(FATFSINC)
