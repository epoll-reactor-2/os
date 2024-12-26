# Disable generation of compressed instructions
# This is to avoid complications when setting values
# of CSRs such as mtvec and stvec which have alignment
# constraints
.option norvc

# Common symbols
.set NUM_GP_REGS, 32
.set REG_SIZE, 8
.set NUM_FPU_REGS, 32
.set FPU_REG_SIZE, 8

# Use alternative macro syntax (see GNU assembler docs for details)
.altmacro

# Common macros
.macro save_gp i, basereg=t6
	sd x\i, \i * REG_SIZE(\basereg)
.endm

.macro load_gp i, basereg=t6
	ld x\i, \i * REG_SIZE(\basereg)
.endm

.macro save_fpu_regs i, basereg=t6
#	fsd f\i, \i * FPU_REG_SIZE(\basereg)
.endm

.macro load_fpu_regs i, basereg=t6
#	fld f\i, \i * FPU_REG_SIZE(\basereg)
.endm

# Importation of linker symbols
.section .rodata
.global __heap_start
__heap_start: .dword __ld_heap_start

.global __heap_size
__heap_size: .dword __ld_heap_size

.global __init_start
__init_start: .dword __ld_init_start

.global __init_end
__init_end: .dword __ld_init_end

.global __text_start
__text_start: .dword __ld_text_start

.global __text_end
__text_end: .dword __ld_text_end

.global __rodata_start
__rodata_start: .dword __ld_rodata_start

.global __rodata_end
__rodata_end: .dword __ld_rodata_end

.global __data_start
__data_start: .dword __ld_data_start

.global __data_end
__data_end: .dword __ld_data_end

.global __bss_start
__bss_start: .dword __ld_bss_start

.global __bss_end
__bss_end: .dword __ld_bss_end

.global __kernel_stack_start
__kernel_stack_start: .dword __ld_kernel_stack_start

.global __kernel_stack_end
__kernel_stack_end: .dword __ld_kernel_stack_end

.global __make_syscall
__make_syscall: .dword make_syscall

.section .data
.global __kernel_table
__kernel_table: .dword 0

.section .init, "ax"
.global _start
_start:
	# Initialize CSRs for M-mode

	# Supervisor address translation and protection
	# SATP should already be zero, but just to make sure ...
	csrw satp, zero

	# Machine status
	# MPP = mstatus[12:11]
	#      MPP=3 (M-level access with no translation)
	li t0, 0b11 << 11
	csrw mstatus, t0

	# Enable FPU
	li t0, 0x6000
	csrs mstatus, t0

	# Machine exception program counter
	# Set this to kernel_main so executing mret jumps to kernel_main
	la t0, kernel_main
	csrw mepc, t0

	# Do not allow interrupts in M-mode
	csrw mie, zero

	# Zero the BSS section
	la t0, __bss_start
	la t1, __bss_end
__bss_zero_loop_start:
	bgeu t0, t1, __bss_zero_loop_end
	sd zero, 0(t0)
	addi t0, t0, 8
	j __bss_zero_loop_start
__bss_zero_loop_end:
	# Initialize global pointer register
	.option push
	.option norelax
	la gp, __ld_global_pointer
	.option pop

	# Initialize stack and frame pointer registers
	la sp, __ld_kernel_stack_end
	mv fp, sp

	# If kernel_main returns, we're done with everything so halt forever
	la ra, halt_forever

	# Now jump to kernel_main for M-mode initialization
	mret

# We're already done with everything - let's halt forever
halt_forever:
	csrw mie, zero
	wfi
	j halt_forever

# Interrupt handler
interrupt_handler:
	# Save all general purpose registers into kernel trap frame
	# No need to save floating point registers since we haven't used them yet
	# No need to save satp, trap_stack since we don't modify them
	# No need to save hartid since that is always 0
	# (we only have a single CPU core)
	# This requires a bit of trickery to do correctly:
	# 
	# 0. mscratch has address of kernel trap frame - see kinit() for details
	# 1. Atomically swap mscratch and t6 registers
	#    Now t6 has address of kernel trap frame, and mscratch the
	#    original value of t6
	# 2. Now save registers x1-x30 into kernel trap frame, using
	#    t6=x31 as base
	#    No need to save zero=x0 since that is read-only zero
	# 3. Move address of kernel trap frame to t5=x30 so we don't lose it
	# 4. Move mscratch (= original value of t6) back into t6 and save that
	# 5. Write address of kernel trap frame from t5 back into mscratch
	csrrw t6, mscratch, t6
	.set i, 1
	.rept 30
		save_gp %i
		.set i, i + 1
	.endr

	.set i, 0
	.rept NUM_FPU_REGS
		save_fpu_regs %i
		.set i, i + 1
	.endr

	mv t5, t6
	csrr t6, mscratch
	save_gp 31, t5
	csrw mscratch, t5

	# Now invoke our M-mode trap handler
	csrr a0, mepc
	csrr a1, mtval
	csrr a2, mcause
	csrr a3, mhartid
	csrr a4, mstatus
	mv a5, t5 # t5 still contains copy of mscratch
	# Make sure we use the kernel stack as our trap stack
	# instead of that of our user process
	# Best practice is probably to allocate a page for
	# dedicated use as the trap stack, but using the kernel
	# stack directly seems to work for now
	la sp, __ld_kernel_stack_end
	call m_mode_trap_handler

	# m_mode_trap_handler returns the PC value via a0
	csrw mepc, a0

	# Restore registers and return
	# This is more straightforward, since we can overwrite t6=x31 at the end
	csrr t6, mscratch
	.set i, 1
	.rept 31
		load_gp %i
		.set i, i + 1
	.endr

	.set i, 0
	.rept NUM_FPU_REGS
		load_fpu_regs %i
		.set i, i + 1
	.endr

	# Continue execution at the given PC value
	mret

.global make_syscall
make_syscall:
	ecall
	ret

# a0 - frame address
# a1 - program counter
# a2 - SATP register
.global switch_to_user
switch_to_user:

	# Set MPP=0 (U-mode), MPIE=1, SPIE=1
	li t0, (0b00 << 11) | (1 << 7) | (1 << 5)
	csrw mstatus, t0

	csrw mscratch, a0
	csrw mepc, a1
	csrw satp, a2

	# Enable external, timer and software interrupts from
	# M-mode and S-mode alike
	li t0, 0xAAA
	csrw mie, t0

	# Set interrupt handler. When an interrupt or exception occurs in
	# M-mode, control will transfer to the address specified in mtvec
	la t0, interrupt_handler
	csrw mtvec, t0

	# # When an interrupt or exception occurs in M-mode, control
	# will transfer to the address specified in mtvec
	sfence.vma

	# Define PMP region to allow (indirect) access to all
	# physical memory in U-mode
	# By default, M-mode can access all physical memory and
	# no other modes can access any physical memory
	# pmp0cfg = pmpcfg0[7:0]
	#      A=TOR         X=1        W=1        R=1
	li t0, (0b01 << 3) | (1 << 2) | (1 << 1) | (1 << 0)
	csrw pmpcfg0, t0
	# Set all 1 for the top address (exclusive)
	# The bottom address (inclusive) is implicitly 0 when setting
	# pmpcfg0 and pmpaddr0
	li t0, -1
	csrw pmpaddr0, t0

	# Load process context frame
	mv t6, a0
	.set i, 1
	.rept 31
		load_gp %i, t6
		.set i, i + 1
	.endr

	.set i, 0
	.rept NUM_FPU_REGS
		load_fpu_regs %i
		.set i, i + 1
	.endr

	# Enable FPU
	li t0, 0x6000
	csrs mstatus, t0

	# Now jump to U-mode. Execution resumes in the user process.
	mret