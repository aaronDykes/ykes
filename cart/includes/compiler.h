#ifndef _YKES_COMPILER_H
#define _YKES_COMPILER_H
#include "virtual_machine.h"

bool compile(const char *src, Chunk ch, vm *machine);
#endif