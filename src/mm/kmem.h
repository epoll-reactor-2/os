#ifndef KMEM_H
#define KMEM_H

#include <stddef.h>

/*
 * Here comes our byte-grained memory allocator
 *
 * KMMD stands for "kernel memory metadata"
 * The idea is that, for every block of memory allocated with
 * kmalloc(), the first 64 bits of that memory KMMD hold metadata
 * indicating:
 *
 * - Whether the block is taken in KMMD[63]
 * - The size of the block (including metadata) in KMMD[62:0]
 */
#define __kmem_metadata_taken		(1ull << 63)
#define __kmem_metadata_is_taken(block)	(!!(*(const size_t *)(block) & __kmem_metadata_taken))
#define __kmem_metadata_is_free(block) 	(!__kmem_metadata_is_taken(block))
#define __kmem_metadata_set_taken(block) ({				\
	*(size_t *)(block) |= __kmem_metadata_taken;			\
})
#define __kmem_metadata_set_free(block) ({				\
	*(size_t *)(block) &= ~__kmem_metadata_taken;			\
})
#define __kmem_metadata_set_size(block, sz) ({				\
	size_t _k = __kmem_metadata_is_taken(block);			\
	*(size_t *)(block) = (size_t)(sz) & ~__kmem_metadata_taken;	\
	if (_k)								\
		__kmem_metadata_set_taken(block);			\
})
#define __kmem_metadata_get_size(block) (*(const size_t *)(block) & ~__kmem_metadata_taken)

void *kmem_get_head(void);
struct page_table *kmem_get_page_table(void);
size_t kmem_get_num_allocations(void);

void kmem_init(void);
void *kcalloc(size_t, size_t);
void *kmalloc(size_t);
void kfree(void *);

void kmem_print_table(void);

#endif
