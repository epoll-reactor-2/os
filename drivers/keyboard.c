#include "keyboard.h"
#include "vga.h"
#include "cpu/ports.h"
#include "cpu/isr.h"
#include "cpu/timer.h"
#include "lib/string.h"
#include "kernel/compiler.h"
#include "kernel/kernel.h"

const char *sc_name[] = {
	"ERROR", "Esc", "1", "2", "3", "4", "5", "6", 
	"7", "8", "9", "0", "-", "=", "Backspace", "Tab", "Q", "W", "E", 
	"R", "T", "Y", "U", "I", "O", "P", "[", "]", "Enter", "Lctrl", 
	"A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "`", 
	"LShift", "\\", "Z", "X", "C", "V", "B", "N", "M", ",", ".", 
	"/", "RShift", "Keypad *", "LAlt", "Spacebar"
};

const char sc_ascii[] = {
	'?', '?', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '?', '?',
	'Q', 'W', 'E', 'R', 'T', 'Y',  'U', 'I', 'O', 'P', '[', ']',
	'?', '?', 'A', 'S', 'D', 'F', 'G',  'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z', 'X', 'C', 'V', 
	'B', 'N', 'M', ',', '.', '/', '?', '?', '?', ' '
};

static char key_buffer[256];

static void keyboard_callback(__unused struct registers regs)
{
	/* The PIC leaves us the scancode in port 0x60 */
	u8 scancode = port_byte_in(0x60);

	if (scancode > SC_MAX) return;

	if (scancode == BACKSPACE) {
		backspace(key_buffer);
		kprint_backspace();
	} else if (scancode == ENTER) {
		kprint("\n");
		user_input(key_buffer);
		key_buffer[0] = '\0';
	} else {
		char letter = sc_ascii[(int)scancode];
		/* Remember that kprint only accepts char[] */
		char str[2] = {letter, '\0'};
		append(key_buffer, letter);
		kprint(str);
	}
}

void keyboard_init()
{
	register_interrupt_handler(IRQ1, keyboard_callback); 
}
