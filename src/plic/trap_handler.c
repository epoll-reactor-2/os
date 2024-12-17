#include <stdint.h>
#include "trap_handler.h"
#include "../uart/uart.h"
#include "../common/common.h"
#include "cpu.h"
#include "plic.h"
#include "../syscon/syscon.h"
#include "../process/syscall.h"
#include "../process/sched.h"
#include "../process/process.h"
#include "../mm/sv39.h"
#include "../mm/page.h"

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

	if (__cause_is_interrupt(cause)) {
		switch (exception_code) {
		case 7: {
			// Timer interrupt
			set_timer_interrupt_delay_us(1 * __us_per_second);
			struct process *process = sched_schedule();
			__assert(process != NULL,
				"m_mode_trap_handler(): unexpected got NULL when attempting to schedule next process\n");

			kprintf("Context switch: scheduling next process with PID = %d\n",
				process->pid);

			switch_to_user((size_t)process->frame, process->pc,
				__satp_from(__mode_sv39, process->pid,
				(size_t)process->root >> __page_order));

			break;
		}

		case 11: {
			// External interrupt (UART)
			uint32_t claim = __plic_claim();
			__assert(claim == __plic_uart,
				"m_mode_trap_handler(): unknown interrupt source #%d with machine external interrupt\n",
				claim
			);

			uint8_t rcvd = uart_get();
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
		case 2:
			// Illegal instruction
			__halt();
			break;

		case 8:
			// U-mode syscall
			return_pc = do_syscall(return_pc, frame);
			break;

		case 13:
			// Load page fault
			kprintf("Load page fault: attempted to dereference address %p\n", tval);
			return_pc += 4;
			break;

		case 15:
			// Store/AMO page fault
			kprintf("Store/AMO page fault: attempted to dereference address %p\n",
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
