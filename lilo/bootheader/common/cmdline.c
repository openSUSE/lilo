/* $Id$ */
#include <cmdline.h>

struct _builtin_cmd_line _builtin_cmd_line = {
	.prefer = '0',
	.cmdling_start_flag = cmdline_start_string,
	.string = "",
	.cmdline_end_flag = cmdline_end_string,
};

