#include "devices/fb.h"
#include <stdint.h>

const uint8_t logo_bitmap_argb8888[] = {
#include "logo.argb8888"
};

static void logo(volatile uint8_t *fb, int w, int h, int depth)
{
	for (int y = 0; y < 121; ++y) {
		for (int x = 0; x < 536; ++x) {
			for (int d = 0; d < depth; ++d)
				fb[(y * w + x) * depth + d] = logo_bitmap_argb8888[(y * 536 + x) * depth + d];
		}
	}
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