#ifndef _VM_UTIL_H
#define _VM_UTIL_H
#include "arena_math.h"

static Arena find(Arena tmp);
static bool exists(Arena tmp);
static void close_upvalues(Stack *local);
static void define_native(Arena ar, NativeFn native);
static inline Element clock_native(int argc, Stack *args);
static inline Element square_native(int argc, Stack *args);
static inline Element prime_native(int argc, Stack *args);
#endif