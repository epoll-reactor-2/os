#ifndef SCHED_H
#define SCHED_H

struct process_ll {
	struct process		*process;
	struct process_ll	*prev;
	struct process_ll	*next;
};

void sched_init(void);
void sched_enqueue(void (*)(void), const char *name);
struct process *sched_schedule(void);
void sched_print_ptree();

#endif
