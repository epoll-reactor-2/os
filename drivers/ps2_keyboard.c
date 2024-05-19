#include "drivers/ps2_keyboard.h"
#include "cpu/ports.h"
#include "cpu/isr.h"
#include "cpu/timer.h"
#include "lib/stdio.h"
#include "lib/string.h"
#include "kernel/compiler.h"
#include "kernel/kernel.h"

enum {
	KEY_BACKSPACE	= 0x0E,
	KEY_ENTER	= 0x1C
};

const char *sc_name[] = {
	"ERROR",
	"Esc",   "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "Backspace",
	"Tab",   "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Enter",
	"Lctrl", "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "`", "LShift",
	"\\",    "Z", "X", "C", "V", "B", "N", "M", ",", ".",  "/",          "RShift",
	"Keypad *",
	"LAlt",
	"Spacebar"
};

const char sc_ascii[] = {
	'?', '?',  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-',  '=',
	'?', '?',  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[',  ']',
	'?', '?',  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',
	'?', '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/',
	'?', '?', '?', ' '
};


static void irq_keyboard(__unused struct registers regs)
{
	u8 code = port_byte_in(PS2_PORT);
	static char key_buf[256];

	if (code >= sizeof (sc_ascii))
		return;

	switch (code) {
	case KEY_BACKSPACE:
		backspace(key_buf);
		kprint("\b");
		break;

	case KEY_ENTER:
		/* TODO: Should this non-driver function be placed here?
		         Maybe, invent some interface to report events. */
		kprint("\n");
		user_input(key_buf);
		*key_buf = '\0';
		break;

	default: {
		char sym = sc_ascii[(int) code];
		char buf[2] = { sym, '\0' };
		append(key_buf, sym);
		kprint(buf);
		break;
	}
	} /* switch */
}

void keyboard_install()
{
	irq_install_handler(IRQ1, irq_keyboard);
}
