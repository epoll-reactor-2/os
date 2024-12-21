#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <limits.h>
#include "devices/uart.h"

/// Initialize NS16550A UART
void uart_init(void)
{
	volatile uint8_t *ptr = (uint8_t *) __uart_addr;

	// Set word length to 8 (LCR[1:0])
	const uint8_t lcr = 0b11;
	ptr[3] = lcr;

	// Enable FIFO (FCR[0])
	ptr[2] = 0b1;

	// Enable receiver buffer interrupts (IER[0])
	ptr[1] = 0b1;

	// For a real UART, we need to compute and set the baud rate
	// But since this is an emulated UART, we don't need to do anything
	// 
	// Assuming clock rate of 22.729 MHz, set signaling rate to 2400 baud
	// divisor = ceil(CLOCK_HZ / (16 * BAUD_RATE))
	// = ceil(22729000 / (16 * 2400))
	// = 592
	// 
	// uint16_t divisor = 592;
	// uint8_t divisor_least = divisor & 0xFF;
	// uint8_t divisor_most = divisor >> 8;
	// ptr[3] = LCR | 0x80;
	// ptr[0] = divisor_least;
	// ptr[1] = divisor_most;
	// ptr[3] = LCR;
}

void uart_put(uint8_t c)
{
	*(uint8_t *) __uart_addr = c;
}

uint8_t uart_get(void)
{
	return *(uint8_t *) __uart_addr;
}