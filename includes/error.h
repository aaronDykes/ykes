#ifndef _ERR_H
#define _ERR_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void error(const char *fmt, ...);
void exit_error(const char *fmt, ...);

#endif
