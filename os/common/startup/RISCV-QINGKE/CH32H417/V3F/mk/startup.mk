# Startup build file for CH32H417 V3F core.
STARTUPASM = $(CHIBIOS)/os/common/startup/RISCV-QINGKE/CH32H417/V3F/crt0.S
STARTUPASM += $(CHIBIOS)/os/common/startup/RISCV-QINGKE/CH32H417/V3F/vector.S
STARTUPSRC = $(CHIBIOS)/os/common/startup/RISCV-QINGKE/compilers/GCC/crt1.c
STARTUPSRC += $(CHIBIOS)/os/common/ext/RISCV/QINGKE/Core/V3F/core_riscv.c

STARTUPINC = $(CHIBIOS)/os/common/portability/GCC \
             $(CHIBIOS)/os/common/startup/RISCV-QINGKE/CH32H417/V3F \
             $(CHIBIOS)/os/common/ext/WCH/CH32H417 \
             $(CHIBIOS)/os/common/ext/RISCV/QINGKE/Core/V3F

STARTUPLD  = $(CHIBIOS)/os/common/startup/RISCV-QINGKE/CH32H417/V3F

ALLXASMSRC += $(STARTUPASM)
ALLCSRC    += $(STARTUPSRC)
ALLINC     += $(STARTUPINC)
