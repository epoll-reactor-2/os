#include "lib/stdio.h"
#include "lib/string.h"
#include "lib/mem.h"
#include "drivers/vga.h"
#include <stdarg.h>
#include <stdbool.h>

void kprint(const char *mem)
{
	vga_put_string(mem);
}

static inline s32 mod(s32 v)
{
	return v < 0 ? -v : v;
}

static inline char digit_symbol(s32 val)
{
	if (val < 10) return '0' + val;
	if (val < 36) return 'a' + val - 10;
	return '?';
}

static inline s32 digit_value(char digit)
{
	if (digit >= '0' && digit <= '9') return digit - '0';
	if (digit >= 'A' && digit <= 'Z') return digit - 'A' + 10;
	if (digit >= 'a' && digit <= 'z') return digit - 'a' + 10;
	return -1;
}

/* Thank you https://github.com/LekKit. */
size_t int_to_str_base(char *str, size_t size, s32 val, u8 base)
{
	size_t len = 0;
	s32 negative = val < 0;

	val = mod(val);
	if (base >= 2 && base <= 36) do {
		if (len + 1 >= size) {
			len = 0;
			break;
		}
		str[len++] = digit_symbol(val % base);
		val /= base;
	} while (val);

	if (negative)
		str[len++] = '-';

	/* Reverse the string. */
	for (size_t i = 0; i < len / 2; ++i) {
		char tmp = str[i];
		size_t r = len - i - 1;
		str[i] = str[r];
		str[r] = tmp;
	}

	if (size)
		str[len] = '\0';

	return len;
}

static u8 fmt_to_base(char c)
{
	switch (c) {
	case 'b': return  2;
	case 'o': return  8;
	case 'd': return 10;
	case 'x': return 16;
	default:  return -1;
	}
}

static s32 vprintf(const char* fmt, va_list list)
{
	s32 made = 0;
	char buf[16] = {0};

	while (*fmt) {
		if (*fmt != '%') {
			vga_put_byte(*fmt);
			++fmt;
			continue;
		}

		++fmt; /* Eat %. */

		/* TODO: Ultra simple version without any format specifiers.
		         Extend later. */
		switch (*fmt) {
		case 'b':
		case 'o':
		case 'd':
		case 'x': {
			s32 i = va_arg(list, int);
			int_to_str_base(buf, sizeof (buf) - 1, i, fmt_to_base(*fmt));
			vga_put_string(buf);
			++made;
			break;
		}
		case 'c': {
			char c = va_arg(list, int);
			vga_put_byte(c);
			++made;
			break;
		}
		case 's': {
			vga_put_string(va_arg(list, const char *));
			++made;
			break;
		}
		default:
			break;
		}

		++fmt;
	}

	return made;
}

void kprintf(const char *fmt, ...)
{
	va_list list;
	va_start(list, fmt);
	vprintf(fmt, list);
	va_end(list);
}