all: alloc alloc_run

alloc: alloc.c
	@ echo " CC " $^
	@ gcc -O0 -ggdb $^ -o $@

alloc_run: alloc
	@ ./$^
