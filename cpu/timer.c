#include "timer.h"
#include "ports.h"
#include "isr.h"
#include "kernel/compiler.h"
#include "lib/stdio.h"
#include "lib/string.h"

static u32 tick = 0;

__unused static void kprint_ticks()
{
	kprintf("%d\n", tick);
}

static void timer_callback(__unused struct stack_frame frame)
{
	tick++;
	// kprint_ticks();
}

void timer_init(u32 freq)
{
	/* Install the function we just wrote */
	irq_install_handler(IRQ0, timer_callback);

	/* Get the PIT value: hardware clock at 1193180 Hz */
	u32 divisor = 1193180 / freq;
	u8 low  = (u8)(divisor & 0xFF);
	u8 high = (u8)( (divisor >> 8) & 0xFF);
	/* Send the command */
	port_byte_out(0x43, 0x36); /* Command port */
	port_byte_out(0x40, low);
	port_byte_out(0x40, high);
}