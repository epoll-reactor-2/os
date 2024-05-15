#include "vga.h"
#include "cpu/ports.h"
#include "cpu/type.h"
#include "lib/mem.h"

#define vga_color(fg, bg) (fg | bg << 4)

/* Hardware text mode color constants. */
enum vga_color {
	VGA_COLOR_BLACK		= 0,
	VGA_COLOR_BLUE		= 1,
	VGA_COLOR_GREEN		= 2,
	VGA_COLOR_CYAN		= 3,
	VGA_COLOR_RED		= 4,
	VGA_COLOR_MAGENTA	= 5,
	VGA_COLOR_BROWN		= 6,
	VGA_COLOR_LIGHT_GREY	= 7,
	VGA_COLOR_DARK_GREY	= 8,
	VGA_COLOR_LIGHT_BLUE	= 9,
	VGA_COLOR_LIGHT_GREEN	= 10,
	VGA_COLOR_LIGHT_CYAN	= 11,
	VGA_COLOR_LIGHT_RED	= 12,
	VGA_COLOR_LIGHT_MAGENTA	= 13,
	VGA_COLOR_LIGHT_BROWN	= 14,
	VGA_COLOR_WHITE		= 15,
	VGA_COLOR_DEFAULT	= vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK)
};

enum vga_phys {
	VGA_PHYS_VIDEO_ADDR	= 0xB8000,
	VGA_H			= 25,
	VGA_W			= 80
};

/* Screen i/o ports */
#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

static s32 vga_cursor_off()
{
	/* Use the VGA ports to get the current cursor position
	   1. Ask for high byte of the cursor off (data 14)
	   2. Ask for low byte (data 15) */
	port_byte_out(REG_SCREEN_CTRL, 14);
	s32 off = port_byte_in(REG_SCREEN_DATA) << 8; /* High byte: << 8 */
	port_byte_out(REG_SCREEN_CTRL, 15);
	off += port_byte_in(REG_SCREEN_DATA);
	return off * 2; /* Position * size of character cell */
}

static void vga_set_cursor_off(s32 off)
{
	/* Similar to vga_cursor_off, but instead of reading we write data */
	off /= 2;
	port_byte_out(REG_SCREEN_CTRL, 14);
	port_byte_out(REG_SCREEN_DATA, (u8) (off >> 0x08));
	port_byte_out(REG_SCREEN_CTRL, 15);
	port_byte_out(REG_SCREEN_DATA, (u8) (off & 0xFF));
}

static s32 vga_off(s32 col, s32 row)
{
	return 2 * (row * VGA_W + col);
}

static s32 vga_off_row(s32 off)
{
	return off / (2 * VGA_W);
}

static s32 vga_off_col(s32 off)
{
	return (off - (vga_off_row(off) * 2 * VGA_W)) / 2;
}

/* Declaration of private functions */
/* Innermost print function for our kernel, directly accesses the video memory

   If 'col' and 'row' are negative, we will print at current cursor location
   If 'attr' is zero it will use 'white on black' as default
   Returns the off of the next character
   Sets the video cursor to the returned off */
static s32 vga_put_char(char c, s32 col, s32 row, char attr)
{
	u8 *video_memory = (u8*) VGA_PHYS_VIDEO_ADDR;

	s32 off;
	if (col >= 0 && row >= 0)
		off = vga_off(col, row);
	else
		off = vga_cursor_off();

	switch (c) {
	case '\n':
		row = vga_off_row(off);
		off = vga_off(0, row + 1);
		break;
	case '\b':
		video_memory[off] = ' ';
		video_memory[off + 1] = attr;
		break;
	default:
		video_memory[off] = c;
		video_memory[off + 1] = attr;
		off += 2;
		break;
	}

	/* Check if the off is over screen size and scroll */
	if (off >= VGA_H * VGA_W * 2) {
		for (s32 i = 1; i < VGA_H; i++)
			memcpy(
				(u8*) (vga_off(0, i    ) + VGA_PHYS_VIDEO_ADDR),
				(u8*) (vga_off(0, i - 1) + VGA_PHYS_VIDEO_ADDR),
				VGA_W * 2
			);

		/* Blank last line */
		char *last_line = (char*) (vga_off(0, VGA_H - 1) + (u8*) VGA_PHYS_VIDEO_ADDR);
		for (s32 i = 0; i < VGA_W * 2; i++)
			last_line[i] = 0;

		off -= 2 * VGA_W;
	}

	vga_set_cursor_off(off);
	return off;
}

/* Print a message on the specified location
   If col, row, are negative, we will use the current off */
void kprint_at(const char *message, s32 col, s32 row)
{
	/* Set cursor if col/row are negative */
	s32 off;
	if (col >= 0 && row >= 0)
		off = vga_off(col, row);
	else {
		off = vga_cursor_off();
		row = vga_off_row(off);
		col = vga_off_col(off);
	}

	while (*message) {
		off = vga_put_char(*message++, col, row, VGA_COLOR_DEFAULT);
		row = vga_off_row(off);
		col = vga_off_col(off);
	}
}

void kprint(const char *message)
{
	kprint_at(message, -1, -1);
}

void kprint_backspace()
{
	s32 off = vga_cursor_off() - 2;
	s32 row = vga_off_row(off);
	s32 col = vga_off_col(off);
	vga_put_char('\b', col, row, VGA_COLOR_DEFAULT);
}

void vga_init()
{
	s32 screen_size = VGA_W * VGA_H;
	s32 i;
	u8 *screen = (u8*) VGA_PHYS_VIDEO_ADDR;

	for (i = 0; i < screen_size; i++) {
		screen[i * 2] = ' ';
		screen[i * 2 + 1] = VGA_COLOR_DEFAULT;
	}
	vga_set_cursor_off(vga_off(0, 0));
}