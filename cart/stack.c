#include "stack.h"
#include <stdio.h>

void write_chunk(chunk *c, uint8_t byte, int line)
{

    size_t size = 0;
    if (c->op_codes.len < c->op_codes.count + 1)
    {

        size = c->op_codes.size * INC;
        c->op_codes = GROW_ARENA(&c->op_codes, size * sizeof(uint8_t), ARENA_BYTES);
    }

    if (c->lines.len < c->lines.count + 1)
    {
        size = c->lines.size * INC;
        c->lines = GROW_ARENA(&c->lines, size * sizeof(int), ARENA_INTS);
    }

    c->lines.listof.Ints[c->lines.count++] = line;
    c->op_codes.listof.Bytes[c->op_codes.count++] = byte;
}

int add_constant(chunk *c, element ar)
{
    push(&c->constants, ar);
    return c->constants->count - 1;
}

void reset_stack(stack *s)
{
    s->top = s;
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

void popn(stack **s, int ival)
{
    for (int i = 0; i < ival; i++)
        --(*s)->count, --(*s)->top;
}

void push(stack **s, element e)
{
    stack *st = *s;
    check_stack_size(st);

    if (!st)
    {
        st = GROW_STACK(NULL, STACK_SIZE);
        *s = st;
        reset_stack(*s);
    }

    (st->top++)->as = e;
    st->count++;
}
