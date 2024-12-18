#include "printk/printk.h"
#include "uart/uart.h"
#include <stdarg.h>

void printk(const char *fmt, ...)
{
	kprintf("[0.000000] ");

	va_list args;
	va_start(args, fmt);
	kvprintf(fmt, args);
	va_end(args);
}