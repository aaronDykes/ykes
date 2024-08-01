#ifndef _STACK_H
#define _STACK_H

#include "arena_memory.h"

int add_constant(chunk *c, element ar);

void push(stack **s, element e);
element pop(stack **s);
void popn(stack **s, int ival);
void set_object(stack **s, uint8_t index, element obj);
element get_object(stack **s, uint8_t index);

#endif
