#ifndef _NATIVE_H
#define _NATIVE_H

#include "object_math.h"

void    define_natives(stack **stk);
element clock_native(int argc, element *argv);
element file_native(int argc, element *argv);
element square_native(int argc, element *argv);

#endif
