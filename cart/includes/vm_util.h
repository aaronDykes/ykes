#ifndef _VM_UTIL_H
#define _VM_UTIL_H
#include "arena_math.h"

static void close_upvalues(element *local);
static void define_native(_key ar, NativeFn native);
static inline element clock_native(int argc, element *argv);
static inline element file_native(int argc, element *argv);
static inline element square_native(int argc, element *argv);

#endif
