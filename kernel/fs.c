#include "fs.h"
#include "drivers/ata.h"
#include "lib/mem.h"
#include "lib/stdio.h"
#include "lib/string.h"

void fs_init()
{
	kprint("Example usage of ATA driver \n");
	u32 t = chs_to_lba(0, 0, 30);
	u16 data[4] = {1, 2, 3, 4};

	if(ata_write(t, 1, data, 4)) {
		kprint("Write error");
	}

	u16 *tmp = ata_read(t, 1);

	for (int i = 0; i < 16; i++) {
		kprintf("%x\n", tmp[i]);
	}
}
