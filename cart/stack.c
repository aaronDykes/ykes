#include "stack.h"
#include <stdio.h>

void free_function(Function *f);

Stack *stack(size_t size)
{
    Stack *s = NULL;
    s = ALLOC((size * sizeof(Stack)) + sizeof(Stack));
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
        free_stack(&st);
        return NULL;
    }
    Stack *s = NEW_STACK(size);

    if (!st)
        return s;

    for (size_t i = 0; i < (st - 1)->size; i++)
        switch (st[i].as.type)
        {
        case ARENA:
            s[i].as = OBJ(st[i].as.arena);
            break;
        case NATIVE:
            s[i].as = NATIVE(st[i].as.native);
            break;
        case CLOSURE:
            s[i].as = CLOSURE(st[i].as.closure);
            break;
        }
    s->count = st->count;
    s->top += s->count;
    free_stack(&st);
    return s;
}
void free_stack(Stack **stack)
{
    Stack *tmp = *stack;
    if (!(stack - 1))
        return;
    for (size_t i = 0; i < (tmp - 1)->size; i++)
        switch (tmp[i].as.type)
        {
        case ARENA:
            FREE_ARRAY(&tmp[i].as.arena);
            break;
        case NATIVE:
            FREE_NATIVE(tmp[i].as.native);
            break;
        case CLOSURE:
            FREE_CLOSURE(tmp[i].as.closure);
            break;
        }
    tmp = NULL;
}

Upval *indices(size_t size)
{
    Upval *up = ALLOC((sizeof(Upval) * size) + sizeof(Upval));

    up->size = size;
    for (size_t i = 1; i < size; i++)
        up[i].index = NULL;

    (up + 1)->len = (int)size;
    (up + 1)->count = 0;
    return up + 1;
}

void free_indices(Upval *up)
{
    for (size_t i = 0; i < (up - 1)->size; i++)
        up[i].index = NULL;

    up = NULL;
}

Element Obj(Arena ar)
{
    Element s;
    s.arena = ar;
    s.type = ARENA;
    return s;
}
Element native_fn(Native *native)
{
    Element s;

    s.native = native;
    s.type = NATIVE;
    return s;
}

Element closure(Closure *closure)
{
    Element el;
    el.closure = closure;
    el.type = CLOSURE;
    return el;
}

Function *function(Arena name)
{
    Function *func = ALLOC(sizeof(Function));
    func->arity = 0;
    func->upvalue_count = 0;
    func->name = name;
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

Native *native(NativeFn func, Arena ar)
{
    Native *native = ALLOC(sizeof(Native));
    native->fn = func;
    native->obj = ar;
    return native;
}

void free_native(Native *native)
{

    if (!native)
        return;
    native->fn = NULL;

    FREE_ARRAY(&native->obj);
}
Closure *new_closure(Function *func)
{
    Closure *closure = ALLOC(sizeof(Closure));
    closure->func = func;
    if (!func)
        return closure;
    closure->upvals = indices(func->upvalue_count);
    closure->upval_count = func->upvalue_count;
    return closure;
}

void free_closure(Closure *closure)
{
    if (!closure)
        return;
    closure = NULL;
}

Upval *upval(Stack *index)
{
    Upval *up = ALLOC(sizeof(Upval));
    up->index = index;
    up->next = NULL;
    return up;
}
void free_upval(Upval *up)
{
    if (!up)
        return;
    up->index = NULL;
}

void init_chunk(Chunk *c)
{
    c->op_codes.len = 0;
    c->op_codes.count = 0;
    c->op_codes.listof.Bytes = NULL;
    c->cases.len = 0;
    c->cases.count = 0;
    c->cases.listof.Ints = NULL;
    c->cases.len = 0;
    c->line = 0;
    c->constants = GROW_STACK(NULL, STACK_SIZE);
}

void write_chunk(Chunk *c, uint8_t byte)
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

    c->op_codes.listof.Bytes[c->op_codes.count++] = byte;
}

int add_constant(Chunk *c, Element ar)
{
    push(&c->constants, ar);
    return c->constants->count - 1;
}

void free_chunk(Chunk *c)
{
    if (!c)
    {
        init_chunk(c);
        return;
    }
    if (c->op_codes.listof.Bytes)
        FREE_ARRAY(&c->op_codes);
    if (c->cases.listof.Ints)
        FREE_ARRAY(&c->cases);
    c->constants = NULL;
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
