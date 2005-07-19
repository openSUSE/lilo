#ifndef _PPC_BOOT_CMDLINE_H_
#define _PPC_BOOT_CMDLINE_H_
/* $Id$ */

#define cmdline_start_string   "cmd_line_start"
#define cmdline_end_string     "cmd_line_end"
struct _builtin_cmd_line {
	unsigned char prefer;
	unsigned char cmdling_start_flag[sizeof(cmdline_start_string) - 1];	/* without trailing zero */
	unsigned char string[512];	/* COMMAND_LINE_SIZE */
	unsigned char cmdline_end_flag[sizeof(cmdline_end_string)];	/* with trailing zero */
} __attribute__ ((__packed__));

extern struct _builtin_cmd_line _builtin_cmd_line;
extern int get_cmdline(char *p, int len, int max);
extern void print_keys(void);
#endif				/* _PPC_BOOT_CMDLINE_H_ */
