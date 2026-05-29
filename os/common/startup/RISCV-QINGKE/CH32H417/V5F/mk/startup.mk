# Startup build file for CH32H417 V5F core (SKELETON).
STARTUPASM = $(CHIBIOS)/os/common/startup/RISCV-QINGKE/CH32H417/V5F/crt0.S
STARTUPASM += $(CHIBIOS)/os/common/startup/RISCV-QINGKE/CH32H417/V5F/vector.S
STARTUPSRC = $(CHIBIOS)/os/common/startup/RISCV-QINGKE/compilers/GCC/crt1.c
STARTUPSRC += $(CHIBIOS)/os/common/ext/RISCV/QINGKE/Core/V5F/core_riscv.c

STARTUPINC = $(CHIBIOS)/os/common/portability/GCC \
             $(CHIBIOS)/os/common/startup/RISCV-QINGKE/CH32H417/V5F \
             $(CHIBIOS)/os/common/ext/WCH/CH32H417 \
             $(CHIBIOS)/os/common/ext/RISCV/QINGKE/Core/V5F

STARTUPLD  = $(CHIBIOS)/os/common/startup/RISCV-QINGKE/CH32H417/V5F

ALLXASMSRC += $(STARTUPASM)
ALLCSRC    += $(STARTUPSRC)
ALLINC     += $(STARTUPINC)
