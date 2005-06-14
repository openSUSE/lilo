/* $Id$ */
#include <stdarg.h>
#include <prom.h>

int call_prom(const char *service, int nargs, int nret, ...)
{
	va_list list;
	int i;
	struct prom_args prom_args;

	prom_args.service = service;
	prom_args.nargs = nargs;
	prom_args.nret = nret;
	va_start(list, nret);
	for (i = 0; i < nargs; ++i)
		prom_args.args[i] = va_arg(list, prom_arg_t);
	va_end(list);
	for (i = 0; i < nret; ++i)
		prom_args.args[i + nargs] = 0;
	prom(&prom_args);
	return prom_args.args[nargs];
}
