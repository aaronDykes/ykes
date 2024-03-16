#ifndef _STACK_H
#define _STACK_H

#include "arena_memory.h"

void write_chunk(Chunk *c, uint8_t byte);
int add_constant(Chunk *c, Element ar);
void print(Element ar);
void push(Stack **s, Element e);
void popn(Stack **s, int ival);
void check_stack_size(Stack *s);
void reset_stack(Stack *s);

#endif