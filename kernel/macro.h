#ifndef MACRO_H
#define MACRO_H

#include "plic/cpu.h"
#include "printk/printk.h"

#define __section(x) __attribute__ ((section (x)))
#define __aligned(x) __attribute__ ((aligned (x)))

#define __halt() ({                                    \
       __set_mie(0);                                   \
       __asm__ __volatile__ ("wfi");                   \
})

#define __panic(format, ...) ({\
       printk("Kernel panic at %s:%d:\n" format,       \
               __FILE__,                               \
               __LINE__                                \
               __VA_OPT__(,) __VA_ARGS__               \
       );                                              \
       __halt();                                       \
})

#define __assert(condition, format, ...) ({            \
       if (!(condition))                               \
               __panic("Failed asserting that %s\n" format, \
                       #condition                      \
                       __VA_OPT__(,) __VA_ARGS__       \
               );                                      \
})

#endif
