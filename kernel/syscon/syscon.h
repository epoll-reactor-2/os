#ifndef SYSCON_H
#define SYSCON_H

// "test" syscon-compatible device is at memory-mapped address 0x100000
// according to our device tree
#define __syscon_addr 0x100000

void poweroff(void);
void reboot(void);

#endif
