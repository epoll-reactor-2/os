SRC=ttf_to_bitmap.c
BIN=$(SRC:.c=)

BUILD_SRC=../$(BUILD_DIR)/src

.PHONY: fonts
fonts:
	@ echo " CC " $(SRC)
	@ gcc -O0 -ggdb $(SRC) -I /usr/include/freetype2/ -lfreetype -o $(BIN)
	@ ./ttf_to_bitmap DOS-VGA-437.ttf A 24 font_dos_vga_437
	@ mkdir -p $(BUILD_SRC)
	@ cp font_*.h font_*.c $(BUILD_SRC)

.PHONY: clean
