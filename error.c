
#include "error.h"

void error(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	printf("\n");
	va_end(args);
}

void exit_error(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	printf("\n");
	exit(1);
}
