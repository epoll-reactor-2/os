# Build
CC = riscv64-unknown-elf-gcc
CFLAGS = -ffreestanding -nostartfiles -nostdlib -nodefaultlibs
CFLAGS += -I kernel
CFLAGS += -g -Wl,--gc-sections -mcmodel=medany -march=rv64g
#CFLAGS += -Wl,--no-warn-rwx-segments
RUNTIME = kernel/asm/crt0.S
LINKER_SCRIPT = kernel/lds/riscv64-virt.ld
BUILD_DIR = os.release.riscv
KERNEL_IMAGE = $(BUILD_DIR)/kernel.riscv64

# QEMU
QEMU = qemu-system-riscv64
MACH = virt
RUN = $(QEMU) -nographic -machine $(MACH)
RUN += -bios none -kernel $(KERNEL_IMAGE)

SRC = $(shell find kernel misc -name '*.c')
OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

# Format
INDENT_FLAGS = -linux -brf -i2

all: dir $(KERNEL_IMAGE)

dir:
	@mkdir -p $(BUILD_DIR)

$(KERNEL_IMAGE): $(OBJ) $(RUNTIME)
	@echo "  LD " $@
	@$(CC) $^ $(CFLAGS) -T $(LINKER_SCRIPT) -o $@

$(BUILD_DIR)/%.o: %.c
	@# This restores directory structure in build folder.
	@mkdir -p $(dir $@)
	@echo "  CC " $^
	@$(CC) -c $< $(CFLAGS) -o $@

.PHONY: run
run: all
	$(RUN)

.PHONY: debug
debug: all
	$(RUN) -s -S

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
