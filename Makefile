##################################
# Variables                      #
##################################

# +----------+-----------------------+
# | Arch     | i686-elf-*            |
# | Ubuntu   | i686-linux-gnu-*      |
# +----------+-----------------------+

AS 		= i686-linux-gnu-as
CC 		= i686-linux-gnu-gcc-12
KERNEL_BIN	= kernel.elf
LINK_SRC	= kernel/link.ld
CFLAGS		= -std=gnu99 -ffreestanding -O2 -nostdlib -I.
WARN_CFLAGS	= -Wall -Wextra

SRC_DIR		= kernel lib
SRC 		= $(shell find $(SRC_DIR) -name '*.c')
OBJ 		= $(SRC:.c=.o)

ASM_SRC		= $(shell find $(SRC_DIR) -name '*.S')
ASM_OBJ		= $(ASM_SRC:.S=.o)

AS_LABEL	:= "     AS  "
CC_LABEL	:= "     CC  "
LD_LABEL	:= "     LD  "

##################################
# Targets                        #
##################################

all: $(KERNEL_BIN)

# C sources
%.o: %.c
	@echo $(CC_LABEL) $(@F)
	@$(CC) -c $(CFLAGS) $(WARN_CFLAGS) $^ -o $@

# Assembly sources
%.o: %.S
	@echo $(AS_LABEL) $(@F)
	@$(AS) $^ -o $@

# Link C and Assembly files into single kernel image.
$(KERNEL_BIN): $(OBJ) $(ASM_OBJ)
	@echo $(LD_LABEL) $(@F)
	@$(CC) -T $(LINK_SRC) -o $(KERNEL_BIN) $(CFLAGS) $(OBJ) $(ASM_OBJ) -lgcc

##################################
# Phony targets                  #
##################################

.PHONY: clean
clean:
	@rm -rf $(OBJ) $(ASM_OBJ) $(KERNEL_BIN)

.PHONY: run
run:
	qemu-system-i386 -kernel $(KERNEL_BIN)