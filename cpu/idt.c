#include "idt.h"

struct idt_gate idt[IDT_ENTRIES];
struct idt_register idt_reg;

void idt_set_gate(int n, u32 handler)
{
	idt[n].low_offset = low_16(handler);
	idt[n].sel = KERNEL_CS;
	idt[n].always0 = 0;
	idt[n].flags = 0x8E; 
	idt[n].high_offset = high_16(handler);
}

void idt_set()
{
	idt_reg.base = (u32) &idt;
	idt_reg.limit = IDT_ENTRIES * sizeof(struct idt_gate) - 1;
	/* Don't make the mistake of loading &idt -- always load &idt_reg */
	__asm__ __volatile__("lidtl (%0)" : : "r" (&idt_reg));
}
