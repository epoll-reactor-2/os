#include "kernel/cmd.h"
#include "kernel/compiler.h"
#include "cpu/type.h"
#include "drivers/ata.h"
#include "drivers/vga.h"
#include "lib/stdio.h"

void cmd_die()
{
	kprint("Stopping the CPU\n");
	__asm__ __volatile__ ("hlt");
}

void cmd_ata_reset()
{
	kprint("ATA software is reset");
	ata_init();	
}

void cmd_clear()
{
	vga_init();
}

void cmd_help()
{
	kprint("Ready for input. Commands:\n");

	for (u32 i = 0; i < __array_size(cmds); ++i) {
		const struct command *e = &cmds[i];

		kprintf("%s - %s\n", e->cmd, e->desc, "Abc");
	}
}

struct command cmds[] = {
	{ cmd_die,		"DIE", 		"halt the CPU" 		},
	{ cmd_ata_reset, 	"ATA RESET", 	"ATA reset" 		},
	{ cmd_clear,		"CLEAR", 	"clear screen" 		},
	{ cmd_help,		"HELP", 	"print help" 		},
};