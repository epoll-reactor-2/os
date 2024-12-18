#ifndef TRAP_FRAME_H
#define TRAP_FRAME_H

#include <stddef.h>

#define __num_regs	32
#define __num_fregs	32

struct trap_frame {
	size_t	regs[__num_regs]; 	/* General purpose registers */
	size_t 	fregs[__num_fregs];	/* FPU registers */

	size_t 	satp;
	void	*trap_stack;
	size_t	hart_id;
};

#define __trap_frame_zero ((struct trap_frame){	\
	.regs		= {},			\
	.fregs		= {},			\
	.satp		= 0,			\
	.trap_stack	= NULL,			\
	.hart_id	= 0			\
})

struct trap_frame *get_kernel_trap_frame(void);

#endif
