#ifndef _VM_UTIL_H
#define _VM_UTIL_H
#include "object_math.h"

static void close_upvalues(void);
static void define_native(_key ar, NativeFn native, uint8_t index);
static inline element clock_native(int argc, element *argv);
static inline element file_native(int argc, element *argv);
static inline element square_native(int argc, element *argv);

#endif
