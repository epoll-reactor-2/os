#ifndef UART_H
#define UART_H

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

#define __uart_addr 0x10000000

void uart_init(void);
uint8_t uart_get(void);

void uart_put(uint8_t c);

#endif
