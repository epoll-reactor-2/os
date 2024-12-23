#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define __fixed_w 	 12
#define __fixed_h 	 20
#define __ascii_start	 32
#define __ascii_end	126

static inline int letter_off(char c)
{
	switch (c) {
	case 'g':
	case 'j':
	case 'p':
	case 'q':
	case 'y':
		return 0;
	default:
		return 5;
	}
}

void write_bitmap_as_c_array(uint32_t *bitmap, int width, int height, char c, FILE *f)
{
	fprintf(f, "    /* %c */\n", c);

	int baseline_offset = letter_off(c);
	for (int y = 0; y < __fixed_h; ++y) {
		fprintf(f, "    ");
		for (int x = 0; x < __fixed_w; ++x) {
			/* Letters have different size. So this
			   aligns them properly. */
			int32_t off = __fixed_h - height - baseline_offset;
			uint32_t letter = 1;
			letter &= y >= off;
			letter &= y <  __fixed_h - baseline_offset;
			letter &= x <  width;

			uint32_t pixel = bitmap[(y - off) * width + x];

			fprintf(f, "0x%08X, ", letter ? pixel : 0x00000000);
		}
		fprintf(f, "\n");
	}
	fprintf(f, "\n");
}

int main(int argc, char **argv)
{
	if (argc != 5) {
		printf("Usage: %s <font file> <character> <size> <output file>\n", argv[0]);
		return 1;
	}

	const char *font_file = argv[1];
	char character = argv[2][0];
	int size = atoi(argv[3]);
	const char *output_name = argv[4];
	char output_file[128] = {0};

	sprintf(output_file, "%s.c", output_name);

	FILE *f = fopen(output_file, "w");
	if (!f) {
		perror("Failed to open output file");
		return 1;
	}
	fprintf(f, "#include \"%s.h\"\n\n", output_name);
	fprintf(f, "\n");
	fprintf(f, "const uint32_t %s[%d * %d * %d] = {\n",
		output_name, __ascii_end - __ascii_start, __fixed_h, __fixed_w
	);

	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		fprintf(stderr, "Could not initialize FreeType library\n");
		return 1;
	}

	for (int i = __ascii_start; i <= __ascii_end; ++i) {
		FT_Face face;
		if (FT_New_Face(ft, font_file, 0, &face)) {
			fprintf(stderr, "Could not load font: %s\n", font_file);
			FT_Done_FreeType(ft);
			return 1;
		}

		FT_Set_Pixel_Sizes(face, 0, size);

		if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
			fprintf(stderr, "Could not load character: %c\n", character);
			FT_Done_Face(face);
			FT_Done_FreeType(ft);
			return 1;
		}

		FT_GlyphSlot g = face->glyph;
		int width = g->bitmap.width;
		int height = g->bitmap.rows;

		uint32_t *bitmap = (uint32_t *) malloc(width * height * sizeof(uint32_t));
		if (!bitmap) {
			perror("Failed to allocate bitmap");
			FT_Done_Face(face);
			FT_Done_FreeType(ft);
			return 1;
		}

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				unsigned char value = g->bitmap.buffer[y * width + x];
				bitmap[y * width + x] = value == 0 ? 0 : 0x00FFFFFF;
			}
		}

		write_bitmap_as_c_array(bitmap, width, height, i, f);

		free(bitmap);
		FT_Done_Face(face);
	}

	fprintf(f, "};\n");

	sprintf(output_file, "%s.h", output_name);

	f = fopen(output_file, "w");
	if (!f) {
		perror("Failed to open output file");
		return 1;
	}

	fprintf(f, "#ifndef __%s\n", output_name);
	fprintf(f, "#define __%s\n", output_name);
	fprintf(f, "\n");
	fprintf(f, "#include <stdint.h>\n");
	fprintf(f, "\n");
	fprintf(f, "#define __%s_start %d\n", output_name, __ascii_start);
	fprintf(f, "#define __%s_end   %d\n", output_name, __ascii_end);
	fprintf(f, "#define __%s_w     %d\n", output_name, __fixed_w);
	fprintf(f, "#define __%s_h     %d\n", output_name, __fixed_h);
	fprintf(f, "\n");
	fprintf(f, "extern const uint32_t %s[%d * %d * %d];\n",
		output_name, __ascii_end - __ascii_start, __fixed_h, __fixed_w
	);
	fprintf(f, "\n");
	fprintf(f, "#endif /* __%s */\n", output_name);

	FT_Done_FreeType(ft);
}