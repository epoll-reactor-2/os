#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>
#include <stddef.h>

#define __page_taken 	(1 << 0)
#define __page_last 	(1 << 1)
#define __page_order 	12
#define __page_size 	(1 << __page_order) /* 4096 */

struct page {
	uint8_t flags;
};

size_t page_total(void);

size_t align_val(size_t, size_t);

void  page_init(void);
void *page_alloc_many(size_t);
void *page_alloc(void);
void  pages_dealloc(void *);
void  page_print_allocation_table(void);

#endif
