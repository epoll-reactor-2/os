#ifndef FB_H
#define FB_H

#define __mmio_fb_addr	0x28000000
/* TODO: Parse DTB. */
#define __fb_w		640
#define __fb_h		480
#define __fb_depth	4

/* For now framebuffer is working only
   in the kernel space. */
void fb_init(int w, int h, int depth);
void fb_put(char c);

#endif /* FB_H */