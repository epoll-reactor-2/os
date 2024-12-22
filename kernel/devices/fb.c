#include "devices/fb.h"
#include "libc/string.h"
#include "macro.h"
#include <stdint.h>
#include "font_dos_vga_437.h"

__section("fb.bitmap")
static const uint8_t logo_bitmap_argb8888[] = {
#include "logo.argb8888"
};

#define __bitmap_w	536
#define __bitmap_h	121
#define __font_size	 24
#define __letter_off	 10

struct framebuffer {
	volatile uint8_t	*buf;
	int			w;
	int			h;
	int			depth;
};

static struct framebuffer fb = {
	.buf 	= (volatile uint8_t *) __mmio_fb_addr,
};

static void logo(struct framebuffer *fb)
{
	for (int y = 0; y < __bitmap_h; ++y)
		for (int x = 0; x < __bitmap_w; ++x)
			for (int d = 0; d < fb->depth; ++d) {

				int fb_pos = (y * fb->w + x) * fb->depth + d;
				int logo_pos = (y * __bitmap_w + x) * fb->depth + d;

				fb->buf[fb_pos] = logo_bitmap_argb8888[logo_pos];
			}
}

/* Slow as fuck. */
static inline void rotate()
{
	size_t line_size = fb.w * __font_dos_vga_437_h * fb.depth;

	for (size_t i = 0; i < (fb.h - __font_dos_vga_437_h) * fb.w * fb.depth; ++i)
		fb.buf[i] = fb.buf[i + line_size];

	size_t last_line = (fb.h - __font_dos_vga_437_h) * fb.w * fb.depth;

	for (size_t i = last_line; i < fb.h * fb.w * fb.depth; ++i)
		fb.buf[i] = 0x00000000;
}

static inline void render_letter(char letter)
{
	static int x_off = 0;
	static int y_off = 0;

	for (int y = 0; y < __font_dos_vga_437_h; ++y) {
		for (int x = 0; x < __font_dos_vga_437_w; ++x) {
			uint32_t siz   = __font_dos_vga_437_w * __font_dos_vga_437_h;
			uint32_t start = siz * (letter - __font_dos_vga_437_start);
			uint32_t byte  = font_dos_vga_437[start + (y * __font_dos_vga_437_w + x)];

			/* This will allow to easily change background color. */
			if (byte == 0x00000000)
				continue;

			uint32_t off = x_off + ((y + y_off) * fb.w + x) * fb.depth;

			uint32_t *__fb = (uint32_t *) &fb.buf[off];
			*__fb = byte;
		}
	}

	x_off += __font_dos_vga_437_w + (__letter_off * fb.depth);

	if (letter == '\n') {
		if (y_off <= (fb.h - (__font_dos_vga_437_h * 2)))
			y_off += __font_size;
		else
			rotate();
		x_off = 0;
	}
}

void fb_put(char c)
{
	render_letter(c);
}

void fb_init(int w, int h, int depth)
{
	fb.buf = (volatile uint8_t *) __mmio_fb_addr;
	fb.w = w;
	fb.h = h;
	fb.depth = depth;

	for (int i = 0; i < w * h * depth; i += depth) {
		volatile uint32_t *p = (volatile uint32_t *) &fb.buf[i];

		*p = 0x00000000;
	}
}