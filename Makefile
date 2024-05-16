SRC = $(wildcard kernel/*.c drivers/*.c cpu/*.c lib/*.c)
OBJ = $(SRC:.c=.o cpu/interrupt.o)

# +----------+-----------------------+
# | Arch     | i686-elf-*            |
# | Ubuntu   | i686-linux-gnu-*      |
# +----------+-----------------------+
CC = gcc
LD = gold
GDB = gdb
# -g: Use debugging symbols in gcc
CFLAGS = -g -ffreestanding -Wall -Wextra -fno-exceptions -m32 -I.

IMAGE_SIZE = 5M

AS_LABEL = "    AS "
CC_LABEL = "    CC "
LD_LABEL = "    LD "

$(info CC: $(CC), LD: $(LD))

# First rule is run by default
os-image.bin: boot/bootsect.bin kernel.bin
	cat $^ > os-image.bin & qemu-img resize -f raw os-image.bin $(IMAGE_SIZE)

# '--oformat binary' deletes all symbols as a collateral, so we don't need
# to 'strip' them manually on this case
kernel.bin: boot/kernel_entry.o $(OBJ)
	@echo $(LD_LABEL) $@
	@$(LD) -o $@ -Ttext 0x1000 $^ --oformat binary

# Used for debugging purposes
kernel.elf: boot/kernel_entry.o $(OBJ)
	@echo $(LD_LABEL) $<
	@$(LD) -o $@ -Ttext 0x1000 $^ 

run: os-image.bin
	qemu-system-i386 -drive if=ide,format=raw,file=os-image.bin,readonly=off

# Open the connection to qemu and load our kernel-object file with symbols
debug: os-image.bin kernel.elf
	qemu-system-i386 -s -S -drive if=ide,format=raw,file=os-image.bin,readonly=off -d guest_errors,int &
	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

# Generic rules for wildcards
# To make an object, always compile from its .c
%.o: %.c
	@echo $(CC_LABEL) $<
	@$(CC) $(CFLAGS) -ffreestanding -c $< -o $@

%.o: %.S
	@echo $(AS_LABEL) $<
	@nasm $< -f elf -o $@

%.bin: %.S
	@echo $(AS_LABEL) $<
	@nasm $< -f bin -o $@

clean:
	rm -rf *.bin *.dis *.o os-image.bin *.elf
	rm -rf kernel/*.o boot/*.bin drivers/*.o boot/*.o lib/*.o cpu/*.o
