#ifndef PLIC_H
#define PLIC_H

#define __plic_addr 0xc000000

/*
 * PLIC registers
 * See chapter 10 of the SiFive FU540-C000 Manual for details
 * https://sifive.cdn.prismic.io/sifive%2F834354f0-08e6-423c-bf1f-0cb58ef14061_fu540-c000-v1.0.pdf
 */

// Priority registers
// For each of the 53 interrupt sources numbered 1 through 53
// (0 is reserved), __plic_addr + 4 * source is a 32-bit register
// that specifies the priority of the numbered interrupt source
// Priorities must be within the range [0, 8) where 0 means the
// interrupt source is disabled and 7 is the highest priority
#define __plic_set_prio(source, priority) ({			\
	((uint32_t *) __plic_addr)[source] = (priority) & 0x7;	\
})

// Interrupt pending bits
// Starting from __plic_addr + 0x1000 we have two 32-bit read-only
// interrupt pending registers `pending0`, `pending1` specifying
// which interrupt sources are pending
// pending0[31:0] specifies which source in the range [0, 32)
// has an interrupt pending based on the bit position (pending[0]
// is hardwired to 0)
// pending1[21:0] specifies which source in the range [32, 54)
// has an interrupt pending based on the bit position, with the
// bit position = source + 32
// The remaining bits pending1[31:22] are reserved and WIRI
// (write illegal, read illegal)
#define __plic_pending(source) \
	(!((source) & ~0x1Full) ? \
		*(uint32_t *) &((uint8_t *) __plic_addr)[0x1000] & (1ull << (source)) :\
		*(uint32_t *) &((uint8_t *) __plic_addr)[0x1004] & (1ull << ((source) & 0x1F)))

// Interrupt enables (`enables0`, `enables1`)
// Similar to interrupt pending bits, but starting from
// __plic_addr + 0x2000 and the valid bits are writable (except
// enables0[0] which is read-only 0), to specify which interrupt
// source(s) to enable
#define __plic_enable(source) ({								\
	if (!((source) & ~0x1Full))								\
		*(uint32_t *) &((uint8_t *) __plic_addr)[0x2000] = 1ull << (source);		\
	else											\
		*(uint32_t *) &((uint8_t *) __plic_addr)[0x2004] = 1ull << ((source) & 0x1F);	\
})

// Priority threshold
// __plic_addr + 0x200000 is a 32-bit register threshold[31:0]
// specifying which priority level interrupts <= threshold
// should be masked
// The only valid values are [0, 8) where 0 means "do not mask"
// and 7 means "mask all interrupts"
#define __plic_set_threshold(threshold) ({							\
	*(uint32_t *) &((uint8_t *) __plic_addr)[0x200000] = (threshold) & 0x7;			\
})

// Interrupt claim/complete register
// __plic_addr + 0x200004 is a 32-bit register for claiming (read)
// and completing (write) interrupts, where the value specifies the
// interrupt source
// Claiming an interrupt means the OS kernel informs the hardware that
// it is aware there is an interrupt waiting to be handled, and that
// it will process the interrupt
// Completing an interrupt means the OS kernel informs the hardware
// that it has finished handling the interrupt and ready to wait for
// the next interrupt from the same source
#define __plic_claim() (*(uint32_t *) &((uint8_t *) __plic_addr)[0x200004])
#define __plic_complete(source) ({\
	*(uint32_t *) &((uint8_t *) __plic_addr)[0x200004] = (source);\
})

// UART interrupt
// From our device tree we see that UART has interrupts = <0x0a>,
// i.e. it has source = 10
#define __plic_uart 10

#endif
