a small tool to modify a built-in cmdline
it can change a cmd_line string, the content will override whatever is 
passed from OF or the bootloader.
it can disable/enable the string
the purpose of this tool is a simplified netboot.
SMS can not pass boot options to the bootfile, they must be
stored in the 'boot-file' OF property.
This tool allows you to just boot the zImage and modify the 
kernel command line on the netboot server.
olh@suse.de
