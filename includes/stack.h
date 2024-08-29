#ifndef _STACK_H
#define _STACK_H

#include "object_memory.h"

int add_constant(chunk *c, element ar);

void     push(stack **s, element e);
element *pop(stack **s);
void     popn(stack **s, int ival);

field_stack *_fstack(void);
field_stack *realloc_field_stack(field_stack **f);
void         free_field_stack(field_stack **f);

init_table pop_itab(field_stack **f);
void       push_itab(field_stack **f, uint8_t init, table *field);

#endif
