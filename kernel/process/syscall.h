#ifndef SYSCALL_H
#define SYSCALL_H

#include <stddef.h>
#include "../plic/trap_frame.h"

#define __syscall_exit		0
#define __syscall_test		1

// Defined in src/asm/crt0.s
size_t make_syscall(size_t);

size_t do_syscall(size_t, struct trap_frame *);

#endif
