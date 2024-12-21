SRC=ttf_to_bitmap.c
BIN=$(SRC:.c=)

.PHONY: fonts
fonts:
	@echo " CC " $(SRC)
	@gcc $(SRC) -I /usr/include/freetype2/ -lfreetype -o $(BIN)
	@./ttf_to_bitmap DOS-VGA-437.ttf A 24 dos_vga_437.c

.PHONY: clean
