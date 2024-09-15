#ifndef _YKES_COMPILER_H
#define _YKES_COMPILER_H
#include "object_util.h"

function *compile(const char *src, table **lookup);
function *compile_path(const char *src, const char *path, const char *name);
#endif
