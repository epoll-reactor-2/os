# Build
CC = riscv64-unknown-elf-gcc
GDB = riscv64-elf-gdb
CFLAGS = -ffreestanding -nostartfiles -nostdlib -nodefaultlibs
CFLAGS += -I kernel
CFLAGS += -O0 -g -Wl,--gc-sections -mcmodel=medany -march=rv64g
#CFLAGS += -Wl,--no-warn-rwx-segments
RUNTIME = kernel/asm/crt0.S
LINKER_SCRIPT = kernel/lds/riscv64-virt.ld
BUILD_DIR = os.release.riscv
KERNEL_IMAGE = $(BUILD_DIR)/kernel.riscv64

# QEMU
QEMU = qemu-system-riscv64
MACH = virt

SRC = $(shell find kernel misc -name '*.c')
OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

# Format
INDENT_FLAGS = -linux -brf -i2

all: dir fonts $(KERNEL_IMAGE)

fonts:
	make -C scripts

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

# To debug:
# qemu cmd with -S -s
# riscv64-elf-gdb -ex "target remote localhost:1234" -ex "symbol-file os.release.riscv/kernel.riscv64"

.PHONY: run
run: all
	$(QEMU) -machine $(MACH) -kernel $(KERNEL_IMAGE) -nographic -bios none -monitor none

.PHONY: run_gui
run_gui: all
	$(QEMU) -machine $(MACH) -kernel $(KERNEL_IMAGE) -display sdl -bios none -monitor none

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
