# List of the ChibiOS/RT RISCV generic port files.
PORTSRC = $(CHIBIOS)/os/common/ports/RISCV-QINGKE/chcore.c

PORTINC = $(CHIBIOS)/os/common/ports/RISCV-QINGKE \
          $(CHIBIOS)/os/common/ports/RISCV-QINGKE/compilers/GCC

# Shared variables
ALLXASMSRC += $(PORTASM)
ALLCSRC    += $(PORTSRC)
ALLINC     += $(PORTINC)