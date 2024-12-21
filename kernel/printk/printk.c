#include "printk/printk.h"
#include "plic/cpu.h"
#include "libc/stdio.h"
#include "config.h"
#include <stdarg.h>

void printk(const char *fmt, ...)
{
	/* Some illegal instruction..........
	   Also, we cannot print kernel panic with
	   printk, since there is problem with floats. */
#if CONFIG_FPU
	size_t mtime_value = *(volatile size_t *) __mtime_addr;
	size_t elapsed_us = mtime_value / (__ticks_per_second / __us_per_second);

	/* Maybe move out timer code somewhere. */
	double elapsed_sec = (double) elapsed_us / 1000000;

	kprintf("\r[ %5f ] ", elapsed_sec);
#else
	kprintf("\r[       ] ");
#endif /* CONFIG_FPU */

	va_list args;
	va_start(args, fmt);
	kvprintf(fmt, args);
	va_end(args);
}