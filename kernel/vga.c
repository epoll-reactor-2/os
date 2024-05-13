#include "config.h"
#include "vga.h"
#include "lib/stdlib.h"

#define VGA_W CONFIG_VGA_WIDTH
#define VGA_H CONFIG_VGA_HEIGHT

static uint64_t  vga_row;
static uint64_t  vga_col;
static uint8_t   vga_color;
static uint64_t  vga_default_entry;
static uint16_t *vga_mem;

/* Extra video buffer. Now aligned with VGA screen size.
   To implement scroll, we should enlarge this buffer and
   extend `vga_rotate_buf()` to specified position rotation:

   vga_rotate_buf(/ * to line= * / 0)
   vga_rotate_buf(/ * to line= * / 10)
   vga_rotate_buf(/ * to line= * / 15) */
static uint16_t  vga_buf[VGA_W * VGA_H];

static inline uint64_t vga_at(uint64_t x, uint64_t y)
{
	return y * VGA_W + x;
}

static void vga_rotate_buf()
{
	/* Rotation. */
	for (uint64_t y = 1; y < VGA_H; ++y) {
		for (uint64_t x = 0; x < VGA_W; ++x) {
			vga_buf[vga_at(x, y - 1)] = vga_buf[vga_at(x, y)];
			vga_mem[vga_at(x, y - 1)] = vga_mem[vga_at(x, y)];
		}
	}

	/* Cleanup of last row. */
	for (uint64_t x = 0; x < VGA_W; ++x) {
		vga_buf[vga_at(x, VGA_H - 1)] = vga_default_entry;
		vga_mem[vga_at(x, VGA_H - 1)] = vga_default_entry;
	}
}

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}

static inline uint16_t vga_entry(uint8_t uc, uint8_t color)
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

static void vga_put_at(char c, uint8_t color, uint64_t x, uint64_t y)
{
	const uint64_t index = y * VGA_W + x;
	vga_buf[index] = vga_entry(c, color);
	vga_mem[index] = vga_entry(c, color);
}

void vga_putchar(char c)
{
	vga_put_at(c, vga_color, vga_col, vga_row);

	if (c == '\n') {
		vga_col = 0;
		if (vga_row < VGA_H - 1)
			vga_row++;
		else
			/* In this place vga_row is always points
			   to most bottom line. Needed to add more logic
			   to implement scroll. */
			vga_rotate_buf();
	} else
		++vga_col;
}

void vga_write(const char *data, uint64_t size)
{
	for (uint64_t i = 0; i < size; i++)
		vga_putchar(data[i]);
}

static inline void __vga_init(uint8_t text, uint8_t bg, char fill)
{
	vga_row = 0;
	vga_col = 0;
	vga_color = vga_entry_color(text, bg);

	// Wiki: The VGA text buffer is located at physical memory address 0xB8000.
	vga_mem = (uint16_t *) VGA_PHYS_VIDEO_ADDR;
	vga_default_entry = vga_entry(fill, vga_color);

	for (uint64_t y = 0; y < VGA_H; y++) {
		for (uint64_t x = 0; x < VGA_W; x++) {
			uint64_t idx = vga_at(x, y);
			vga_buf[idx] = vga_default_entry;
			vga_mem[idx] = vga_default_entry;
		}
	}
}

void vga_init()
{
	__vga_init(VGA_COLOR_WHITE, VGA_COLOR_BLUE, ' ');
}