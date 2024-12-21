#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>

/* I have left k' prefix intentionally. */

int kputchar(int);

int kputs(const char *);

void kvprintf(const char *, va_list);

void kprintf(const char *, ...);

int kgetchar(void);

#endif /* STDIO_H */