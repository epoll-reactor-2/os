#include "idt.h"

static struct idt_gate idt[IDT_ENTRIES];
static struct idt_register idt_reg;

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
	__asm__ __volatile__("lidtl (%0)" : : "r" (&idt_reg));
}
