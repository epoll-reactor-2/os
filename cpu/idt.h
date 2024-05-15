#ifndef KERNEL_CPU_IDT_H
#define KERNEL_CPU_IDT_H

#include "cpu/type.h"
#include "kernel/compiler.h"

/* Segment selectors */
#define KERNEL_CS 0x08

/* How every interrupt gate (handler) is defined */
struct idt_gate {
	u16	low_offset; /* Lower 16 bits of handler function address */
	u16	sel; /* Kernel segment selector */
	u8 	always0;
	/* First byte
	   Bit    7: "Interrupt is present"
	   Bits 6-5: Privilege level of caller (0=kernel..3=user)
	   Bit    4: Set to 0 for interrupt gates
	   Bits 3-0: bits 1110 = decimal 14 = "32 bit interrupt gate" */
	u8 	flags; 
	u16 	high_offset; /* Higher 16 bits of handler function address */
} __packed;

/* A pointer to the array of interrupt handlers.
   Assembly instruction 'lidt' will read it */
struct idt_register {
    u16		limit;
    u32		base;
} __packed;

void idt_set_gate(int n, u32 handler);
void idt_set();

#endif /* KERNEL_CPU_IDT_H */