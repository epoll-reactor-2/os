#include "mem.h"
#include "cpu/type.h"

void *memcpy(void *__dst, const void *__src, size_t n)
{
	u8 *dst = __dst;
	const u8 *src = __src;

	for (size_t i = 0; i < n; i++)
		dst[i] = src[i];

	return dst;
}

void *memset(void *s, s32 c, size_t n)
{
	u8 *temp = (u8 *) s;
	while (n-- > 0)
		*temp++ = c;

	return s;
}