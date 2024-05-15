#include "cpu/isr.h"
#include "cpu/timer.h"
#include "lib/string.h"
#include "lib/mem.h"
#include "kernel/compiler.h"
#include "drivers/vga.h"
#include "drivers/ata.h"
#include "fs.h"

static void cmd_die()
{
	kprint("Stopping the CPU\n");
	asm volatile("hlt");
}

static void cmd_ata_reset()
{
	kprint("ATA software is reset");
	ata_init();	
}

static void cmd_fs_init()
{
	fs_init();
}

static void cmd_clear()
{
	vga_init();
}

static void help();
static void cmd_help()
{
	help();
}

struct command {
	void	(*routine)();
	char	cmd[64];
	char	desc[64];
};

static struct command commands[] = {
	{ cmd_die,		"DIE", 		"halt the CPU" 		},
	{ cmd_ata_reset, 	"ATA RESET", 	"ATA reset" 		},
	{ cmd_fs_init,		"FS INIT", 	"init filesystem"	},
	{ cmd_clear,		"CLEAR", 	"clear screen" 		},
	{ cmd_help,		"HELP", 	"print help" 		},
};

void help()
{
	kprint("Ready for input. Commands:\n");

	for (u32 i = 0; i < __array_size(commands); ++i) {
		struct command *e = &commands[i];

		kprint(e->cmd);
		kprint(" - ");
		kprint(e->desc);
		kprint("\n");
	}
}

void kernel_main()
{
	isr_install();
	irq_install();

	vga_init();

	help();
}

void user_input(char *input)
{
	if (strcmp(input, "DIE") == 0) {
		kprint("Stopping the CPU\n");
		asm volatile("hlt");
	} else if (strcmp(input, "ATA RESET") == 0) {
		kprint("ATA software is reset");
		ata_init();
	} else if (strcmp(input, "FS INIT") == 0) {
		fs_init();
	} else if (strcmp(input, "CLEAR") == 0) {
		vga_init();
	} else if (strcmp(input, "HELP") == 0) {
		help();
	} else {
		kprint("Command not found.");
	}

	// int found = 0;
// 
	// for (u32 i = 0; i < __array_size(commands); ++i) {
		// struct command *e = &commands[i];
// 
		// if (strcmp(input, e->cmd) == 0) {
			// found = 1;
			// e->routine();
		// }
	// }
// 
	// if (!found)
		// kprint("Command not found\n");

	kprint("\n> ");
}
