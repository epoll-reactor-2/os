#ifndef KERNEL_DRIVERS_VGA_H
#define KERNEL_DRIVERS_VGA_H

#include "cpu/type.h"

void vga_init();
void vga_put_byte(char c);
void vga_put_string(const char *mem);

#endif /* KERNEL_DRIVERS_VGA_H */