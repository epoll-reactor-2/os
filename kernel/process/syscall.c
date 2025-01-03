#include "process/syscall.h"
#include "printk/printk.h"
#include "macro.h"

size_t do_syscall(size_t mepc, struct trap_frame *frame)
{
	// a0 = x10
	size_t syscall_number = frame->regs[10];

	int a = 0;
	int b = 0;

	switch (syscall_number) {
	case __syscall_exit:
		__panic("do_syscall(): exit() system call not implemented (syscall %d)\n",
			syscall_number);

	case __syscall_test:
		/* It is easy to notice, that each process has same address
		   space. Separate it. */
		++a;
		++b;
		printk("process stack: %8x, %d\n", &a, a);
		printk("process stack: %8x, %d\n", &b, b);
		printk("__syscall_test()\n");
		return mepc + 4;

	default:
		// FIXME: handle this gracefully as errors in user space should not
		// bring down the system
		__panic("do_syscall(): unknown system call %d\n", syscall_number);
	}

}
