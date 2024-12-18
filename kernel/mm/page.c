#include <stdbool.h>
#include "page.h"
#include "../common/common.h"
#include "../printk/printk.h"

extern const size_t HEAP_START;
extern const size_t HEAP_SIZE;
extern const size_t HEAP_END;

static size_t heap_bottom = 0;
static size_t num_pages   = 0;
static size_t alloc_start = 0;
static size_t alloc_end   = 0;

size_t get_num_pages(void)
{
	return num_pages;
}

// Align pointer to nearest 2^order bytes, rounded up
size_t align_val(size_t val, size_t order)
{
	size_t o = (1ull << order) - 1;
	return (val + o) & ~o;
}

// Get page address from page id
static size_t page_address_from_id(size_t id)
{
	return alloc_start + __page_size * id;
}

// Initialize the heap for page allocation
void page_init(void)
{
	heap_bottom = HEAP_START;
	num_pages = HEAP_SIZE / __page_size;
	struct page *ptr = (struct page *)heap_bottom;
	// Explicitly mark all pages as free
	for (size_t i = 0; i < num_pages; ++i)
		ptr[i].flags = 0;

	alloc_start =
		align_val(heap_bottom + num_pages * sizeof(struct page), __page_order);
	alloc_end = page_address_from_id(num_pages);

	// Re-compute alloc_end and num_pages as the heap should not
	// extend beyond our memory region
	size_t error = alloc_end - (heap_bottom + HEAP_SIZE);
	num_pages -= error / __page_size;
	alloc_end = heap_bottom + HEAP_SIZE;

	__assert(
		page_address_from_id(num_pages) <= alloc_end,
		"page_init(): Heap extends beyond our available memory region!"
	);
}

// Attempts to allocate the specified number of contiguous free pages
// and returns a pointer to the beginning of the first page if successful
// All allocated pages are automatically zeroed if successful
// Otherwise, return NULL
void *alloc_pages(size_t n)
{
	__assert(n != 0, "alloc_pages(): attempted to allocate 0 pages");
	struct page *ptr = (struct page *)heap_bottom;

	for (size_t i = 0; i + n < num_pages + 1; ++i) {
		// Check that the next `n` pages are all free
		bool found = true;
		for (size_t j = 0; j < n; ++j)
			if (ptr[i + j].flags & __page_taken) {
				found = false;
				break;
			}

		if (!found)
			continue;

		// Mark the next `n` pages as all taken and indicate
		// the last page
		for (size_t j = 0; j < n; ++j)
			ptr[i + j].flags = __page_taken;

		ptr[i + n - 1].flags |= __page_last;

		// Zero memory for all `n` pages and return a pointer to
		// the beginning of the 1st page
		// Do it in chunks of size_t bytes for efficiency
		size_t *result = (size_t *)page_address_from_id(i);
		size_t size = (__page_size * n) / sizeof(size_t);

		for (size_t j = 0; j < size; ++j)
			result[j] = 0;

		return (void *)result;
	}

	// Failed to find `n` contiguous free pages
	return NULL;
}

// Attempts to allocate a single zeroed free page; NULL otherwise
void *alloc_page(void)
{
	return alloc_pages(1);
}

// Deallocate a set of contiguous pages from a pointer returned
// from alloc_pages()
void dealloc_pages(void *ptr)
{
	__assert(ptr != NULL, "dealloc_pages(): attempted to free NULL pointer");

	// Fetch corresponding page struct for given page address
	size_t addr = heap_bottom + ((size_t)ptr - alloc_start) / __page_size;

	__assert(
		heap_bottom <= addr && addr < heap_bottom + HEAP_SIZE,
		"dealloc_pages(): Variable addr = %p outside heap range [%p, %p)",
		addr, heap_bottom, heap_bottom + HEAP_SIZE
	);

	// Keep clearing pages until we hit the last page
	struct page *p = (struct page *)addr;

	while ((p->flags & __page_taken) && !(p->flags & __page_last)) {
		p->flags = 0;
		++p;
	}

	__assert(
		p->flags & __page_last,
		"dealloc_pages(): Found a free page before reaching the "
		"last page; possible double-free error occurred"
	);

	// Clear the flags on the last page
	p->flags = 0;
}

void print_page_allocations(void)
{
	struct page *ptr = (struct page *)heap_bottom;
	size_t total = 0;
	size_t TOTAL_BYTES = num_pages * __page_size;

	printk("Page allocation table\n");
	printk("_________________________________________________\n");
	printk("|   Total usable memory: %d pages (%d bytes)\n", num_pages, TOTAL_BYTES);
	printk("|   Metadata:            [%p, %p)\n", ptr, &ptr[num_pages]);
	printk("|   Pages:               [%p, %p)\n", alloc_start, alloc_end);
	printk("|________________________________________________\n");

	for (size_t i = 0; i < num_pages; ++i) {
		if (ptr[i].flags & __page_taken) {
			size_t start_addr = page_address_from_id(i);
			if (ptr[i].flags & __page_last) {
				printk("|   [%p, %p): 1 page\n", start_addr, start_addr + __page_size);
				++total;
				continue;
			}
			++i;
			while (i < num_pages && (ptr[i].flags & __page_taken)
				&& !(ptr[i].flags & __page_last))
				++i;

			__assert(i < num_pages,
				"print_page_allocations(): reached end of metadata before finding the last page"
			);

			__assert(ptr[i].flags & __page_taken,
				"print_page_allocations(): found free page before reaching the "
				"last page - possible double-free error"
			);

			size_t end_addr = page_address_from_id(i + 1);
			size_t pages = (end_addr - start_addr) / __page_size;

			printk("|   [%p, %p): %d pages\n", start_addr, end_addr, pages);
			total += pages;
		}
	}

	size_t ALLOC_BYTES = total * __page_size;
	printk("|\n");
	printk("|   Total allocated: %d pages (%d bytes)\n", total, ALLOC_BYTES);
	printk("|   Total free:      %d pages (%d bytes)\n", num_pages - total,
		TOTAL_BYTES - ALLOC_BYTES);
	printk("|________________________________________________\n");
	printk("\n");
}
