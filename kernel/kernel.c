#include "cpu/isr.h"
#include "cpu/timer.h"
#include "lib/string.h"
#include "lib/mem.h"
#include "drivers/vga.h"
#include "drivers/ata.h"
#include "fs.h"

void help()
{
	kprint("Ready for input. Commands:\n"
		"DIE       - to halt the CPU \n"
		"ATA RESET - to do software reset on ATA Primary drive \n"
		"FS INIT   - to init filesystem (Work in Progress!)\n"
		"CLEAR     - clear screen\n"
		"HELP      - print help\n"
		"> "
	);	
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
	kprint("\n> ");
}
