##############################################################################
# Top-level Makefile for RT-CH32H417-DUAL
#
# Orchestrates building Core0 (V3F) and Core1 (V5F) independently,
# then merges the resulting HEX files into a single flash image.
##############################################################################

.PHONY: all cores clean core0 core1

all: cores merge

cores: core0 core1

core0:
	@echo ""
	@echo "========================================"
	@echo "  Building Core0 (V3F / Primary Core)"
	@echo "========================================"
	$(MAKE) -C$(CURDIR)/core0 "TOOLCHAIN_PATH=${TOOLCHAIN_PATH}"

core1:
	@echo ""
	@echo "========================================"
	@echo "  Building Core1 (V5F / Secondary Core)"
	@echo "========================================"
	$(MAKE) -C$(CURDIR)/core1 "TOOLCHAIN_PATH=${TOOLCHAIN_PATH}"

merge:
	@echo ""
	@echo "========================================"
	@echo "  Merging HEX files..."
	@echo "========================================"
	python3 merge_hex.py

clean:
	$(MAKE) -Ccore0 clean
	$(MAKE) -Ccore1 clean
	rm -f combined.hex
	@echo ""
	@echo "Clean done."
	@echo ""
