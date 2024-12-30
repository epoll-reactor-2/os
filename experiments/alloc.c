#include <stdio.h>
#include <stdint.h>

#define u8	 uint8_t
#define i8	  int8_t
#define u16	uint16_t
#define i16	 int16_t
#define u32	uint32_t
#define i32	 int32_t
#define u64	uint64_t
#define i64	 int64_t

#define __packed(x)	__attribute__ ((packed))
#define __section(x) 	__attribute__ ((section (x)))

#define __mem_section		"__kernel.allocator"
#define __mem_size		((u64) (1 << 20))
#define __mem_log(fmt, ...)	printf(__mem_section ": " fmt, ##__VA_ARGS__)

__section(__mem_section)
static u8 memory[__mem_size];

struct alloc_entry {
	u64	off;
	u64	siz;
	u8	busy;
} __packed;

void *allocate(u64 siz)
{
	u64 off = 0;
	struct alloc_entry *e = (struct alloc_entry *) &memory[0];

	while (off < __mem_size && e->busy) {
		off = e->off;
		off += e->siz;
		off += sizeof (*e);
		e = (struct alloc_entry *) &memory[off];
		/* TODO: Check current entry.
		         If not busy and siz <= requested length,
		         allocate there.
		         Otherwise skip. */
	}

	if (off)
		++off;

	if ((off + siz + sizeof (*e)) >= __mem_size) {
		u64 mem_off = off + siz + sizeof (*e);
		__mem_log("enomem (\n");
		__mem_log("  siz:           %ld\n", siz);
		__mem_log("  mem_siz:       %ld\n", __mem_size);
		__mem_log("  off:           %ld\n", mem_off);
		__mem_log("  off > mem_siz: %d\n",  mem_off >= __mem_size);
		__mem_log(")\n");
		return NULL;
	} else {
		__mem_log("ok\n");
	}

	e->off = off;

	e->siz = siz;
	e->busy = 1;

	void *area = (void *) ((u8 *) e + sizeof (*e));

	return area;
}

void allocate_dump()
{
	u64 off = 0;
	struct alloc_entry *e = (struct alloc_entry *) &memory[0];

	while (e->busy) {
		u64 metadata = e->off;

		__mem_log("            +------------------------+\n");
		__mem_log(" 0x%08lx | meta: %p \n",
			metadata,
			e
		);
		__mem_log("            | siz: %08lx\n",
			e->siz);
		__mem_log("            | mem:  %p\n",
			(u8 *) e + sizeof (*e)
		);

		off = e->off;
		off += e->siz;
		off += sizeof (*e);
		e = (struct alloc_entry *) &memory[off];
	}
}

int main()
{
	__mem_log("mem:   [%p %p]\n", &memory[0], &memory[__mem_size - 1]);
	__mem_log("entry: %ld\n", sizeof (struct alloc_entry));
	__mem_log("area:  %ld\n", __mem_size);

	int *area_1 = allocate(sizeof (int));
	int *area_2 = allocate(sizeof (int));
	int *area_3 = allocate(1 << 19);
	int *area_4 = allocate(sizeof (int));

	if (area_1) *area_1 = 0x10;
	if (area_2) *area_2 = 0x20;
	if (area_3) *area_3 = 0x30;
	if (area_4) *area_4 = 0x40;

	if (area_1) __mem_log("area_1: %p %d\n", area_1, *area_1);
	if (area_2) __mem_log("area_2: %p %d\n", area_2, *area_2);
	if (area_3) __mem_log("area_3: %p %d\n", area_3, *area_3);
	if (area_4) __mem_log("area_4: %p %d\n", area_4, *area_4);
	__mem_log("\n");

	allocate_dump();
}