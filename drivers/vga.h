#ifndef KERNEL_DRIVERS_VGA_H
#define KERNEL_DRIVERS_VGA_H

void vga_init();
/* TODO: Separate printk from VGA driver. Make krpintf. */
void kprint(const char *message);

#endif /* KERNEL_DRIVERS_VGA_H */