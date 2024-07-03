#ifndef _DEBUG_H
#define _DEBUG_H

#include "stack.h"

void disassemble_chunk(chunk *c, const char *str);
int disassemble_instruction(chunk *c, int offset);

#endif
