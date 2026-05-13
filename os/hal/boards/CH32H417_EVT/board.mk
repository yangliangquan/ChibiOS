# List of all the board related files.
BOARDSRC = $(CHIBIOS)/os/hal/boards/CH32H417_EVT/board.c

# Required include directories
BOARDINC = $(CHIBIOS)/os/hal/boards/CH32H417_EVT

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
