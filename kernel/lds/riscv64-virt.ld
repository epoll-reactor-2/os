/* Entry symbol is _start */
ENTRY(_start)

/* RAM starts at 0x80000000 - see misc/riscv64-virt.dts for details */
. = 0x80000000;

SECTIONS {
        /**
        * ELF file layout: text section at bottom, followed by data and bss
        * sections
        *
        * Global pointer starts where BSS ends
        * The heap also starts right after the BSS section, growing upwards
        * The stack grows downwards, starting from the end of RAM
        * This mean there's a chance stack and heap collide, but we'll assume
        * it never happens
        */
        .init : ALIGN(4K) {
                PROVIDE(__ld_init_start = .);
                *(.init);
                PROVIDE(__ld_init_end = .);
        }

        .text : ALIGN(4K) {
                PROVIDE(__ld_text_start = .);
                *(.text);
                PROVIDE(__ld_text_end = .);
        }

        .rodata : ALIGN(4K) {
                PROVIDE(__ld_rodata_start = .);
                *(.rodata);
                PROVIDE(__ld_rodata_end = .);
        }

        .data : ALIGN(4K) {
                PROVIDE(__ld_data_start = .);
                *(.data);
                PROVIDE(__ld_data_end = .);
        }

        .bss : ALIGN(4K) {
                PROVIDE(__ld_bss_start = .);
                *(.bss);
                PROVIDE(__ld_bss_end = .);
        }

        PROVIDE(__ld_global_pointer = .);
        PROVIDE(__ld_heap_start = .);
        . = 0x88000000;
        PROVIDE(__ld_kernel_stack_start = . - 0x80000);
        PROVIDE(__ld_kernel_stack_end = .);
        PROVIDE(__ld_heap_size = __ld_kernel_stack_start - __ld_heap_start);
}
