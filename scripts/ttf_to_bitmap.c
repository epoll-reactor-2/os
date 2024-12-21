#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void write_bitmap_as_c_array(uint32_t *bitmap, int width, int height, const char *output_filename)
{
	FILE *f = fopen(output_filename, "w");
	if (!f) {
		perror("Failed to open output file");
		return;
	}

	fprintf(f, "const uint32_t font_bitmap[%d][%d] = {\n", height, width);
	for (int y = 0; y < height; ++y) {
		fprintf(f, "    {");
		for (int x = 0; x < width; ++x) {
			fprintf(f, "0x%08X%s", bitmap[y * width + x], (x < width - 1) ? ", " : "");
		}
		fprintf(f, "}%s\n", (y < height - 1) ? "," : "");
	}

	fprintf(f, "};\n");
	fclose(f);
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
	const char *output_file = argv[4];

	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		fprintf(stderr, "Could not initialize FreeType library\n");
		return 1;
	}

	FT_Face face;
	if (FT_New_Face(ft, font_file, 0, &face)) {
		fprintf(stderr, "Could not load font: %s\n", font_file);
		FT_Done_FreeType(ft);
		return 1;
	}

	FT_Set_Pixel_Sizes(face, 0, size);

	if (FT_Load_Char(face, character, FT_LOAD_RENDER)) {
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

	write_bitmap_as_c_array(bitmap, width, height, output_file);

	free(bitmap);
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}