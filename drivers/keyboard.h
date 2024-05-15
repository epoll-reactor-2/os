#ifndef KERNEL_DRIVERS_KEYBOARD_H
#define KERNEL_DRIVERS_KEYBOARD_H

#include "cpu/type.h"

#define BACKSPACE	0x0E
#define ENTER		0x1C
#define SC_MAX		57

void keyboard_init();

#endif /* KERNEL_DRIVERS_KEYBOARD_H */