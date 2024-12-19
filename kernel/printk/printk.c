#include "printk/printk.h"
#include "plic/cpu.h"
#include "uart/uart.h"
#include <stdarg.h>

void printk(const char *fmt, ...)
{
	size_t mtime_value = *(volatile size_t *) __mtime_addr;
	size_t elapsed_us = mtime_value / (__ticks_per_second / __us_per_second);

	/* Maybe move out timer code somewhere. */
	double elapsed_sec = (double) elapsed_us / 1000000;

	kprintf("\r[ %5f ] ", elapsed_sec);

	va_list args;
	va_start(args, fmt);
	kvprintf(fmt, args);
	va_end(args);
}