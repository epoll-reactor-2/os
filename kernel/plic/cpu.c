#include <stddef.h>
#include "plic/cpu.h"
#include "common/common.h"

// Set timer interrupt to fire `us` microseconds from now
void set_timer_interrupt_delay_us(size_t us)
{
	*(size_t *)__mtimecmp_addr = *(size_t *)__mtime_addr + us * (__ticks_per_second / __us_per_second);
}
