#include "drivers/mouse.h"
#include "drivers/vga.h"
#include "cpu/isr.h"
#include "cpu/type.h"
#include "cpu/ports.h"
#include "kernel/compiler.h"

#define MOUSE_IRQ 	12

#define MOUSE_PORT	0x60
#define MOUSE_STATUS	0x64
#define MOUSE_ABIT	0x02
#define MOUSE_BBIT	0x01
#define MOUSE_WRITE	0xD4
#define MOUSE_F_BIT	0x20
#define MOUSE_V_BIT	0x08

static void mouse_wait(u8 a_type)
{
	u32 timeout = 100000;
	if (!a_type) {
		while (--timeout) {
			if ((port_byte_in(MOUSE_STATUS) & MOUSE_BBIT) == 1) {
				return;
			}
		}
		kprint("mouse timeout\n");
		return;
	} else {
		while (--timeout) {
			if (!((port_byte_in(MOUSE_STATUS) & MOUSE_ABIT))) {
				return;
			}
		}
		kprint("mouse timeout\n");
		return;
	}
}

static void mouse_write(u8 write)
{
	port_byte_out(MOUSE_STATUS, MOUSE_WRITE);
	port_byte_out(MOUSE_PORT, write);
}

static u8 mouse_read()
{
	mouse_wait(0);
	return port_byte_in(MOUSE_PORT);
}

static void mouse_callback(__unused struct registers regs)
{
	kprint("Mouse trigger\n");

	/* u8 scancode = */ port_byte_in(0x60);
}

void mouse_init()
{
	mouse_wait(1);
	port_byte_out(MOUSE_STATUS, 0xA8);
	mouse_wait(1);
	port_byte_out(MOUSE_STATUS, 0x20);
	mouse_wait(0);
	u8 status = port_byte_in(0x60) | 2;
	mouse_wait(1);
	port_byte_out(MOUSE_STATUS, 0x60);
	mouse_wait(1);
	port_byte_out(MOUSE_PORT, status);
	mouse_write(0xF6);
	mouse_read();
	mouse_write(0xF4);
	mouse_read();

	register_interrupt_handler(IRQ12, mouse_callback);
}