#ifndef KERNEL_CPU_PORTS_H
#define KERNEL_CPU_PORTS_H

#include "type.h"

enum {
	PORT_PS2			= 0x060,

	PORT_ATA_PRIMARY_IO		= 0x1F0,
	PORT_ATA_SECONDARY_IO		= 0x170,
	PORT_ATA_PRIMARY_DEVCTL		= 0x3F6,
	PORT_ATA_SECONDARY_DEVCTL	= 0x376,
};

u8   port_byte_in(u16 port);
void port_byte_out(u16 port, u8 data);
u16  port_word_in(u16 port);
void port_word_out(u16 port, u16 data);

#endif /* KERNEL_CPU_PORTS_H */