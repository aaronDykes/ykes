#ifndef _YKES_COMPILER_H
#define _YKES_COMPILER_H
#include "stack.h"

function *compile(const char *src);
function *compile_path(const char *src, const char *path, const char *name);
#endif
