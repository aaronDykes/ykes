#include "stack.h"
#include "debug.h"
#include <stdio.h>

void free_function(Function *f);

Stack *stack(size_t size)
{
    Stack *s = NULL;
    s = alloc_ptr((size * sizeof(Stack)) + sizeof(Stack));
    s->size = size;
    (s + 1)->len = (int)size;
    (s + 1)->count = 0;
    (s + 1)->top = s + 1;
    return s + 1;
}
Stack *realloc_stack(Stack *st, size_t size)
{
    if (size == 0)
    {
        free_stack(st);
        return NULL;
    }

    Stack *s = stack(size);

    if (!st)
        return s;

    for (size_t i = 0; i < (st - 1)->size; i++)
        switch (st[i].as.type)
        {
        case ARENA:
            s[i].as = Obj(st[i].as.arena);
            break;
        case FUNC:
            s[i].as = Func(st[i].as.func);
            break;
        }
    free_stack(st);
    return s;
}
void free_stack(Stack *stack)
{
    if (!stack)
        return;
    for (size_t i = 0; i < (stack - 1)->size; i++)
        switch (stack[i].as.type)
        {
        case ARENA:
            arena_free(&stack[i].as.arena);
            break;
        case FUNC:
            free_function(stack[i].as.func);
            break;
        }
    stack = NULL;
}
Element Obj(arena ar)
{
    Element s;
    s.arena = ar;
    s.type = ARENA;
    return s;
}
Element Func(Function *f)
{
    Element s;

    s.func = f;
    s.type = FUNC;
    return s;
}

Function *function()
{
    Function *func = alloc_ptr(sizeof(Function));
    func->arity = 0;
    init_chunk(&func->ch);
    return func;
}

void free_function(Function *func)
{
    if (!func)
        return;
    if (func->name.type != ARENA_NULL)
        FREE_ARRAY(&func->name);
    free_chunk(&func->ch);
    func = NULL;
}

void init_chunk(Chunk *c)
{
    c->op_codes = arena_alloc(STACK_SIZE * sizeof(uint8_t), ARENA_BYTE_PTR);
    c->op_codes.len = STACK_SIZE;
    c->op_codes.count = 0;
    c->cases = arena_alloc(STACK_SIZE * sizeof(int), ARENA_INT_PTR);
    c->cases.len = STACK_SIZE;
    c->cases.count = 0;
    c->line = 0;
    c->constants = GROW_STACK(c->constants, STACK_SIZE);
}

void write_chunk(Chunk *c, uint8_t byte)
{

    if (c->op_codes.len < c->op_codes.count + 1)
    {
        c->op_codes.len *= INC;
        c->op_codes = GROW_ARRAY(&c->op_codes, c->op_codes.len * sizeof(uint8_t));
    }
    if (c->cases.len < c->cases.count + 1)
    {
        c->cases.len *= INC;
        c->cases = GROW_ARRAY(&c->cases, c->cases.len * sizeof(int));
    }

    c->op_codes.listof.Bytes[c->op_codes.count++] = byte;
}

int add_constant(Chunk *c, Element ar)
{
    push(&c->constants, ar);
    return c->constants->count - 1;
}

void free_chunk(Chunk *c)
{
    FREE_ARRAY(&c->op_codes);
    FREE_ARRAY(&c->cases);
    FREE_STACK(c->constants);
    init_chunk(c);
}

void reset_stack(Stack *s)
{
    s->top = s;
}
void check_stack_size(Stack *s)
{
    if (s->count + 1 > s->len)
    {
        s->len *= INC;
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
    (*st->top++).as = e;
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
    arena a = ar.arena;
    if (ar.type == FUNC && ar.func)
    {
        printf("<fn %s>\n", ar.func->name.as.String ? ar.func->name.as.String : "<fn NULL>");
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
    case ARENA_INT_PTR:
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
    }
}