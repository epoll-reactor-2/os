#include "devices/fb.h"
#include "macro.h"
#include <stdint.h>

__section("fb.bitmap")
static const uint8_t logo_bitmap_argb8888[] = {
#include "logo.argb8888"
};

#define __bitmap_w	536
#define __bitmap_h	121
#define __font_size	 24

struct framebuffer {
	volatile uint8_t	*buf;
	int			w;
	int			h;
	int			depth;
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

static void render_letter(struct framebuffer *fb)
{
	/* One letter for test. See scripts/ttf_to_bitmap.c */
	const uint32_t font_bitmap[15][11] = {
		{ 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00FFFFFF, 0x00FFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
		{ 0x00000000, 0x00000000, 0x00000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00000000, 0x00000000, 0x00000000 },
		{ 0x00000000, 0x00000000, 0x00000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00000000, 0x00000000, 0x00000000 },
		{ 0x00000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00000000, 0x00000000 },
		{ 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF },
		{ 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF },
		{ 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF },
		{ 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF },
		{ 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF },
		{ 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF },
		{ 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF },
		{ 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF },
		{ 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF },
		{ 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF },
		{ 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF }
	};

	for (int y = 0; y < 15; ++y) {
		for (int x = 0; x < 11; ++x) {
			uint32_t byte = font_bitmap[y][x];

			/* This will allow to easily change background color. */
			if (byte == 0x00000000)
				continue;

			uint32_t *__fb = (uint32_t *) &fb->buf[(y * fb->w + x) * fb->depth];
			*__fb = font_bitmap[y][x];
		}
	}
}

void fb_init(int w, int h, int depth)
{
	struct framebuffer fb = {
		.buf 	= (volatile uint8_t *) __mmio_fb_addr,
		.w 	= w,
		.h 	= h,
		.depth	= depth
	};

	for (int i = 0; i < w * h * depth; i += depth) {
		volatile uint32_t *p = (volatile uint32_t *) &fb.buf[i];

		*p = 0x00101010;
	}

	// logo(&fb);
	render_letter(&fb);
}