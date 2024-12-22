#include "devices/fb.h"
#include "devices/uart.h"
#include "mm/page.h"
#include "mm/sv39.h"
#include "mm/kmem.h"
#include "plic/trap_frame.h"
#include "plic/cpu.h"
#include "plic/plic.h"
#include "printk/printk.h"
#include "process/process.h"
#include "process/sched.h"
#include "syscon/syscon.h"
#include "config.h"
#include "macro.h"

// Identity map range
// Takes a contiguous allocation of memory and maps it using __page_size
// `start` must not exceed `end`
static void id_map_range(struct page_table *root, size_t start, size_t end,
		  uint64_t bits)
{
	__assert(root != NULL, "id_map_range(): root page table cannot be NULL");
	__assert(start <= end, "id_map_range(): start must not exceed end");
	__assert(__pte_is_leaf(bits),
		"id_map_range(): Provided bits must correspond to leaf entry");

	size_t memaddr = start & ~(__page_size - 1);
	size_t num_kb_pages = (align_val(end, __page_order) - memaddr) / __page_size;

	for (size_t i = 0; i < num_kb_pages; ++i) {
		map(root, memaddr, memaddr, bits, 0);
		memaddr += __page_size;
	}
}

static void printk_intro()
{
	printk("          ....                                                                s       .                              \n");
	printk("      .x~X88888Hx.                                                           :8      @88>                            \n");
	printk("     H8X 888888888h.    .d``                       .u    .                  .88      %8P      u.    u.               \n");
	printk("    8888:`*888888888:   @8Ne.   .u        .u     .d88B :@8c        u       :888ooo    .     x@88k u@88c.      uL     \n");
	printk("    88888:        `%8   %8888:u@88N    ud8888.  ='8888f8888r    us888u.  -*8888888  .@88u  ^'8888''8888'  .ue888Nc.. \n");
	printk("  . `88888          ?>   `888I  888. :888'8888.   4888>'88'  .@88 '8888'   8888    ''888E`   8888  888R  d88E`'888E` \n");
	printk("  `. ?888%           X    888I  888I d888 '88%'   4888> '    9888  9888    8888      888E    8888  888R  888E  888E  \n");
	printk("    ~*??.            >    888I  888I 8888.+'      4888>      9888  9888    8888      888E    8888  888R  888E  888E  \n");
	printk("   .x88888h.        <   uW888L  888' 8888L       .d888L .+   9888  9888   .8888Lu=   888E    8888  888R  888E  888E  \n");
	printk("  :'''8888888x..  .x   '*88888Nu88P  '8888c. .+  ^'8888*'    9888  9888   ^%888*     888&   '*88*' 8888' 888& .888E  \n");
	printk("  `    `*888888888'    ~ '88888F`     '88888%       'Y'      '888*''888'    'Y'      R888'    ''   'Y'   *888' 888&  \n");
	printk("          ''***''         888 ^         'YP'                  ^Y'   ^Y'               ""                  `'   '888E \n");
	printk("                          *8E                                                                            .dWi   `88E \n");
	printk("                          '8>                                                                            4888~  J8%  \n");
	printk("                           '                                                                              ^'===*'`   \n");
	printk("                      ...                         .x+=:.        s                                                    \n");
	printk("                  .x888888hx    :   ..           z`    ^%      :8                                                    \n");
	printk("                 d88888888888hxx   @L               .   <k    .88                  ..    .     :                     \n");
	printk("                8' ... `'*8888%`  9888i   .dL     .@8Ned8'   :888ooo      .u     .888: x888  x888.                   \n");
	printk("               !  '   ` .xnxx.    `Y888k:*888.  .@^%8888'  -*8888888   ud8888.  ~`8888~'888X`?888f`                  \n");
	printk("                X   .H8888888%:    888E  888I x88:  `)8b.   8888    :888'8888.   X888  888X '888>                    \n");
	printk("                'hn8888888*'   >   888E  888I 8888N=*8888   8888    d888 '88%'   X888  888X '888>                    \n");
	printk("               : `*88888%`     !   888E  888I  %8'    R88   8888    8888.+'      X888  888X '888>                    \n");
	printk("               '8h.. ``     ..x8>   888E  888I   @8Wou 9%   .8888Lu= 8888L        X888  888X '888>                   \n");
	printk("                `88888888888888f   x888N><888' .888888P`    ^%888*   '8888c. .+  '*88%''*88' '888!`                  \n");
	printk("                 '%8888888888*'     '88'  888  `   ^'F        'Y'     '88888%      `~    '    `.`                    \n");
	printk("                    ^'****''`             88F                           'YP'                                         \n");
	printk("                                         98'                                                                         \n");
	printk("                                       ./'                                                                           \n");
	printk("                                      ~`                                                                             \n");
}

void kernel_main(void)
{
	uart_init();
	page_init();
	kmem_init();

	printk_intro();

#if CONFIG_FPU
	printk("FPU test: %f\n", 1.23 / 0.00000000000000001);
#endif /* CONFIG_FPU */

	__plic_set_threshold(0);
	__plic_enable(__plic_uart);
	__plic_set_prio(__plic_uart, 1);

#if CONFIG_FB
	fb_init(__fb_w, __fb_h, __fb_depth);
#endif /* CONFIG_FB */
	printk("Initializing the process scheduler ...\n");
	sched_init();


	kmalloc(123);
	kmem_print_table();
	print_page_allocations();

	printk("Adding few processes to scheduler ...\n");
	sched_enqueue(process_init);
	sched_enqueue(process_init);
	sched_enqueue(process_init);
	sched_enqueue(process_init);
	sched_enqueue(process_init);
	sched_enqueue(process_init);

	sched_print_ptree();

	printk("Starting our first process ...\n");
	struct process *process = sched_schedule();
	__assert(process != NULL,
		"kmain(): process structure returned from scheduler was unexpectedly NULL\n");

	printk("Our first process has PID = %d\n", process->pid);

	printk("Issuing our first context switch timer ...\n");
	set_timer_interrupt_delay_us(1 * __us_per_second);
	printk("Timer done\n");

	process_switch_to_user(process);

	__panic("kmain(): failed to start our first process!\n");
}
