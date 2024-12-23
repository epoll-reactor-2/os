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
#define __tab_w		  8
#define __bg		0x00000000

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

static inline void rotate()
{
	uint64_t    line_w =  fb.w * __font_dos_vga_437_h * fb.depth;
	uint64_t     total = (fb.h - __font_dos_vga_437_h) * fb.w * fb.depth;
	uint64_t   aligned = total & ~0x63;
	uint64_t         i = 0;

	for (; i < aligned; i += 64) {
		*(uint64_t *) (fb.buf + i + (8 *  0)) = *(uint64_t *) (fb.buf + i + (8 *  0) + line_w);
		*(uint64_t *) (fb.buf + i + (8 *  1)) = *(uint64_t *) (fb.buf + i + (8 *  1) + line_w);
		*(uint64_t *) (fb.buf + i + (8 *  2)) = *(uint64_t *) (fb.buf + i + (8 *  2) + line_w);
		*(uint64_t *) (fb.buf + i + (8 *  3)) = *(uint64_t *) (fb.buf + i + (8 *  3) + line_w);
		*(uint64_t *) (fb.buf + i + (8 *  4)) = *(uint64_t *) (fb.buf + i + (8 *  4) + line_w);
		*(uint64_t *) (fb.buf + i + (8 *  5)) = *(uint64_t *) (fb.buf + i + (8 *  5) + line_w);
		*(uint64_t *) (fb.buf + i + (8 *  6)) = *(uint64_t *) (fb.buf + i + (8 *  6) + line_w);
		*(uint64_t *) (fb.buf + i + (8 *  7)) = *(uint64_t *) (fb.buf + i + (8 *  7) + line_w);
	}

	for (; i < total; ++i)
		fb.buf[i] = fb.buf[i + line_w];

	size_t last_line = (fb.h - __font_dos_vga_437_h) * fb.w * fb.depth;

	for (size_t i = last_line; i < fb.h * fb.w * fb.depth; ++i)
		fb.buf[i] = __bg;
}

static inline void render_visible(int start, int *x_off, int *y_off)
{
	int letter_off = __font_dos_vga_437_w + (__letter_off * fb.depth);

	for (int y = 0; y < __font_dos_vga_437_h; ++y) {
		for (int x = 0; x < __font_dos_vga_437_w; ++x) {
			uint32_t byte = font_dos_vga_437[start + (y * __font_dos_vga_437_w + x)];

			/* This will allow to easily change background color. */
			if (byte == __bg)
				continue;

			uint32_t off = *x_off + ((y + *y_off) * fb.w + x) * fb.depth;

			uint32_t *__fb = (uint32_t *) &fb.buf[off];
			*__fb = byte;

		}
	}

	*x_off += letter_off;
}

static void render_letter(char letter)
{
	int siz = __font_dos_vga_437_w * __font_dos_vga_437_h;

	static int x_off = 0;
	static int y_off = 0;

	switch (letter) {
	case '\n':
		if (y_off <= (fb.h - (__font_dos_vga_437_h * 2)))
			y_off += __font_size;
		else
			rotate();
		x_off = 0;
		break;
	case '\t': {
		int letter_off = __font_dos_vga_437_w + (__letter_off * fb.depth);
		/* Tab consists of spaces. */
		int start = siz * (' ' - __font_dos_vga_437_start);

		for (int i = 0; i < (x_off / letter_off) % __tab_w; ++i)
			render_visible(start, &x_off, &y_off);
		break;
	}
	case '\r':
		x_off = 0;
		break;
	default: {
		int start = siz * (letter - __font_dos_vga_437_start);
		render_visible(start, &x_off, &y_off);
		break;
	}
	} /* switch */
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

		*p = __bg;
	}
}