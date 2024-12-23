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
		printk("process stack: %x\n", &a);
		printk("process stack: %x\n", &b);
		printk("__syscall\t_test()\n");
		printk("__syscall1\t_test()\n");
		printk("__syscall11\t_test()\n");
		printk("__syscall111\t_test()\n");
		printk("__syscall1111\t_test()\n");
		printk("__syscall11111\t_test()\n");
		printk("__syscall111111\t_test()\n");
		printk("__syscall1111111\t_test()\n");
		return mepc + 4;

	default:
		// FIXME: handle this gracefully as errors in user space should not
		// bring down the system
		__panic("do_syscall(): unknown system call %d\n", syscall_number);
	}
}
