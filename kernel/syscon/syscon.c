#include <stdint.h>
#include "syscon.h"
#include "../printk/printk.h"


/* These numbers are connected with DTS somehow. */

void poweroff(void)
{
	printk("Poweroff requested");
	*(uint32_t *) __syscon_addr = 0x5555;
}

void reboot(void)
{
	printk("Reboot requested");
	*(uint32_t *) __syscon_addr = 0x7777;
}
