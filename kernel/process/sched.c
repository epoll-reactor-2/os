#include <stddef.h>
#include "mm/kmem.h"
#include "process/process.h"
#include "process/sched.h"
#include "printk/printk.h"
#include "macro.h"

static struct process_ll *processes = NULL;

void sched_init(void)
{
	__assert(processes == NULL,
		"sched_init(): should only be called once at system startup\n");
}

void sched_enqueue(void (*func)(void), const char *name)
{
	struct process_ll *nd = kmalloc(sizeof(struct process_ll));

	__assert(nd != NULL,
		"sched_enqueue(): failed to allocate linked list node for new process\n");

	nd->process = process_create(func, name);

	if (processes == NULL) {
		nd->prev = nd;
		nd->next = nd;
		processes = nd;
	} else {
		nd->prev = processes->prev;
		nd->next = processes;
		processes->prev->next = nd;
		processes->prev = nd;
	}
}

struct process *sched_schedule(void)
{
	__assert(processes != NULL,
		"sched_schedule(): cannot schedule a process from an empty process list - did you call sched_init()?\n");

	struct process *process = processes->process;

	__assert(process != NULL,
		"sched_schedule(): process structure from head of process list was unexpectedly NULL\n");

	processes = processes->next;

	return process;
}

void sched_print_ptree()
{
	printk("\n");

	printk("Process table\n");
	printk("_________________________________________________\n");

	struct process_ll *current = processes;

	do {
		printk("|  name:            %s\n", current->process->name);
		printk("|  PID:             %d\n", current->process->pid);
		printk("|  state:           %s\n", process_state_string(current->process->state));
    	printk("_________________________________________________\n");
		current = current->next;
	} while (current != processes);

	printk("\n");
}