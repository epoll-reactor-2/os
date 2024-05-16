// source of constants: https://wiki.osdev.org/IDE
#ifndef KERNEL_DRIVERS_ATA_H
#define KERNEL_DRIVERS_ATA_H

#include <stdint.h>
#include "cpu/type.h"

enum {
	ATA_PRIMARY_IO 			= 0x1F0,
	ATA_SECONDARY_IO 		= 0x170,

	ATA_PRIMARY_DEVCTL 		= 0x3F6,
	ATA_SECONDARY_DEVCTL	 	= 0x376,

	/* Status port masks. */
	ATA_SR_BSY     			= 0x80,    /* Busy */
	ATA_SR_DRDY    			= 0x40,    /* Drive ready */
	ATA_SR_DF      			= 0x20,    /* Drive write fault */
	ATA_SR_DSC     			= 0x10,    /* Drive seek complete */
	ATA_SR_DRQ     			= 0x08,    /* Data request ready */
	ATA_SR_CORR    			= 0x04,    /* Corrected data */
	ATA_SR_IDX     			= 0x02,    /* Index */
	ATA_SR_ERR     			= 0x01,    /* Error */

	/* ATA command list. */
	ATA_CMD_READ_PIO		= 0x20,
	ATA_CMD_READ_PIO_EXT		= 0x24,
	ATA_CMD_WRITE_PIO		= 0x30,
	ATA_CMD_WRITE_PIO_EXT		= 0x34,
	ATA_CMD_CACHE_FLUSH		= 0xE7,
	ATA_CMD_CACHE_FLUSH_EXT		= 0xEA,
	ATA_CMD_PACKET			= 0xA0,
	ATA_CMD_IDENTIFY_PACKET		= 0xA1,
	ATA_CMD_IDENTIFY		= 0xEC,

	/* Registers offset. */
	ATA_REG_DATA			= 0x00,
	ATA_REG_ERROR			= 0x01,
	ATA_REG_FEATURES		= 0x01,
	ATA_REG_SECCOUNT0		= 0x02,
	ATA_REG_LBA0			= 0x03,
	ATA_REG_LBA1			= 0x04,
	ATA_REG_LBA2			= 0x05,
	ATA_REG_HDDEVSEL		= 0x06,
	ATA_REG_COMMAND			= 0x07,
	ATA_REG_STATUS			= 0x07,
	ATA_REG_SECCOUNT1		= 0x08,
	ATA_REG_LBA3			= 0x09,
	ATA_REG_LBA4			= 0x0A,
	ATA_REG_LBA5			= 0x0B,
	ATA_REG_CONTROL			= 0x0C,
	ATA_REG_ALTSTATUS		= 0x0C,
	ATA_REG_DEVADDRESS		= 0x0D,
};

void ata_init();
u32  chs_to_lba(u32 cylinder, u32 head, u32 sector);
u16* ata_read(u32 lba, u8 sector_num);
u8   ata_write(u32 lba, u8 sector_num, u16* data, u8 size);

#endif /* KERNEL_DRIVERS_ATA_H */