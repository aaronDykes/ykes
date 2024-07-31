#include "stack.h"
#include <stdio.h>

void check_stack_size(stack **s)
{

    if (!*s || !(*s)->as)
        return;
    if ((*s)->count + 1 > (*s)->len)
    {
        *s = GROW_STACK(*s, (*s)->len * INC);
    }
}

element pop(stack **s)
{

    return *((*s)->as + --(*s)->count);
}

void popn(stack **s, int ival)
{
    (*s)->count -= ival;

    if ((*s)->count < 0)
        (*s)->count = 0;
}

void push(stack **s, element e)
{

    check_stack_size(s);

    if (!*s || !(*s)->as)
        *s = GROW_STACK(NULL, STACK_SIZE);

    *((*s)->as + (*s)->count++) = e;
}
