#ifndef _STACK_H
#define _STACK_H

#include "object_memory.h"

int add_constant(chunk *c, element ar);

void push(stack **s, element e);
element pop(stack **s);
void popn(stack **s, int ival);

#endif
