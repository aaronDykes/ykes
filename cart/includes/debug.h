#ifndef _DEBUG_H
#define _DEBUG_H

#include "chunk.h"

void disassemble_chunk(Chunk c, const char *str);
int disassemble_instruction(Chunk c, int offset);

#endif