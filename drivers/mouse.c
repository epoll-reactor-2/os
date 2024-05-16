#include "drivers/mouse.h"
#include "drivers/vga.h"
#include "cpu/isr.h"
#include "cpu/type.h"
#include "cpu/ports.h"
#include "kernel/compiler.h"

#define MOUSE_IRQ 	12

#define MOUSE_STATUS	0x64
#define MOUSE_ABIT	0x02
#define MOUSE_BBIT	0x01
#define MOUSE_WRITE	0xD4
#define MOUSE_F_BIT	0x20
#define MOUSE_V_BIT	0x08

/* https://techdocs.altium.com/display/FPGA/PS2+Commands */
#define PS2_CMD_RESET				0xFF
#define PS2_CMD_RESEND				0xFE
#define PS2_CMD_SET_DEFAULTS			0xF6
#define PS2_CMD_DISABLE_DATA_REPORTING		0xF5
#define PS2_CMD_ENABLE_DATA_REPORTING		0xF4
#define PS2_CMD_SET_SAMPLE_RATE			0xF3
#define PS2_CMD_GET_DEV_ID			0xF2
#define PS2_CMD_SET_REMOTE_MODE			0xF0
#define PS2_CMD_SET_WRAP_MODE			0xEE
#define PS2_CMD_RESET_WRAP_MODE			0xEC
#define PS2_CMD_READ_DATA			0xEB
#define PS2_CMD_SET_STREAM_MODE			0xEA
#define PS2_CMD_STATUS_REQ			0xE9
#define PS2_CMD_SET_RESOLUTION			0xE8
#define PS2_CMD_SET_SCALING_2_1			0xE7
#define PS2_CMD_SET_SCALING_1_1			0xE6

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

static void irq_mouse(__unused struct registers regs)
{
	static u8 cycle = 0;
	static s8 bytes[3];

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
				/* We now have a full mouse packet ready to use */
				if (bytes[0] & 0x80 || bytes[0] & 0x40) {
					/* x/y overflow? bad packet! */
					break;
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
	/* TODO: What is this mean? */
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