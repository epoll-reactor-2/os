#ifndef KENREL_CONFIG_H
#define KENREL_CONFIG_H

/* Kernel configuration file. */

enum {
	/* How many columns are available in terminal. */
	CONFIG_VGA_WIDTH		=     80,
	/* How many rows are available in terminal. */
	CONFIG_VGA_HEIGHT		=     25,
	/* Size of buffer for terminal output. Needed for scroll. */
	CONFIG_VGA_BUFFER_LEN		=   8192,
};

#endif /* KENREL_CONFIG_H */