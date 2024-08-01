#include "stack.h"
#include <stdio.h>

static void check_stack_size(stack **s)
{

    if (!*s || !(*s)->as)
    {
        *s = GROW_STACK(NULL, STACK_SIZE);
        return;
    }
    if ((*s)->count + 1 > (*s)->len)
        *s = GROW_STACK(*s, (*s)->len * INC);
}
static void check_stack_index(stack **s, uint8_t index)
{

    if (!*s || !(*s)->as)
    {
        *s = GROW_STACK(NULL, STACK_SIZE);
        return;
    }
    if (index + 1 > (*s)->len)
        *s = GROW_STACK(*s, (*s)->len * INC);
}

element pop(stack **s)
{

    if ((*s)->count == 0)
        return *((*s)->as);
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

    *((*s)->as + (*s)->count++) = e;
}

void set_object(stack **s, uint8_t index, element obj)
{
    check_stack_index(s, index);
    *((*s)->as + index) = obj;
}
element get_object(stack **s, uint8_t index)
{
    check_stack_index(s, index);
    return *((*s)->as + index);
}