#include "libc/stdio.h"
#include "config.h"

#if CONFIG_UART
#include "devices/uart.h"
#endif /* CONFIG_UART */

#if CONFIG_FB
#include "devices/fb.h"
#endif /* CONFIG_FB */

#include "libc/ctype.h"
#include <stdint.h>
#include <stddef.h>
#include <limits.h>

int kputchar(int c)
{
#if CONFIG_UART
	uart_put((uint8_t) c);
#endif /* CONFIG_UART */

#if CONFIG_FB
	fb_put((char) c);
#endif /* CONFIG_FB */

	return c;
}

static void kprint(const char *str)
{
	while (*str) {
		kputchar((int) *str);
		++str;
	}
}

int kputs(const char *str)
{
	kprint(str);
	kputchar((int)'\n');
	return 0;
}

static void print_number(uint64_t n, int base, int uppercase)
{
	char buf[20] = {0};
	char *p = buf;
	char *alphabet = uppercase
		? "0123456789ABCDEF"
		: "0123456789abcdef";

	if (n == 0)
		*p++ = alphabet[0];

	while (n) {
		*p++ = alphabet[n % base];
		n /= base;
	}

	while (p != buf)
		kputchar(*--p);
}

#define __to_hex_digit(n) ('0' + (n) + ((n) < 10 ? 0 : 'a' - '0' - 10))

static void print_number_hex(uint64_t n, int base, int uppercase)
{
	char buf[20] = {0};
	char *p = buf;
	char lsh = __to_hex_digit(n % 16);
	n /= base;

	while (n) {
		*p++ = __to_hex_digit(n % 16);
		n /= base;
	}

	while (p != buf)
		kputchar(*--p);

	if (uppercase)
		kputchar(toupper(lsh));
	else
		kputchar(lsh);
}

static size_t number_len(int64_t n, int base)
{
	size_t len = (n == 0) ? 1 : 0;

	if (n < 0)
		n = -n;

	while (n > 0) {
		n /= base;
		len++;
	}

	return len;
}

static void print_float(double value, int precision)
{
	if (value < 0) {
		kputchar('-');
		value = -value;
	}

	uint64_t i = (uint64_t) value;
	double frac = value - (double) i;

	print_number(i, 10, /* uppercase */ 0);

	kputchar('.');

	for (int i = 0; i < precision; i++) {
		frac *= 10;
		int digit = (int) frac;
		kputchar('0' + digit);
		frac -= digit;
	}
}

// Limited version of vprintf() which only supports the following
// specifiers:
// 
// +------+-------------------------------------------+
// | d/i  | Signed decimal integer                    |
// | u    | Unsigned decimal integer                  |
// | o    | Unsigned octal                            |
// | x    | Unsigned hexadecimal integer              |
// | X    | Unsigned hexadecimal integer (uppercase)  |
// | c    | Character                                 |
// | s    | String of characters                      |
// | p    | Pointer address                           |
// | %    | Literal '%'                               |
// +------+-------------------------------------------+
// 
// None of the sub-specifiers are supported for the sake of simplicity.
// The `n` specifier is not supported since that is a major source of
// security vulnerabilities. None of the floating-point specifiers are
// supported since floating point operations don't make sense in kernel
// space
// 
// Anyway, this subset should suffice for printf debugging
void kvprintf(const char *format, va_list arg)
{
	while (*format) {
		if (*format == '%') {
			++format;
			if (!*format)
				return;

			// Handle width specifier
			int width = 0;
			while (*format >= '0' && *format <= '9')
			{
				width = width * 10 + (*format - '0');
				++format;
			}

			switch (*format) {
			case 'd':
			case 'i': {
				int n = va_arg(arg, int);
				if (n == INT_MIN) {
					kprint("-2147483648");
					break;
				}

				int neg = n < 0;

				if (neg) {
					kputchar('-');
					n = ~n + 1;
				}

				int abs_n = neg ? -n : n;
				int len = number_len(abs_n, 10) + (neg ? 1 : 0);
				int padding = (width > len) ? (width - len) : 0;

				print_number(n, /* base */ 10, /* uppercase */ 0);

				while (padding-- > 0)
					kputchar(' ');

				break;
			}

			case 'f': {
				double n = va_arg(arg, double);

				int neg = n < 0;

				int abs_n = neg ? -n : n;
				int len = number_len(abs_n, 10) + (neg ? 1 : 0);
				int padding = (width > len) ? (width - len) : 0;

				print_float(n, 6);

				while (padding-- > 0)
					kputchar(' ');

				break;
			}

			case 'u': {
				unsigned n = va_arg(arg, unsigned);
				print_number(n, /* base */ 10, /* uppercase */ 0);
				break;
			}
			case 'o': {
				unsigned n = va_arg(arg, unsigned);
				print_number(n, /* base */ 8, /* uppercase */ 0);
				break;
			}
			case 'x': {
				unsigned n = va_arg(arg, unsigned);

				int neg = n < 0;

				if (neg) {
					kputchar('-');
					n = ~n + 1;
				}

				int abs_n = neg ? -n : n;
				int len = number_len(abs_n, 10) + (neg ? 1 : 0);
				int padding = (width > len) ? (width - len) : 0;

				print_number(n, /* base */ 16, /* uppercase */ 0);

				while (padding-- > 0)
					kputchar(' ');

				break;
			}

			case 'X': {
				unsigned n = va_arg(arg, unsigned);

				int neg = n < 0;

				if (neg) {
					kputchar('-');
					n = ~n + 1;
				}

				int abs_n = neg ? -n : n;
				int len = number_len(abs_n, 10) + (neg ? 1 : 0);
				int padding = (width > len) ? (width - len) : 0;

				print_number(n, /* base */ 16, /* uppercase */ 1);

				while (padding-- > 0)
					kputchar(' ');

				break;
			}
			case 'c':
				kputchar(va_arg(arg, int));
				break;
			case 's':
				kprint(va_arg(arg, char *));
				break;
			case 'p': {
				kprint("0x");
				size_t n = va_arg(arg, size_t);
				print_number(n, /* base */ 16, /* uppercase */ 0);
				break;
			}
			case '%':
				kputchar('%');
				break;
			default:
				kputchar('%');
				kputchar(*format);
				break;
			} /* switch */
		} else
			kputchar(*format);
		++format;
	}
}

void kprintf(const char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	kvprintf(format, arg);
	va_end(arg);
}

int kgetchar(void)
{
#if CONFIG_UART
	return uart_get();
#endif /* CONFIG_UART */
}