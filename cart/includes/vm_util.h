#ifndef _VM_UTIL_H
#define _VM_UTIL_H
#include "arena_math.h"

static element find(table *t, arena tmp);
static void close_upvalues(stack *local);
static void define_native(arena ar, NativeFn native);
static inline element clock_native(int argc, stack *args);
static inline element file_native(int argc, stack *argv);
static inline element square_native(int argc, stack *args);
static inline element prime_native(int argc, stack *args);
#endif
