#include "drivers/ps2_mouse.h"
#include "cpu/isr.h"
#include "cpu/type.h"
#include "cpu/ports.h"
#include "kernel/compiler.h"
#include "lib/stdio.h"
#include "lib/string.h"

enum {
	MOUSE_STATUS	= 0x64,
	MOUSE_ABIT	= 0x02,
	MOUSE_BBIT	= 0x01,
	MOUSE_WRITE	= 0xD4,
	MOUSE_F_BIT	= 0x20,
	MOUSE_V_BIT	= 0x08,
};

/* https://techdocs.altium.com/display/FPGA/PS2+Commands */
enum {
	PS2_CMD_RESET			= 0xFF,
	PS2_CMD_RESEND			= 0xFE,
	PS2_CMD_SET_DEFAULTS		= 0xF6,
	PS2_CMD_DISABLE_DATA_REPORTING	= 0xF5,
	PS2_CMD_ENABLE_DATA_REPORTING	= 0xF4,
	PS2_CMD_SET_SAMPLE_RATE		= 0xF3,
	PS2_CMD_GET_DEV_ID		= 0xF2,
	PS2_CMD_SET_REMOTE_MODE		= 0xF0,
	PS2_CMD_SET_WRAP_MODE		= 0xEE,
	PS2_CMD_RESET_WRAP_MODE		= 0xEC,
	PS2_CMD_READ_DATA		= 0xEB,
	PS2_CMD_SET_STREAM_MODE		= 0xEA,
	PS2_CMD_STATUS_REQ		= 0xE9,
	PS2_CMD_SET_RESOLUTION		= 0xE8,
	PS2_CMD_SET_SCALING_2_1		= 0xE7,
	PS2_CMD_SET_SCALING_1_1		= 0xE6,
};

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
	port_byte_out(PS2_PORT, write);
}

static u8 mouse_read()
{
	mouse_wait(0);
	return port_byte_in(PS2_PORT);
}

__unused static int mouse_report_click(s8 bytes[3])
{
	if (bytes[0] & 0x80 || bytes[0] & 0x40) {
		/* X/Y overflow. Bad packet. */
		return -1;
	}
	if (bytes[0] & 0x01) {
		kprint("Left click\n");
	}
	if (bytes[0] & 0x02) {
		kprint("Right click\n");
	}
	if (bytes[0] & 0x04) {
		kprint("Middle click\n");
	}

	return 0;
}

__unused static void mouse_report_coords(s8 bytes[3])
{
	/* TODO: Some usage of that. */
	static int mouse_x = 0;
	static int mouse_y = 0;

	/* Calculate x and y displacement */
	s8 x_move = bytes[1];
	s8 y_move = bytes[2];
	if (bytes[0] & 0x10)
		x_move |= 0xFFFFFF00;
	if (bytes[0] & 0x20)
		y_move |= 0xFFFFFF00;

	/* Usually, Y movement is inverted. */
	mouse_x += x_move;
	mouse_y -= y_move;

	kprintf("coords: (%d, %d)\n", mouse_x, mouse_y);
}

static void irq_mouse(__unused struct registers regs)
{
	static u8 cycle = 0;
	__unused static s8 bytes[3];

	u8 status = port_byte_in(MOUSE_STATUS);
	while (status & MOUSE_BBIT) {
		u8 in = port_byte_in(PS2_PORT);
		if (status & MOUSE_F_BIT) {
			switch (cycle) {
			case 0:
				bytes[0] = in;
				if (!(in & MOUSE_V_BIT))
					return;
				++cycle;
				break;
			case 1:
				bytes[1] = in;
				++cycle;
				break;
			case 2:
				bytes[2] = in;
				/* Report function can be easily reworked
				   for any other need. */
				if (mouse_report_click(bytes) != 0)
					break;
				mouse_report_coords(bytes);
				cycle = 0;
				break;
			}
		}
		status = port_byte_in(MOUSE_STATUS);
	}

	/* TODO: Learn what is acknowledge in context of PS/2. */
}

void mouse_install()
{
	/* NOTE: For totally unknown for me reason HDMI port and
	         my USB mouse are interconnected somehow. This mouse
	         handler works iff HDMI is plugged out from laptop. */
	mouse_wait(1);
	port_byte_out(MOUSE_STATUS, 0xA8);
	mouse_wait(1);
	port_byte_out(MOUSE_STATUS, 0x20);
	mouse_wait(0);
	u8 status = port_byte_in(PS2_PORT) | 2;
	mouse_wait(1);
	port_byte_out(MOUSE_STATUS, 0x60);
	mouse_wait(1);
	port_byte_out(PS2_PORT, status);
	mouse_write(PS2_CMD_SET_DEFAULTS);
	mouse_read();
	mouse_write(PS2_CMD_ENABLE_DATA_REPORTING);
	mouse_read();

	irq_install_handler(IRQ12, irq_mouse);
}