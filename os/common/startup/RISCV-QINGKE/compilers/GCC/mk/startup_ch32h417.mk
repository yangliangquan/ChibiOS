# List of the ChibiOS generic CH32H417 file.
STARTUPASM = $(CHIBIOS)/os/common/startup/RISCV-QINGKE/compilers/GCC/crt0.S
STARTUPASM += $(CHIBIOS)/os/common/startup/RISCV-QINGKE/compilers/GCC/vector.S
STARTUPSRC = $(CHIBIOS)/os/common/startup/RISCV-QINGKE/compilers/GCC/crt1.c
STARTUPSRC += $(CHIBIOS)/os/common/ext/RISCV/QINGKE/Core/core_riscv.c

STARTUPINC = $(CHIBIOS)/os/common/portability/GCC \
             ${CHIBIOS}/os/common/startup/RISCV-QINGKE/devices/CH32H417 \
             $(CHIBIOS)/os/common/ext/WCH/CH32H417 \
             $(CHIBIOS)/os/common/ext/RISCV/QINGKE/Core


STARTUPLD  = ${CHIBIOS}/os/common/startup/RISCV-QINGKE/compilers/GCC/ld

# Shared variables
ALLXASMSRC += $(STARTUPASM)
ALLCSRC    += $(STARTUPSRC)
ALLINC     += $(STARTUPINC)
