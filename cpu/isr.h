#ifndef KERNEL_CPU_ISR_H
#define KERNEL_CPU_ISR_H

#include "type.h"

/* ISRs reserved for CPU exceptions */
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

enum {
	IRQ0 	= 32,
	IRQ1 	= 33,
	IRQ2 	= 34,
	IRQ3 	= 35,
	IRQ4 	= 36,
	IRQ5 	= 37,
	IRQ6 	= 38,
	IRQ7 	= 39,
	IRQ8 	= 40,
	IRQ9 	= 41,
	IRQ10 	= 42,
	IRQ11 	= 43,
	IRQ12 	= 44,
	IRQ13 	= 45,
	IRQ14 	= 46,
	IRQ15 	= 47,
};

/* Struct which aggregates many registers
   TODO: Where this structure and order of registers
         is specified? */
struct registers {
	u32 ds; /* Data segment selector */
	u32 edi, esi, ebp, esp, ebx, edx, ecx, eax; /* Pushed by pusha. */
	u32 int_no, err_code; /* Interrupt number and error code (if applicable) */
	u32 eip, cs, eflags, useresp, ss; /* Pushed by the processor automatically */
};

void isr_install();
void isr_handler(struct registers r);

typedef void (*isr_t)(struct registers);
void irq_install_handler(u8 n, isr_t handler);
void irq_install();

/* Notify PIC controller that interrupt processing is finished.

   NOTE: It just tells the PIC that we are ready
         to receive more IRQ's. */
void irq_eoi(u32 int_no);

#endif /* KERNEL_CPU_ISR_H */