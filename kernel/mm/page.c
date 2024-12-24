#include <stdbool.h>
#include "mm/page.h"
#include "printk/printk.h"
#include "macro.h"

extern const size_t __heap_start;
extern const size_t __heap_size;

static size_t heap_bottom = 0;
static size_t num_pages   = 0;
static size_t alloc_start = 0;
static size_t alloc_end   = 0;

size_t page_total(void)
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
	heap_bottom = __heap_start;
	num_pages = __heap_size / __page_size;
	struct page *ptr = (struct page *)heap_bottom;
	// Explicitly mark all pages as free
	for (size_t i = 0; i < num_pages; ++i)
		ptr[i].flags = 0;

	alloc_start =
		align_val(heap_bottom + num_pages * sizeof(struct page), __page_order);
	alloc_end = page_address_from_id(num_pages);

	// Re-compute alloc_end and num_pages as the heap should not
	// extend beyond our memory region
	size_t error = alloc_end - (heap_bottom + __heap_size);
	num_pages -= error / __page_size;
	alloc_end = heap_bottom + __heap_size;

	printk("page_init(): heap_bottom: 0x%x\n", heap_bottom);
	printk("page_init(): num_pages:   %d\n", num_pages);
	printk("page_init(): alloc_start: 0x%x\n", alloc_start);
	printk("page_init(): alloc_end:   0x%x\n", alloc_end);
	printk("page_init(): error:       0x%x\n", error);

	__assert(
		page_address_from_id(num_pages) <= alloc_end,
		"page_init(): Heap extends beyond our available memory region!"
	);
}

// Attempts to allocate the specified number of contiguous free pages
// and returns a pointer to the beginning of the first page if successful
// All allocated pages are automatically zeroed if successful
// Otherwise, return NULL
void *page_alloc_many(size_t n)
{
	__assert(n != 0, "page_alloc_many(): attempted to allocate 0 pages");
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
		size_t *result = (size_t *) page_address_from_id(i);
		size_t size = (__page_size * n) / sizeof(size_t);

		for (size_t j = 0; j < size; ++j)
			result[j] = 0;

		return result;
	}

	// Failed to find `n` contiguous free pages
	return NULL;
}

// Attempts to allocate a single zeroed free page; NULL otherwise
void *page_alloc(void)
{
	return page_alloc_many(1);
}

// Deallocate a set of contiguous pages from a pointer returned
// from page_alloc_many()
void pages_dealloc(void *ptr)
{
	__assert(ptr != NULL, "pages_dealloc(): attempted to free NULL pointer");

	// Fetch corresponding page struct for given page address
	size_t addr = heap_bottom + ((size_t)ptr - alloc_start) / __page_size;

	__assert(
		heap_bottom <= addr && addr < heap_bottom + __heap_size,
		"pages_dealloc(): Variable addr = %p outside heap range [%p, %p)",
		addr, heap_bottom, heap_bottom + __heap_size
	);

	// Keep clearing pages until we hit the last page
	struct page *p = (struct page *)addr;

	while ((p->flags & __page_taken) && !(p->flags & __page_last)) {
		p->flags = 0;
		++p;
	}

	__assert(
		p->flags & __page_last,
		"pages_dealloc(): Found a free page before reaching the "
		"last page; possible double-free error occurred"
	);

	// Clear the flags on the last page
	p->flags = 0;
}

void page_print_allocation_table(void)
{
	struct page *ptr = (struct page *) heap_bottom;
	size_t total = 0;
	size_t total_bytes = num_pages * __page_size;

	printk("Page allocation table\n");
	printk("_________________________________________________\n");
	printk("|   Total usable memory: %d pages (%d bytes)\n", num_pages, total_bytes);
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
				"page_print_allocation_table(): reached end of metadata before finding the last page"
			);

			__assert(ptr[i].flags & __page_taken,
				"page_print_allocation_table(): found free page before reaching the "
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
		total_bytes - ALLOC_BYTES);
	printk("|________________________________________________\n");
	printk("\n");
}
