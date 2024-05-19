#include "cpu/isr.h"
#include "cpu/timer.h"
#include "lib/string.h"
#include "lib/mem.h"
#include "kernel/compiler.h"
#include "kernel/cmd.h"
#include "drivers/vga.h"
#include "drivers/ata.h"
#include "fs.h"

static void prompt()
{
	kprint("\n> ");
}

void kernel_main()
{
	isr_install();
	irq_install();

	// vga_init();

	cmd_help();
	prompt();
}

void user_input(char *input)
{
	if (!input)
		return;

#if 1
	if (!strcmp(input, "DIE")) {
		cmd_die();
	} else if (!strcmp(input, "ATA RESET")) {
		cmd_ata_reset();
	} else if (!strcmp(input, "FS INIT")) {
		cmd_fs_init();
	} else if (!strcmp(input, "CLEAR")) {
		cmd_clear();
	} else if (!strcmp(input, "HELP")) {
		cmd_help();
	} else {
		kprint("Command not found\n");
	}
#else
	/* There is a bug with `struct command` usage.
	   Depending on a toolchain (elf-* or linux-gnu-*),
	   this code works correctly or not. I guess I have
	   some misunderstanding about exact format of code that
	   compiler produces.

	   From the GDB session it follows that function pointer,
	   placed in a structure is not called properly and continues
	   to run instructions after function end. */
	int found = 0;

	for (u32 i = 0; i < __array_size(cmds); ++i) {
		struct command *e = &cmds[i];

		if (strcmp(input, e->cmd) == 0) {
			kprint(input);
			found = 1;
			e->routine();
		}
	}

	if (!found)
		kprint("Command not found\n");
#endif /* 0|1 */

	prompt();
}
