#ifndef PROCESS_H
#define PROCESS_H

#include <stddef.h>
#include <stdint.h>
#include "plic/trap_frame.h"

// Number of pages per process stack
#define __stack_pages 2

// Start of virtual stack address (bottom)
#define __stack_addr 0x100000000ull

// Start of process virtual address space
#define __process_start_addr 0x80000000ull

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

// Process structure
// We need to know the exact sizes and positions
// of each field since we might need to access them
// in assembly
struct process {
	struct trap_frame 	*frame;		// process[535:0]
	void 			*stack;		// process[543:536]
	size_t 			pc;		// process[551:544]
	uint16_t 		pid;		// process[553:552]
	struct page_table 	*pages;		// process[567:560]
	size_t 			state;		// process[575:568]
	size_t 			sleep_until;	// process[583:576]
};

// Create a new process from function pointer
struct process *process_create(void (*)(void));
void            process_switch_to_user(struct process *process);

// Defined in src/asm/crt0.s
void switch_to_user(size_t frame_addr, size_t pc, size_t satp);

#endif
