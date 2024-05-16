#ifndef KERNEL_CMD_H
#define KERNEL_CMD_H

struct command {
	void	(*routine)();
	char	cmd[64];
	char	desc[64];
};

/* TODO: We need to manually align size of this
         array if set of commands will change.
         Buggy. */
extern struct command cmds[5];

/* Public command. Called manually. */
void cmd_die();
void cmd_ata_reset();
void cmd_fs_init();
void cmd_clear();
void cmd_help();

#endif /* KERNEL_CMD_H */