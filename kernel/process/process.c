#include "mm/kmem.h"
#include "mm/page.h"
#include "mm/sv39.h"
#include "process/process.h"
#include "process/syscall.h"
#include "printk/printk.h"
#include "macro.h"

extern const size_t __make_syscall;

static int __next_pid = 1;

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
	process->pid = __next_pid++;
	process->pages = (struct page_table *) alloc_page();

	__assert(process->pages != NULL,
		"process_create(): failed to allocate page for process pages page table\n");

	process->state = __process_running;
	process->sleep_until = 0;


	size_t __stack_paddr = (size_t) process->stack;	// obtain stack physical address
	// Set stack pointer to point to top of process stack
	process->frame->regs[2] = __stack_addr + __page_size * __stack_pages;	// sp = x2

	// Map process stack to virtual memory
	for (size_t i = 0; i < __stack_pages; ++i) {
		map(process->pages, __stack_addr + i * __page_size, __stack_paddr + i * __page_size,
			__pte_user_rw, 0);

		printk("pid %3d: mapped stack: virt 0x%8x to phys 0x%8x\n",
			process->pid,
			__stack_addr + i * __page_size,
			__stack_paddr + i * __page_size
		);
	}

	// Map user program to virtual memory
	for (size_t i = 0; i < 10000; ++i)
		map(process->pages, func_vaddr + i * __page_size, func_paddr + i * __page_size,
			__pte_user_rx, 0);

	printk("pid %3d: mapped .text\n", process->pid);
	printk("pid %3d:  virt [0x%8x, 0x%8x)\n",
		process->pid,
		func_vaddr +     0 * __page_size,
		func_vaddr + 10000 * __page_size
	);
	printk("pid %3d:  phys [0x%8x, 0x%8x)\n",
		process->pid,
		func_paddr +     0 * __page_size,
		func_paddr + 10000 * __page_size
	);

	// Map make_syscall() to virtual memory
	// This is required since otherwise user programs cannot make
	// system calls from user space
	map(process->pages, __make_syscall, __make_syscall, __pte_user_rx, 0);

	return process;
}

void process_switch_to_user(struct process *process)
{
	switch_to_user(
		(size_t) process->frame, process->pc,
		__satp_from(
			__mode_sv39, process->pid,
			(size_t) process->pages >> __page_order));
}

/* Под низкою крышкою гроба,
‎   Забиты гвоздями,
   Недвижно лежали мы оба,
   С враждебными оба чертами.
   
   Застывшие трупы, мы жили
‎   Сознаньем проклятья,
   Что вот и в могиле — в могиле! —
   Мы в мерзостной позе объятья.
   
   И дьявол смеялся надгробно,
   Плитой погребальной:
   «Эге», — говорил: «как удобно
   Уродцам — в могиле двуспальной!»  */