#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "vga.h"
#include "lib/stdlib.h"

void kprint(const char* data)
{
	while (*data)
		vga_putchar(*data++);
}

void kernel_main(void)
{
	/* Initialize terminal interface */
	vga_init();

	/* Newline support is left as an exercise. */
	kprint("Hello world\n");
	kprint("Message 1\n");
	kprint("Message 2\n");
	kprint("Message 3\n");
	kprint("Message 4\n");
	kprint("Message 5\n");
	kprint("Message 6\n");
	kprint("Message 7\n");
	kprint("Message 8\n");
	kprint("Message 9\n");
	kprint("Message __10\n");
	kprint("Message 11\n");
	kprint("Message __12\n");
	kprint("Message 13\n");
	kprint("Message __14\n");
	kprint("Message 15\n");
	kprint("Message __16\n");
	kprint("Message 17\n");
	kprint("Message ______18\n");
	kprint("Message 19\n");
	kprint("Message _____20\n");
	kprint("Message 21\n");
	kprint("Message ____22\n");
	kprint("Message 23\n");
	kprint("Message ___24\n");
	kprint("Message 25\n");
	kprint("Message __26\n");
	kprint("Message 27\n");
	kprint("Message __28\n");
}