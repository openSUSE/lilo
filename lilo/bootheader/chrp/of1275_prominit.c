/* $Id$ */

#include <prom.h>

prom_entry promptr;

phandle chosen_handle;
ihandle stdin;
ihandle stdout;
ihandle stderr;

ihandle mmu;

void of1275_prominit(prom_entry entry)
{
	promptr = entry;
	chosen_handle = of1275_finddevice("/chosen");
	if (chosen_handle == (phandle) (-1))
		of1275_exit();
	if (of1275_getprop(chosen_handle, "stdout", &stdout, sizeof(stdout)) !=
	    4)
		of1275_exit();
	stderr = stdout;
	if (of1275_getprop(chosen_handle, "stdin", &stdin, sizeof(stdin)) != 4)
		of1275_exit();
}
