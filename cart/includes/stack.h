#ifndef _STACK_H
#define _STACK_H

#include "arena_memory.h"

int add_constant(chunk *c, element ar);

void push(stack **s, element e);
element pop(stack **s);
void popn(stack **s, int ival);
void reset_stack(stack *s);
void check_stack_size(stack *s);

#endif
