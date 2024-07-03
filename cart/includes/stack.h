#ifndef _STACK_H
#define _STACK_H

#include "arena_memory.h"

void write_chunk(chunk *c, uint8_t byte, int line);
int add_constant(chunk *c, element ar);

void push(stack **s, element e);
void popn(stack **s, int ival);
void reset_stack(stack *s);
void check_stack_size(stack *s);

#endif
