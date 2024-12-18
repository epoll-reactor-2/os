#ifndef TRAP_HANDLER_H
#define TRAP_HANDLER_H

#include <stddef.h>
#include "plic/trap_frame.h"

#define __cause_is_interrupt(cause)	(((size_t)(cause) >> 63) & 1)
#define __cause_exception_code(cause)	((size_t)(cause) & 0x7FFFFFFFFFFFFFFFull)

size_t m_mode_trap_handler(size_t, size_t, size_t, size_t, size_t,
			   struct trap_frame *);

#endif
