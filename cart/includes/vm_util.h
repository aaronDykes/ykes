#ifndef _VM_UTIL_H
#define _VM_UTIL_H
#include "arena_math.h"

static arena find(arena tmp);
static bool exists(arena tmp);
static void define_native(arena ar, NativeFn native);
static Element clock_native(int argc, Stack *args);
static Element square_native(int argc, Stack *args);
static Element pop();
#endif