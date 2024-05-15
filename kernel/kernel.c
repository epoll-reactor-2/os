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

	vga_init();

	cmd_help();
	prompt();
}

void user_input(char *input)
{
	int found = 0;

	for (u32 i = 0; i < __array_size(cmds); ++i) {
		struct command *e = &cmds[i];

		if (strcmp(input, e->cmd) == 0) {
			found = 1;
			e->routine();
		}
	}

	if (!found)
		kprint("Command not found\n");

	prompt();
}
