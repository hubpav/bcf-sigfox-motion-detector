-include sdk/Makefile.mk

.PHONY: all
all: sdk debug

.PHONY: sdk
sdk:
	@if [ ! -f sdk/Makefile.mk ]; then \
		echo "Initializing git submodules..."; \
		git submodule update --init; \
	fi

update:
	$(Q)git submodule update --remote --merge
