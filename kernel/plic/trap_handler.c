#include <stdint.h>
#include "devices/uart.h"
#include "libc/stdio.h"
#include "mm/sv39.h"
#include "mm/page.h"
#include "plic/trap_handler.h"
#include "plic/cpu.h"
#include "plic/plic.h"
#include "syscon/syscon.h"
#include "printk/printk.h"
#include "process/syscall.h"
#include "process/sched.h"
#include "process/process.h"
#include "macro.h"

// Handle only the following interrupts for now:
//
// - Load page faults
// - Store/AMO page faults
// - Timer interrupts
// - External interrupts (UART only)
//
// Panic on all other interrupts for the time being, so we know there's
// an issue with our code when we get an unexpected type of interrupt
size_t m_mode_trap_handler(size_t epc, size_t tval, size_t cause, size_t hart,
			   size_t status, struct trap_frame *frame) {
	size_t return_pc = epc;
	size_t exception_code = __cause_exception_code(cause);

	printk("trap handler sp: %x\n", frame->regs[2]);

	if (__cause_is_interrupt(cause)) {
		switch (exception_code) {
		case 7: {
			// Timer interrupt
			set_timer_interrupt_delay_us(1 * __us_per_second);
			struct process *process = sched_schedule();
			__assert(process != NULL,
				"m_mode_trap_handler(): unexpected got NULL when attempting to schedule next process\n");

			printk("Context switch: scheduling next process with PID = %d\n",
				process->pid);

			process_switch_to_user(process);

			break;
		}

		case 11: {
			// External interrupt (UART)
			uint32_t claim = __plic_claim();
			__assert(claim == __plic_uart,
				"m_mode_trap_handler(): unknown interrupt source #%d with machine external interrupt\n",
				claim
			);

			// 
			uint8_t rcvd = kgetchar();
			switch (rcvd) {
			case 3:
				poweroff();

			case 13:
				kprintf("\n");
				break;

			case 127:
				kprintf("%c %c", 8, 8);
				break;

			default:
				kprintf("%c", rcvd);
			}

			__plic_complete(claim);
			break;
		}

		default:
			__panic("m_mode_trap_handler(): unknown interrupt with exception code %d\n",
				exception_code);
		}

	} else {
		switch (exception_code) {
		case 1:
			printk("Instruction address misaligned at EPC: %p\n", epc);
			printk("Stack pointer (sp): %p\n", frame->regs[2]);
   			__halt();
			break;

		case 2:
			// Illegal instruction
			kprintf("--- Kernel panic ---\n");
			kprintf("Illegal instr\n");
			__halt();
			break;

		case 3:
			// FPU disabled trap
			printk("FPU disabled trap at EPC: %p\n", epc);
			break;

		case 7:
			__panic("Non-naturally aligned store access attempt to an I/O region.\n"
				"Store-Conditional or Atomic Memory Operation (AMO) attempt to\n"
				"region without atomic support. Store attempt with address failing PMP check.\n");
			__halt();
			break;

		case 8: {
			uint64_t sp;
			__asm__ __volatile__ ("mv %0, sp" : "=r"(sp));
			__asm__ __volatile__ (
				"mv a0, %1\n"		/* 0 argument		*/
				"mv a1, %2\n"		/* 1 argument		*/
				"mv sp, %3\n"		/* Stack pointer	*/
				"call do_syscall\n"
				: "=r"(return_pc)	/* Get return value	*/
				:  "r"(return_pc), "r"(frame), "r"(frame->regs[2])
			);
			__asm__ __volatile__ ("mv sp, %0" :: "r"(sp));
			break;
		}

		case 13:
			// Load page fault
			printk("Load page fault: attempted to dereference address %p\n", tval);
			return_pc += 4;
			break;

		case 15:
			// Store/AMO page fault
			printk("Store/AMO page fault: attempted to dereference address %p\n",
				tval);

			return_pc += 4;
			break;

		default:
			__panic("m_mode_trap_handler(): unknown synchronous trap with exception code %d\n",
				exception_code);

		}
	}
	return return_pc;
}
