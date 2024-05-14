#include "ata.h"
#include "cpu/type.h"
#include "cpu/ports.h"
#include "drivers/vga.h"
#include "lib/string.h"
#include "lib/mem.h"

// Documentation source: https://wiki.osdev.org/ATA_PIO_Mode

void ide_delay(int n)
{
	for (int i = 0; i < n; i++)
		port_byte_in(ATA_SECONDARY_DEVCTL);
}

// drive software reset
void drive_reset()
{
	port_byte_out(ATA_PRIMARY_DEVCTL, 0x04);
	ide_delay(4);
	port_byte_out(ATA_PRIMARY_DEVCTL, 0x00);
}

// chs to LBA
u32 chs_to_lba(u32 cylinder, u32 head, u32 sector)
{
	return (cylinder * 16 + head) * 63 + (sector - 1);
}

int detect_devtype(int slavebit)
{
	u32 reg_cyl_lo = 4;
	u32 reg_cyl_hi = 5;
	u32 reg_devsel = 6;
	/* waits until master drive is ready again */
	// ata_soft_reset(ctrl->dev_ctl);		
	port_byte_out(ATA_PRIMARY_IO + reg_devsel, 0xA0 | slavebit << 4);
	// 400ns seconds delay
	ide_delay(4);

	u32 cl = port_byte_in(ATA_PRIMARY_IO + reg_cyl_lo);	/* get the "signature bytes" */
	u32 ch = port_byte_in(ATA_PRIMARY_IO + reg_cyl_hi);

	/* differentiate ATA, ATAPI, SATA and SATAPI */
	if (cl == 0x14 && ch == 0xEB) return 0;
	if (cl == 0x69 && ch == 0x96) return 1;
	if (cl == 0x00 && ch == 0x00) return 2;
	if (cl == 0x3c && ch == 0xc3) return 3;
	return -1;
}

int ata_poll()
{
	ide_delay(4);

	while (port_byte_in(ATA_PRIMARY_IO + ATA_REG_STATUS) & ATA_SR_BSY)
		;

	u8 status = port_byte_in(ATA_PRIMARY_IO + ATA_REG_STATUS);

	if (status & ATA_SR_ERR) return 2;
	if (status & ATA_SR_DF) return 1;
	if ((status & ATA_SR_DRQ) == 0) return 3;
	return 0;
}

u16* ata_read(u32 lba, u8 sector_num)
{
	port_byte_out(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xE0 | (u8) (0x00 << 4) | (u8) ((lba >> 24) & 0x0F));
	port_byte_out(ATA_PRIMARY_IO + ATA_REG_SECCOUNT0, (u8) sector_num);
	port_byte_out(ATA_PRIMARY_IO + ATA_REG_LBA0, (u8) lba);
	port_byte_out(ATA_PRIMARY_IO + ATA_REG_LBA1, (u8)(lba >> 8));
	port_byte_out(ATA_PRIMARY_IO + ATA_REG_LBA2, (u8)(lba >> 16));
	port_byte_out(ATA_PRIMARY_IO + ATA_REG_COMMAND, (u8) ATA_CMD_READ_PIO);

	if (ata_poll())
		return NULL;

	u16* data = (u16*) kmalloc(sizeof(u16) * sector_num * 256);
	for (int i = 0; i < sector_num * 256; i++) {
		data[i] = port_word_in(ATA_PRIMARY_IO + ATA_REG_DATA);
	}
	return data;
}

u8 ata_write(u32 lba, u8 sector_num, u16* data, u8 size)
{
	port_byte_out(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xE0 | (u8) (0x00 << 4) | (u8) ((lba >> 24) & 0x0F));
	port_byte_out(ATA_PRIMARY_IO + ATA_REG_SECCOUNT0, (u8) sector_num);
	port_byte_out(ATA_PRIMARY_IO + ATA_REG_LBA0, (u8) lba);
	port_byte_out(ATA_PRIMARY_IO + ATA_REG_LBA1, (u8)(lba >> 8));
	port_byte_out(ATA_PRIMARY_IO + ATA_REG_LBA2, (u8)(lba >> 16));
	port_byte_out(ATA_PRIMARY_IO + ATA_REG_COMMAND, (u8) ATA_CMD_WRITE_PIO);

	if (ata_poll())
		return 1;

	for (int i = 0; i < sector_num * 256; i++) {
		ata_poll();
		if (i < size) {
			port_word_out(ATA_PRIMARY_IO + ATA_REG_DATA, data[i]);
			continue;
		}
		// fill with zeros the rest
		port_word_out(ATA_PRIMARY_IO + ATA_REG_DATA, (u16) 0x00);
	}
	port_byte_out(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH); // cache flush
	ata_poll();
	return 0;
}

void ata_init()
{
	drive_reset();
}
