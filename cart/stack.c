#include "stack.h"
#include <stdio.h>

void write_chunk(chunk *c, uint8_t byte, int line)
{

    if (c->op_codes.len < c->op_codes.count + 1)
    {

        c->op_codes.len = GROW_CAPACITY(c->op_codes.len);
        c->op_codes = GROW_ARENA(&c->op_codes, c->op_codes.len * sizeof(uint8_t), ARENA_BYTES);
    }

    if (c->lines.len < c->lines.count + 1)
    {
        c->lines.len = GROW_CAPACITY(c->lines.len);
        c->lines = GROW_ARENA(&c->lines, c->lines.len * sizeof(int), ARENA_INTS);
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
