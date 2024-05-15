#ifndef KERNEL_DRIVERS_VGA_H
#define KERNEL_DRIVERS_VGA_H

/* Public kernel API */
void vga_init();
void kprint_at(const char *message, int col, int row);
void kprint(const char *message);
void kprint_backspace();

#endif /* KERNEL_DRIVERS_VGA_H */