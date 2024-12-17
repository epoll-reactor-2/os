#include "uart/uart.h"
#include "syscon/syscon.h"
#include "common/common.h"
#include "mm/page.h"
#include "mm/sv39.h"
#include "mm/kmem.h"
#include "plic/trap_frame.h"
#include "plic/cpu.h"
#include "plic/plic.h"
#include "process/process.h"
#include "process/sched.h"

// Identity map range
// Takes a contiguous allocation of memory and maps it using __page_size
// `start` must not exceed `end`
void id_map_range(struct page_table *root, size_t start, size_t end,
		  uint64_t bits)
{
	__assert(root != NULL, "id_map_range(): root page table cannot be NULL");
	__assert(start <= end, "id_map_range(): start must not exceed end");
	__assert(__pte_is_leaf(bits),
		"id_map_range(): Provided bits must correspond to leaf entry");

	size_t memaddr = start & ~(__page_size - 1);
	size_t num_kb_pages = (align_val(end, __page_order) - memaddr) / __page_size;

	for (size_t i = 0; i < num_kb_pages; ++i) {
		map(root, memaddr, memaddr, bits, 0);
		memaddr += __page_size;
	}
}

void kernel_main(void)
{
	uart_init();
	page_init();
	kmem_init();

	__plic_set_threshold(0);
	__plic_enable(__plic_uart);
	__plic_set_prio(__plic_uart, 1);

	kprintf("Initializing the process scheduler ...\n");
	sched_init();

	kprintf("Adding a second and third process to test our scheduler ...\n");
	sched_enqueue(init_process);

	kprintf("Starting our first process ...\n");
	struct process *process = sched_schedule();
	__assert(process != NULL,
		"kmain(): process structure returned from scheduler was unexpectedly NULL\n");

	kprintf("Our first process has PID = %d\n", process->pid);

	kprintf("Issuing our first context switch timer ...\n");
	set_timer_interrupt_delay_us(1 * __us_per_second);

	switch_to_user((size_t)process->frame, process->pc,
		__satp_from(__mode_sv39, process->pid,
		(size_t)process->root >> __page_order));

	__panic("kmain(): failed to start our first process!\n");
}
