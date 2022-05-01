#include <stdlib.h>

#include "error.h"

void warning_msg(const char *fmt, ...)
{
	va_list valist;
	va_start(valist, fmt);

	fprintf(stderr, "CHYBA: ");
	vfprintf(stderr, fmt, valist);

	va_end(valist);
}

void error_exit(const char *fmt, ...)
{
	va_list valist;
	va_start(valist, fmt);

	fprintf(stderr, "CHYBA: ");
	vfprintf(stderr, fmt, valist);

	va_end(valist);
	exit(1);
}