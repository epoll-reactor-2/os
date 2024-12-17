#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>
#include <stddef.h>

#define __page_taken 	(1 << 0)
#define __page_last 	(1 << 1)
#define __page_order 	12
#define __page_size 	(1 << __page_order)

struct page {
	uint8_t flags;
};

size_t get_num_pages(void);

size_t align_val(size_t, size_t);

void page_init(void);
void *alloc_pages(size_t);
void *alloc_page(void);
void dealloc_pages(void *);
void print_page_allocations(void);

#endif
