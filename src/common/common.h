#ifndef COMMON_H
#define COMMON_H

#include "../plic/cpu.h"
#include "../uart/uart.h"

#define __halt() ({					\
	__set_mie(0);					\
	asm volatile ("wfi");				\
})

#define __panic(format, ...) ({\
	kprintf("Kernel panic at %s:%d:\n" format,	\
		__FILE__,				\
		__LINE__ 				\
		__VA_OPT__(,) __VA_ARGS__ 		\
	);						\
	__halt();					\
})

#define __assert(condition, format, ...) ({		\
	if (!(condition))				\
		__panic("Failed asserting that %s\n" format, \
			#condition 			\
			__VA_OPT__(,) __VA_ARGS__	\
		);					\
})

int toupper(int);
char *strcpy(char *, const char *);

#endif
