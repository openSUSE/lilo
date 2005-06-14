/* $Id$ */
#include <prom.h>
ihandle of1275_open(const char *path)
{
	return call_prom("open", 1, 1, path);
}
