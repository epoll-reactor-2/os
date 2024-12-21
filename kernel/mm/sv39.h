#ifndef SV39_H
#define SV39_H

#include <stdint.h>
#include <stddef.h>

// MODE=8 encodes Sv39 paging in SATP register
#define __mode_sv39 8

// Page table entry (PTE) bits
#define __pte_none		 0
#define __pte_valid		(1 << 0)
#define __pte_read		(1 << 1)
#define __pte_write		(1 << 2)
#define __pte_execute		(1 << 3)
#define __pte_user		(1 << 4)
#define __pte_global		(1 << 5)
#define __pte_access		(1 << 6)
#define __pte_dirty		(1 << 7)

// Common PTE bit combinations
#define __pte_rw		(__pte_read | __pte_write)
#define __pte_rx		(__pte_read | __pte_execute)
#define __pte_rwx		(__pte_read | __pte_write | __pte_execute)

// Common PTE bit combinations (user mode)
#define __pte_user_rw		(__pte_user | __pte_rw)
#define __pte_user_rx		(__pte_user | __pte_rx)
#define __pte_user_rwx		(__pte_user | __pte_rwx)

// Common PTE checks
#define __pte_is_valid(entry) 	((entry) & __pte_valid)
#define __pte_is_invalid(entry) (!__pte_is_valid(entry))
#define __pte_is_leaf(entry) 	((entry) & 0xE)
#define __pte_is_branch(entry) 	(!__pte_is_leaf(entry))

// Construct SATP from MODE, ASID and PPN fields
#define __satp_from(mode, asid, ppn) (((size_t)(mode) << 60) | ((size_t)(asid) << 44) | ppn)

// A page table is exactly 4096 / 8 = 512 64-bit entries
#define __page_table_size 512
struct page_table {
	uint64_t entries[__page_table_size];
};

void map(struct page_table *, size_t, size_t, uint64_t, int);
void unmap(struct page_table *);
size_t virt_to_phys(struct page_table const *, size_t);

#endif
