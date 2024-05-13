#include "stdlib.h"

uint64_t strlen(const char* str)
{
	uint64_t len = 0;
	while (str[len])
		len++;
	return len;
}

void *memset (void *__s, int __c, uint64_t __n)
{
	uint8_t *ptr = __s;
	while (__n-- > 0)
		*ptr++ = __c;
	return __s;
}