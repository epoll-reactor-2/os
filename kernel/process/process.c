#include "mm/kmem.h"
#include "mm/page.h"
#include "mm/sv39.h"
#include "process/process.h"
#include "process/syscall.h"
#include "macro.h"

extern const size_t MAKE_SYSCALL;

static uint16_t NEXT_PID = 1;

// This is just a temporary measure
// Ideally, we want to move our hardcoded init process
// out of the kernel as soon as possible
void process_init(void)
{
	while (1) {
		for (size_t i = 0; i < 70000000; ++i)
			;

		make_syscall(1);
	}
}

struct process *process_create(void (*func)(void))
{
	size_t func_paddr = (size_t) func;	// determine process physical address
	size_t func_vaddr = func_paddr;	// set process virtual address

	// Initialize process structure
	struct process *process = kmalloc(sizeof (struct process));

	__assert(process != NULL,
		"process_create(): failed to allocate memory for process structure\n");

	process->frame = (struct trap_frame *)alloc_page();
	__assert(process->frame != NULL,
		"process_create(): failed to allocate page for process context frame\n");

	process->stack = alloc_pages(__stack_pages);

	__assert(process->stack != NULL,
		"process_create(): failed to allocate %d pages for process stack\n",
		__stack_pages);

	process->pc = func_vaddr;
	process->pid = NEXT_PID++;
	process->root = (struct page_table *) alloc_page();

	__assert(process->root != NULL,
		"process_create(): failed to allocate page for process root page table\n");

	process->state = __process_running;
	process->sleep_until = 0;

	size_t stack_paddr = (size_t) process->stack;	// obtain stack physical address
	// Set stack pointer to point to top of process stack
	process->frame->regs[2] = __stack_addr + __page_size * __stack_pages;	// sp = x2

	// Map process stack to virtual memory
	for (size_t i = 0; i < __stack_pages; ++i)
		map(process->root, __stack_addr + i * __page_size, stack_paddr + i * __page_size,
			__pte_user_rw, 0);

	// Map user program to virtual memory
	for (size_t i = 0; i < 100; ++i)
		map(process->root, func_vaddr + i * __page_size, func_paddr + i * __page_size,
			__pte_user_rx, 0);

	// Map make_syscall() to virtual memory
	// This is required since otherwise user programs cannot make
	// system calls from user space
	map(process->root, MAKE_SYSCALL, MAKE_SYSCALL, __pte_user_rx, 0);

	return process;
}

void process_switch_to_user(struct process *process)
{
	switch_to_user(
		(size_t) process->frame, process->pc,
		__satp_from(
			__mode_sv39, process->pid,
			(size_t) process->root >> __page_order));
}