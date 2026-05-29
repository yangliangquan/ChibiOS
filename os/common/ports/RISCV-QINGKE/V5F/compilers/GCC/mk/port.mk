# List of the ChibiOS/RT RISCV V5F generic port files.
#
# SKELETON: Update paths once V5F-specific files are created.

PORTSRC = $(CHIBIOS)/os/common/ports/RISCV-QINGKE/V5F/chcore.c

PORTINC = $(CHIBIOS)/os/common/ports/RISCV-QINGKE/V5F \
          $(CHIBIOS)/os/common/ports/RISCV-QINGKE/V5F/compilers/GCC

# Shared variables
ALLXASMSRC += $(PORTASM)
ALLCSRC    += $(PORTSRC)
ALLINC     += $(PORTINC)
