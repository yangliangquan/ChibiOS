# List of the ChibiOS/RT RISCV V3F generic port files.
PORTSRC = $(CHIBIOS)/os/common/ports/RISCV-QINGKE/V3F/chcore.c

PORTINC = $(CHIBIOS)/os/common/ports/RISCV-QINGKE/V3F \
          $(CHIBIOS)/os/common/ports/RISCV-QINGKE/V3F/compilers/GCC

# Shared variables
ALLXASMSRC += $(PORTASM)
ALLCSRC    += $(PORTSRC)
ALLINC     += $(PORTINC)
