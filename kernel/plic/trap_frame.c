#include <stddef.h>
#include "plic/trap_frame.h"

static struct trap_frame frame = __trap_frame_zero;

struct trap_frame *get_kernel_trap_frame(void)
{
	return &frame;
}
