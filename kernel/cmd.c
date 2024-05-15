#include "kernel/cmd.h"
#include "kernel/compiler.h"
#include "kernel/fs.h"
#include "cpu/type.h"
#include "drivers/ata.h"
#include "drivers/vga.h"

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

/* public */ void cmd_help()
{
	kprint("Ready for input. Commands:\n");

	for (u32 i = 0; i < __array_size(cmds); ++i) {
		const struct command *e = &cmds[i];

		kprint(e->cmd);
		kprint(" - ");
		kprint(e->desc);
		kprint("\n");
	}
}

struct command cmds[] = {
	{ cmd_die,		"DIE", 		"halt the CPU" 		},
	{ cmd_ata_reset, 	"ATA RESET", 	"ATA reset" 		},
	{ cmd_fs_init,		"FS INIT", 	"init filesystem"	},
	{ cmd_clear,		"CLEAR", 	"clear screen" 		},
	{ cmd_help,		"HELP", 	"print help" 		},
};