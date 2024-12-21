#include <stdbool.h>
#include "mm/kmem.h"
#include "mm/page.h"
#include "printk/printk.h"
#include "macro.h"

// Head of allocation. Start here when searching for free memory location
static size_t *kmem_head = NULL;
// Keep track of memory footprint
static size_t kmem_alloc = 0;
static struct page_table *kmem_page_table = NULL;

void *kmem_get_head(void)
{
	return (void *) kmem_head;
}

struct page_table *kmem_get_page_table(void)
{
	return kmem_page_table;
}

size_t kmem_get_num_allocations(void)
{
	return kmem_alloc;
}

// Initialize kernel memory
// This memory must not be allocated to userspace processes!
void kmem_init(void)
{
	void *k_alloc = alloc_pages(64);

	__assert(k_alloc != NULL,
		"kmem_init(): got NULL pointer when requesting pages for kernel"
	);

	kmem_alloc = 64;
	kmem_head = (size_t *) k_alloc;
	__kmem_metadata_set_free(kmem_head);
	__kmem_metadata_set_size(kmem_head, kmem_alloc * __page_size);
	kmem_page_table = (struct page_table *)alloc_page();

	__assert(kmem_page_table != NULL,
		"kmem_init(): got NULL pointer when requesting single page for kernel page table"
	);
}

void *kcalloc(size_t num, size_t size)
{
	size_t total_size = align_val(num * size, 3);
	uint8_t *result = (uint8_t *) kmalloc(total_size);

	if (result != NULL)
		for (size_t i = 0; i < total_size; ++i)
			result[i] = 0;

	return (void *) result;
}

void *kmalloc(size_t sz)
{
	size_t size = align_val(sz, 3) + sizeof(size_t);
	size_t *head = kmem_head;
	size_t *tail = (size_t *)&((uint8_t *) kmem_head)[kmem_alloc * __page_size];

	while ((size_t) head < (size_t)tail)
		if (__kmem_metadata_is_free(head) && size <= __kmem_metadata_get_size(head)) {
			size_t chunk_size = __kmem_metadata_get_size(head);
			size_t remaining = chunk_size - size;
			__kmem_metadata_set_taken(head);

			if (remaining > sizeof(size_t)) {
				size_t *next = (size_t *)&((uint8_t *) head)[size];
				__kmem_metadata_set_free(next);
				__kmem_metadata_set_size(next, remaining);
				__kmem_metadata_set_size(head, size);
			} else {
				__kmem_metadata_set_size(head, chunk_size);
			}

			return (void *)&head[1];

		} else {
			head = (size_t *)&((uint8_t *) head)[__kmem_metadata_get_size(head)];
		}

	return NULL;
}

// Merge adjacent free blocks into one big free block
static void coalesce(void)
{
	size_t *head = kmem_head;
	size_t *tail = (size_t *)&((uint8_t *) kmem_head)[kmem_alloc * __page_size];

	while ((size_t)head < (size_t)tail) {
		bool merged = false;
		// Get next block
		size_t *next = (size_t *)&((uint8_t *) head)[__kmem_metadata_get_size(head)];

		__assert(head != next,
			"coalesce(): found block of 0 bytes - possible double free error"
		);

		if (next >= tail)
		// `head` was the last block - nothing to coalesce
			break;
		else if (__kmem_metadata_is_free(head) && __kmem_metadata_is_free(next)) {
			// Both adjacent blocks are free - merge into `head`
			__kmem_metadata_set_size(head, __kmem_metadata_get_size(head) + __kmem_metadata_get_size(next));
			merged = true;
		}

		if (!merged)
			head = (size_t *)&((uint8_t *) head)[__kmem_metadata_get_size(head)];
	}
}

void kfree(void *ptr)
{
	if (ptr != NULL) {
		size_t *p = &((size_t *)ptr)[-1];
		if (__kmem_metadata_is_taken(p))
			__kmem_metadata_set_free(p);
		coalesce();
	}
}

// Print the kmem table for debugging
void kmem_print_table(void)
{
	printk("\n");
	printk("Memory allocation table\n");
	printk("_________________________________________________\n");
	size_t *head = kmem_head;
	size_t *tail = (size_t *)&((uint8_t *) kmem_head)[kmem_alloc * __page_size];

	while ((size_t) head < (size_t) tail) {
		printk("|   %p: taken = %d, size = %d\n", head,
			__kmem_metadata_is_taken(head),
			__kmem_metadata_get_size(head)
		);
		head = (size_t *)&((uint8_t *) head)[__kmem_metadata_get_size(head)];
	}
	printk("|________________________________________________\n");
	printk("\n");
}
