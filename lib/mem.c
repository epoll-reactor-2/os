#include "mem.h"
#include "cpu/type.h"

void memcpy(u8 *source, u8 *dest, int nbytes)
{
	int i;
	for (i = 0; i < nbytes; i++)
		*(dest + i) = *(source + i);
}

void memset(u8 *dest, u8 val, u32 len)
{
	u8 *temp = (u8 *)dest;
	for ( ; len != 0; len--)
		*temp++ = val;
}

/*
Dynamic Memory Allocation (DMA) code below.

For now, the code is smarter than just growing pointer 
but still far from good dynamic allocation.
TODO: Instead of reusing memory sectors it will be more practical to remove them and
later in linked list detect available memory spaces and fill them with new sectors.
*/

/*
Private functions and variables
*/

// start of available memory, head of the linked list
u32 free_mem_addr = 0x10000;

// pre-defined numbers of bytes to allocate
// 0x200(h) = 512(d) bytes
u32 PAGE_SIZE = 0x200;

// memory sector (mem_sec)
typedef struct mem_sec {
	struct mem_sec *next;
	u32 		is_used;
	u32 		pages;
} mem_sec;

u32 calculate_pages_num(u32 size)
{
	u32 pages_num = size / PAGE_SIZE;
	if ((size % PAGE_SIZE) > 0)
		pages_num++;
	return pages_num;
}

// find if any defined before mem_sec is not used 
// and if it has enough pages to use
mem_sec *find_free_sector(u32 pages_num)
{
	mem_sec *p = (mem_sec*) free_mem_addr;
	while (p != NULL) {
		if (p->is_used == 0 && p->pages >= pages_num)
			return p;
		p = p->next;
	}
	return NULL;
}

mem_sec *insert_sector(u32 pages)
{
	mem_sec *p = (mem_sec*) free_mem_addr;
	while (p != NULL) {
		if (p->next == NULL) {
			u32 new_addr = (u32) p + sizeof(mem_sec) + p->pages * PAGE_SIZE;
			mem_sec *new_sector = (mem_sec*) new_addr;
			new_sector->is_used = 1;
			new_sector->pages = pages;
			new_sector->next = NULL;

			p->next = new_sector;
			return new_sector;
		}
		p = p->next;
	}
	return NULL;
}

u32 calculate_memory_pointer(mem_sec *mem_sec)
{
	return (u32) mem_sec + (u32) sizeof(mem_sec); 
}

/*
Public functions
*/

void free(void *addr)
{
	mem_sec *p = (mem_sec*) free_mem_addr;
	while (p != NULL) {
		if (((u32) p + sizeof(mem_sec)) == (u32) addr) {
			p->is_used = 0;
			return;
		}
		p = p->next;
	}
}

u32 kmalloc(u32 size)
{
	u32 pages_num = calculate_pages_num(size);

	// allocate memory for the first time in the whole system
	if (free_mem_addr == 0x10000) {
		free_mem_addr += 1;
		mem_sec *msec = (mem_sec*) free_mem_addr;
		msec->next = NULL;
		msec->is_used = 1;
		msec->pages = pages_num;
		return calculate_memory_pointer(msec);
	}

	mem_sec *sector = find_free_sector(pages_num);

	if (sector != NULL) {
		sector->is_used = 1;
	} else {
		sector = insert_sector(pages_num);
	}

	return calculate_memory_pointer(sector);
}
