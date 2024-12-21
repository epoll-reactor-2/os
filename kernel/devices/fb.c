#include "devices/fb.h"
#include "macro.h"
#include <stdint.h>

__section("fb.bitmap")
static const uint8_t logo_bitmap_argb8888[] = {
#include "logo.argb8888"
};

#define __bitmap_w	536
#define __bitmap_h	121

static void logo(volatile uint8_t *fb, int w, int h, int depth)
{
	for (int y = 0; y < __bitmap_h; ++y)
		for (int x = 0; x < __bitmap_w; ++x)
			for (int d = 0; d < depth; ++d)
				fb[(y * w + x) * depth + d] = logo_bitmap_argb8888[(y * __bitmap_w + x) * depth + d];
}

void fb_init(int w, int h, int depth)
{
	volatile uint8_t *fb = (volatile uint8_t *) __mmio_fb_addr;

	for (int i = 0; i < w * h * depth; i += depth) {
		volatile uint32_t *p = (volatile uint32_t *) &fb[i];

		*p = 0x00101010;
	}

	logo(fb, w, h, depth);
}