/* https://wiki.osdev.org/IRQ */
#include "isr.h"
#include "idt.h"
#include "ports.h"
#include "timer.h"
#include "lib/stdio.h"
#include "lib/string.h"
#include "drivers/ps2_keyboard.h"
#include "drivers/ps2_mouse.h"

enum {
	PIC1		= 0x20,		/* IO base address for master PIC. */
	PIC2		= 0xA0,		/* IO base address for slave PIC. */
	PIC1_COMMAND	= PIC1,
	PIC1_DATA	= PIC1 + 1,
	PIC2_COMMAND	= PIC2,
	PIC2_DATA	= PIC2 + 1,

	PIC_EOI		= 0x20,		/* End of interrupt. */
};

enum {
/* ICW - Initialization command word. */
	ICW1_ICW4	= 0x01,		/* Indicates that ICW4 will be present. */
	ICW1_SINGLE	= 0x02,		/* Single (cascade) mode. */
	ICW1_INTERVAL4	= 0x04,		/* Call address interval 4 (8). */
	ICW1_LEVEL	= 0x08,		/* Level triggered (edge) mode. */
	ICW1_INIT	= 0x10,		/* Initialization - required! */
};

enum {
	ICW4_8086	= 0x01,		/* 8086/88 (MCS-80/85) mode. */
	ICW4_AUTO	= 0x02,		/* Auto (normal) EOI. */
	ICW4_BUF_SLAVE	= 0x08,		/* Buffered mode/slave. */
	ICW4_BUF_MASTER	= 0x0C,		/* Buffered mode/master. */
	ICW4_SFNM	= 0x10,		/* Special fully nested (not). */
};

isr_t interrupt_handlers[256];

void isr_install()
{
	idt_set_gate(0,  (u32) isr0);
	idt_set_gate(1,  (u32) isr1);
	idt_set_gate(2,  (u32) isr2);
	idt_set_gate(3,  (u32) isr3);
	idt_set_gate(4,  (u32) isr4);
	idt_set_gate(5,  (u32) isr5);
	idt_set_gate(6,  (u32) isr6);
	idt_set_gate(7,  (u32) isr7);
	idt_set_gate(8,  (u32) isr8);
	idt_set_gate(9,  (u32) isr9);
	idt_set_gate(10, (u32) isr10);
	idt_set_gate(11, (u32) isr11);
	idt_set_gate(12, (u32) isr12);
	idt_set_gate(13, (u32) isr13);
	idt_set_gate(14, (u32) isr14);
	idt_set_gate(15, (u32) isr15);
	idt_set_gate(16, (u32) isr16);
	idt_set_gate(17, (u32) isr17);
	idt_set_gate(18, (u32) isr18);
	idt_set_gate(19, (u32) isr19);
	idt_set_gate(20, (u32) isr20);
	idt_set_gate(21, (u32) isr21);
	idt_set_gate(22, (u32) isr22);
	idt_set_gate(23, (u32) isr23);
	idt_set_gate(24, (u32) isr24);
	idt_set_gate(25, (u32) isr25);
	idt_set_gate(26, (u32) isr26);
	idt_set_gate(27, (u32) isr27);
	idt_set_gate(28, (u32) isr28);
	idt_set_gate(29, (u32) isr29);
	idt_set_gate(30, (u32) isr30);
	idt_set_gate(31, (u32) isr31);

	/* https://pdos.csail.mit.edu/6.828/2010/readings/hardware/8259A.pdf */
	port_byte_out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
	port_byte_out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	port_byte_out(PIC1_DATA,    0x20);
	port_byte_out(PIC2_DATA,    0x28);
	/* ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100) */
	port_byte_out(PIC1_DATA,    0x04);
	/* ICW3: tell Slave PIC its cascade identity (0000 0010) */
	port_byte_out(PIC2_DATA,    0x02);
	/* ICW4: have the PICs use 8086 mode (and not 8080 mode) */
	port_byte_out(PIC1_DATA,    ICW4_8086);
	port_byte_out(PIC2_DATA,    ICW4_8086);
	/* Restore saved masks. */
	port_byte_out(PIC1_DATA,    0x00);
	port_byte_out(PIC2_DATA,    0x00);

	idt_set_gate(32, (u32) irq0);
	idt_set_gate(33, (u32) irq1);
	idt_set_gate(34, (u32) irq2);
	idt_set_gate(35, (u32) irq3);
	idt_set_gate(36, (u32) irq4);
	idt_set_gate(37, (u32) irq5);
	idt_set_gate(38, (u32) irq6);
	idt_set_gate(39, (u32) irq7);
	idt_set_gate(40, (u32) irq8);
	idt_set_gate(41, (u32) irq9);
	idt_set_gate(42, (u32) irq10);
	idt_set_gate(43, (u32) irq11);
	idt_set_gate(44, (u32) irq12);
	idt_set_gate(45, (u32) irq13);
	idt_set_gate(46, (u32) irq14);
	idt_set_gate(47, (u32) irq15);

	/* Load initialized IDT table to RAM. */
	idt_set();
}

/* I guess, proper handler should be implemented for
   each exception. As a rule, we must to terminate kernel
   and die. */
static const char *exception_messages[] = {
	"Division By Zero",
	"Debug",
	"Non Maskable Interrupt",
	"Breakpoint",
	"Into Detected Overflow",
	"Out of Bounds",
	/* Note on invalid opcode.

	   QEMU throws this exception,
	   for example, when we attempt to dereference NULL. */
	"Invalid Opcode",
	"No Coprocessor",

	"Double Fault",
	"Coprocessor Segment Overrun",
	"Bad TSS",
	"Segment Not Present",
	"Stack Fault",
	"General Protection Fault",
	"Page Fault",
	"Unknown Interrupt",

	"Coprocessor Fault",
	"Alignment Check",
	"Machine Check",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",

	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

const char *__data = "Data";

void isr_handler(struct registers r)
{
	kprintf("received interrupt: %d, %s\n", r.int_no, exception_messages[r.int_no]);
}

void irq_install_handler(u8 n, isr_t handler)
{
	interrupt_handlers[n] = handler;
}

/* NOTE: This function periodically called to handle
         IRQ0 - timer interrupt.

   NOTE: This function is called when hardware/software
         interrupt is triggered. In responce, we send
         ack */
void irq_handler(struct registers r)
{
	/* After every interrupt we need to send an EOI to the PICs
	* or they will not send another interrupt again */

	/* Handle the interrupt in a more modular way */
	if (interrupt_handlers[r.int_no] != 0) {
		isr_t handler = interrupt_handlers[r.int_no];
		handler(r);
	}

	irq_eoi(r.int_no);
}

void irq_install()
{
	/* Set the interrupt flag (IF) in the EFLAGS register.
	   Or simply "enable interrupts". */
	asm volatile("sti");
	timer_init(50);
	keyboard_install();
	mouse_install();
}

void irq_eoi(u32 int_no)
{
	if (int_no >= 40)
		port_byte_out(PIC2, PIC_EOI); /* Slave. */
	port_byte_out(PIC1, PIC_EOI); /* Master. */
}