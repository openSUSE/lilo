/* $Id$ */

#include <prom.h>

prom_entry promptr;

phandle chosen_handle;
ihandle stdin;
ihandle stdout;
ihandle stderr;

ihandle mmu;
phandle memory;
int claim_needs_map;

/* returns true if s2 is a prefix of s1 */
static int string_match(const char *s1, const char *s2)
{
	for (; *s2; ++s2)
		if (*s1++ != *s2)
			return 0;
	return 1;
}

/*
 * Older OF's require that when claiming a specific range of addresses,
 * we claim the physical space in the /memory node and the virtual
 * space in the chosen mmu node, and then do a map operation to
 * map virtual to physical.
 */
static int check_of_version(void)
{
	phandle oprom;
	char version[20];

	oprom = of1275_finddevice("/openprom");
	if (oprom == (phandle) (-1))
		return 0;
	if (of1275_getprop(oprom, "model", version, sizeof(version)) <= 0)
		return 0;
	if (!string_match(version, "Open Firmware, 1.")
	    && !string_match(version, "FirmWorks,3."))
		return 0;
	memory = (ihandle) call_prom("open", 1, 1, "/memory");
	if (memory == (ihandle) (-1)) {
		memory = (ihandle) call_prom("open", 1, 1, "/memory@0");
		if (memory == (ihandle) (-1))
			return 0;
	}
	return 1;
}

void of1275_prominit(prom_entry entry)
{
	promptr = entry;
	chosen_handle = of1275_finddevice("/chosen");
	if (chosen_handle == (phandle) (-1)) {
		chosen_handle = of1275_finddevice("/chosen@0");
		if (chosen_handle == (phandle) (-1))
			of1275_exit();
	}
	if (of1275_getprop(chosen_handle, "stdout", &stdout, sizeof(stdout)) !=
	    4)
		of1275_exit();
	stderr = stdout;
	if (of1275_getprop(chosen_handle, "stdin", &stdin, sizeof(stdin)) != 4)
		of1275_exit();
	if (!check_of_version())
		return;
	claim_needs_map = 1;
	if (of1275_getprop(chosen_handle, "mmu", &mmu, sizeof(mmu)) != 4)
		of1275_exit();
}
