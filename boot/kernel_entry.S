global _start
[bits 32]
_start:
	[extern kernel_main]
	call kernel_main
	; kernel_main() has no loops, so starting
	; from this point of time, we are in the
	; infinite loop that does nothing. The
	; only way to handle OS is use interrupts
	; (PS/2 devices, other) and exceptions.
	;
	; Notice, hlt instruction in this place is
	; crucial and don't lets CPU use 100% of their
	; resources for no reason.
__wait_for_interrupt:
	hlt
	jmp __wait_for_interrupt
