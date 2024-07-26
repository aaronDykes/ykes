#include "stack.h"
#include <stdio.h>

void reset_stack(stack *s)
{
    s->top = s->as;
}

void check_stack_size(stack *s)
{

    if (!s)
        return;
    if (s->count + 1 > s->len)
    {
        s->len = GROW_CAPACITY(s->len);
        s = GROW_STACK(s, s->len);
        reset_stack(s);
    }
}

element pop(stack **s)
{
    --(*s)->count;
    return *--(*s)->top;
}

void popn(stack **s, int ival)
{
    for (int i = 0; i < ival; i++)
        --(*s)->count, --(*s)->top;
}

void push(stack **s, element e)
{
    check_stack_size(*s);

    if (!*s || !(*s)->as)
    {
        *s = GROW_STACK(NULL, STACK_SIZE);
        reset_stack(*s);
    }

    *(*s)->top = e;
    ++(*s)->top;
    (*s)->count++;
}
