#ifndef PROCESS_H
#define PROCESS_H

#include <stddef.h>
#include <stdint.h>
#include "plic/trap_frame.h"

// Number of pages per process stack
#define __stack_pages 2

#define __process_stack_vaddr	0x100000000ull
#define __process_start_addr	0x80000000ull

// Init process - hardcoded for now, for testing purposes only
void process_init(void);

// - __process_running:  the process is ready to run whenever
//                       the scheduler picks it
//
// - __process_sleeping: the process is waiting for a certain
//                       amount of time
//
// - __process_waiting:  the process is waiting on I/O
//
// - __process_dead:     the process has finished and waiting to
//                       be cleaned up
enum {
	__process_running 	= (1 << 0),
	__process_sleeping 	= (1 << 1),
	__process_waiting 	= (1 << 2),
	__process_dead 		= (1 << 3)
};

static inline const char *process_state_string(int state)
{
	switch (state) {
	case __process_running:  return "running";
	case __process_sleeping: return "sleeping";
	case __process_waiting:  return "waiting";
	case __process_dead:     return "dead";
	default:
		return "<unknown process state>";
	}
}

// Process structure
// We need to know the exact sizes and positions
// of each field since we might need to access them
// in assembly
struct process {
	char			name[32];
	struct trap_frame 	*frame;
	void 			*stack;
	size_t 			pc;
	size_t			sp;
	uint16_t 		pid;
	struct page_table 	*pages;
	struct page_table 	*heap;
	size_t 			state;
	size_t 			sleep_until;
};

// Create a new process from function pointer
struct process *process_create(void (*)(void), const char *name);
void            process_switch_to_user(struct process *process);

// Defined in src/asm/crt0.s
void switch_to_user(size_t frame_addr, size_t pc, size_t satp);

#endif
