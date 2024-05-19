#include "string.h"
#include "cpu/type.h"

size_t strlen(char s[])
{
	size_t i = 0;
	while (s[i] != '\0') ++i;
	return i;
}

char *strcat(char *dst, const char* src)
{
	char *rdst = dst;

	while (*dst)
		dst++;

	while ((*(dst++) = *(src++)))
		;

	return rdst;
}

void strcpy(char *dst, char *src)
{
	int len = strlen(src);
	for (int i = 0; i < len; i++) {
		dst[i] = src[i];
	}
	dst[len] = '\0';
}

void append(char s[], char n)
{
	int len = strlen(s);
	s[len] = n;
	s[len+1] = '\0';
}

void backspace(char s[])
{
	int len = strlen(s);
	s[len-1] = '\0';
}

/* BoP pointers and strings presentation */
size_t strcmp(char *s1, char *s2)
{
	while (*s1 != '\0' && *s1 == *s2) {
		s1++;
		s2++;
	}
	return *s1 - *s2;
}