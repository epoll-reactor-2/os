#ifndef KERNEL_LIB_STRING_H
#define KERNEL_LIB_STRING_H

#include "cpu/type.h"

size_t strlen(char s[]);
char *strcat(char *dst, const char* src);
void strcpy(char *dst, char *src);
void backspace(char s[]);
void append(char s[], char n);
size_t strcmp(char *s1, char *s2);

#endif /* KERNEL_LIB_STRING_H */