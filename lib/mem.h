#ifndef KERNEL_LIB_MEM_H
#define KERNEL_LIB_MEM_H

#include "cpu/type.h"

void memcpy(u8 *source, u8 *dest, int nbytes);
void memset(void *dest, u8 val, u32 len);
void free(void *addr);
u32 kmalloc(u32 size);

#endif /* KERNEL_LIB_MEM_H */