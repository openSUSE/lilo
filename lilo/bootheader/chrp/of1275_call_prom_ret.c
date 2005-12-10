/* $Id$ */
#include <stdarg.h>
#include <prom.h>

int call_prom_ret(const char *service, int nargs, int nret,
		  unsigned int *rets, ...)
{
	va_list list;
	int i;
	struct prom_args prom_args;

	prom_args.service = service;
	prom_args.nargs = nargs;
	prom_args.nret = nret;

	va_start(list, rets);
	for (i = 0; i < nargs; i++)
		prom_args.args[i] = va_arg(list, unsigned int);
	va_end(list);

	for (i = 0; i < nret; i++)
		prom_args.args[nargs + i] = 0;

	if (promptr(&prom_args) < 0)
		return -1;

	if (rets)
		for (i = 1; i < nret; ++i)
			rets[i - 1] = prom_args.args[nargs + i];

	if (nret > 0)
		return prom_args.args[nargs];
	return 0;
}
