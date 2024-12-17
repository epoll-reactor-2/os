# Build
CC=riscv64-unknown-elf-gcc
CFLAGS=-ffreestanding -nostartfiles -nostdlib -nodefaultlibs
CFLAGS+=-g -Wl,--gc-sections -mcmodel=medany -march=rv64g
#CFLAGS+=-Wl,--no-warn-rwx-segments
RUNTIME=kernel/asm/crt0.S
LINKER_SCRIPT=kernel/lds/riscv64-virt.ld
KERNEL_IMAGE=kernel.riscv64

# QEMU
QEMU=qemu-system-riscv64
MACH=virt
RUN=$(QEMU) -nographic -machine $(MACH)
RUN+=-bios none -kernel $(KERNEL_IMAGE)

# Format
INDENT_FLAGS=-linux -brf -i2

all: uart printk syscon common mm plic process kernel_main
	$(CC) *.o $(RUNTIME) $(CFLAGS) -T $(LINKER_SCRIPT) -o $(KERNEL_IMAGE)

uart:
	$(CC) -c kernel/uart/uart.c $(CFLAGS) -o uart.o

printk:
	$(CC) -c kernel/printk/printk.c $(CFLAGS) -o printk.o

syscon:
	$(CC) -c kernel/syscon/syscon.c $(CFLAGS) -o syscon.o

common:
	$(CC) -c kernel/common/common.c $(CFLAGS) -o common.o

mm:
	$(CC) -c kernel/mm/page.c $(CFLAGS) -o page.o
	$(CC) -c kernel/mm/sv39.c $(CFLAGS) -o sv39.o
	$(CC) -c kernel/mm/kmem.c $(CFLAGS) -o kmem.o

plic:
	$(CC) -c kernel/plic/trap_frame.c $(CFLAGS) -o trap_frame.o
	$(CC) -c kernel/plic/cpu.c $(CFLAGS) -o cpu.o
	$(CC) -c kernel/plic/trap_handler.c $(CFLAGS) -o trap_handler.o

process:
	$(CC) -c kernel/process/syscall.c $(CFLAGS) -o syscall.o
	$(CC) -c kernel/process/process.c $(CFLAGS) -o process.o
	$(CC) -c kernel/process/sched.c $(CFLAGS) -o sched.o

kernel_main:
	$(CC) -c kernel/kernel_main.c $(CFLAGS) -o kernel_main.o

run: all
	$(RUN)

debug: all
	$(RUN) -s -S

format:
	find . -name '*.h' -exec indent $(INDENT_FLAGS) '{}' \;
	find . -name '*.c' -exec indent $(INDENT_FLAGS) '{}' \;
	find . -name '*.h' -exec sed -i -r 's/(0) (b[01]+)/\1\2/g' '{}' \;
	find . -name '*.c' -exec sed -i -r 's/(0) (b[01]+)/\1\2/g' '{}' \;
	echo "You should now \`make run\` to confirm the project still builds and runs correctly"

clean:
	rm -vf *.o
	rm -vf $(KERNEL_IMAGE)
	find . -name '*~' -exec rm -vf '{}' \;
