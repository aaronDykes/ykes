#include "stack.h"
#include <stdio.h>

void write_chunk(Chunk *c, uint8_t byte, int line)
{

    if (c->op_codes.len < c->op_codes.count + 1)
    {
        c->op_codes.len = GROW_CAPACITY(c->op_codes.len);
        c->op_codes = GROW_ARRAY(&c->op_codes, c->op_codes.len * sizeof(uint8_t), ARENA_BYTES);
    }
    if (c->cases.len < c->cases.count + 1)
    {
        c->cases.len = GROW_CAPACITY(c->cases.len);
        c->cases = GROW_ARRAY(&c->cases, c->cases.len * sizeof(int), ARENA_INTS);
    }

    if (c->lines.len < c->lines.count + 1)
    {
        c->lines.len = GROW_CAPACITY(c->lines.len);
        c->lines = GROW_ARRAY(&c->lines, c->lines.len * sizeof(int), ARENA_INTS);
    }

    c->lines.listof.Ints[c->lines.count++] = line;
    c->op_codes.listof.Bytes[c->op_codes.count++] = byte;
}
int add_constant(Chunk *c, Element ar)
{
    push(&c->constants, ar);
    return c->constants->count - 1;
}

void reset_stack(Stack *s)
{
    s->top = s;
}
void check_stack_size(Stack *s)
{
    if (s->count + 1 > s->len)
    {
        s->len = GROW_CAPACITY(s->len);
        s = GROW_STACK(s, s->len);
        reset_stack(s);
    }
}

void popn(Stack **s, int ival)
{
    for (int i = 0; i < ival; i++)
        --(*s)->count, --(*s)->top;
}

void push(Stack **s, Element e)
{

    Stack *st = *s;
    check_stack_size(st);
    (st->top++)->as = e;
    st->count++;
}

static void parse_str(const char *str)
{
    char *s = (char *)str;

    for (; *s; s++)
        if (*s == '\\' && s[1] == 'n')
            printf("\n"), s++;
        else if (*s == '\\' && s[1] == 't')
            printf("\t"), s++;
        else
            printf("%c", *s);

    printf("\n");
}

void print(Element ar)
{
    Arena a = ar.arena;

    if (ar.type == NATIVE)
    {
        printf("<native: %s>\n", ar.native->obj.as.String);
        return;
    }
    else if (ar.type == CLOSURE)
    {
        printf("<fn: %s>\n", ar.closure->func->name.as.String);
        return;
    }
    switch (a.type)
    {
    case ARENA_BYTE:
        printf("%d\n", a.as.Byte);
        break;
    case ARENA_CHAR:
        printf("%c\n", a.as.Char);
        break;
    case ARENA_DOUBLE:
        printf("%f\n", a.as.Double);
        break;
    case ARENA_INT:
        printf("%d\n", a.as.Int);
        break;
    case ARENA_LONG:
        printf("%lld\n", a.as.Long);
        break;
    case ARENA_BOOL:
        printf("%s\n", (a.as.Bool == true) ? "true" : "false");
        break;
    case ARENA_STR:
    case ARENA_VAR:
    case ARENA_FUNC:
        parse_str(a.as.String);
        break;
    case ARENA_INTS:
        printf("[ ");
        for (int i = 0; i < a.len; i++)
            if (i == a.len - 1)
                printf("%d ]\n", a.listof.Ints[i]);
            else
                printf("%d, ", a.listof.Ints[i]);
        break;
    case ARENA_NULL:
        printf("[ null ]\n");
        break;

    case ARENA_BYTES:
    case ARENA_DOUBLES:
    case ARENA_LONGS:
    case ARENA_BOOLS:
    case ARENA_STRS:
    case ARENA_NATIVE:
        break;
    }
}
