#include <stdint.h>
#include "syscon.h"
#include "../uart/uart.h"


/* These numbers are connected with DTS somehow. */

void poweroff(void)
{
	kputs("Poweroff requested");
	*(uint32_t *) __syscon_addr = 0x5555;
}

void reboot(void)
{
	kputs("Reboot requested");
	*(uint32_t *) __syscon_addr = 0x7777;
}
