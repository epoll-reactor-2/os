#ifndef KERNEL_LIB_MEM_H
#define KERNEL_LIB_MEM_H

#include "cpu/type.h"

void *memcpy(void *__dst, const void *__src, size_t n);
void *memset(void *s, s32 c, size_t n);

#endif /* KERNEL_LIB_MEM_H */