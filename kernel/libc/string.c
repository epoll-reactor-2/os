#include "libc/string.h"
#include <stddef.h>

char *strcpy(char *dst, const char *src)
{
	if (!dst || !src)
		return NULL;

	char *tmp = dst;
	while (*src)
		*tmp++ = *src++;

	*tmp = '\0';

	return dst;
}

void *memcpy(void *dst, const void *src, size_t len)
{
	char *d = dst;
	const char *s = src;

	while (len--)
		*d++ = *s++;

	return dst;
}