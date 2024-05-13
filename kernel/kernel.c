#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "vga.h"
#include "lib/stdlib.h"

void krpint(const char* data)
{
	vga_write(data, strlen(data));
}

void kernel_main(void)
{
	/* Initialize terminal interface */
	vga_init();

	/* Newline support is left as an exercise. */
	krpint("Hello, kernel World\nNewline\nNewline 2");
}