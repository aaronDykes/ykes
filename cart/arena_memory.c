#include "arena_memory.h"
#include <stdio.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>

#define OFFSET sizeof(Free)
#define PAGE_SIZE \
    (PAGE * OFFSET)

void initialize_global_memory(size_t size)
{

    mem.mem = mmap(
        0,
        PAGE_SIZE,
        PROT_READ | PROT_WRITE,
        POSIX_MADV_RANDOM |
            POSIX_MADV_WILLNEED |
            MAP_PRIVATE |
            MAP_ANON,
        -1, 0);

    mem.mem->size = sizeof(Free);
    mem.mem->prev = NULL;
    mem.mem->next = mem.mem + mem.mem->size;
    mem.mem->next->size = (size * sizeof(Free)) - (sizeof(Free) * 2);
    mem.mem->next->next = NULL;
    mem.mem->next->prev = mem.mem;
    mem.remains = mem.mem->next->size;
    mem.current = (sizeof(Free) * 2);
}

Arena arena_init(void *data, size_t size, T type)
{
    Arena ar;

    switch (type)
    {
    case ARENA_BYTES:
        ar.listof.Bytes = data;
        ar.len = (int)size;
        ar.count = 0;
        break;
    case ARENA_STR:
    case ARENA_FUNC:
    case ARENA_VAR:
    case ARENA_NATIVE:
        ar.as.String = data;
        ar.as.len = (int)size;
        ar.as.count = 0;
        break;
    case ARENA_INTS:
        ar.listof.Ints = data;
        ar.len = (int)(size / sizeof(int));
        ar.count = 0;
        break;
    case ARENA_DOUBLES:
        ar.listof.Doubles = data;
        ar.len = ((int)(size / sizeof(double)));
        ar.count = 0;
        break;
    case ARENA_LONGS:
        ar.listof.Longs = data;
        ar.len = ((int)(size / sizeof(long long int)));
        ar.count = 0;
        break;
    case ARENA_BYTE:
    case ARENA_INT:
    case ARENA_DOUBLE:
    case ARENA_LONG:
    case ARENA_CHAR:
    case ARENA_BOOL:
    case ARENA_STRS:
    case ARENA_NULL:
    case ARENA_BOOLS:

        break;
    }
    ar.size = size;
    ar.type = type;
    return ar;
}

/**
    TODO:
        free off each String in String arena
*/

void arena_free(Arena *ar)
{

    if (!ar)
        return;

    Free *new = NULL;
    size_t new_size = ar->size + OFFSET;

    switch (ar->type)
    {
    case ARENA_BYTES:
        new = (Free *)ar->listof.Bytes - OFFSET;
        ar->listof.Bytes = NULL;
        break;
    case ARENA_STR:
    case ARENA_FUNC:
    case ARENA_NATIVE:
    case ARENA_VAR:
        new = (Free *)ar->as.String - OFFSET;
        ar->as.String = NULL;
        break;
    case ARENA_INTS:
        new = (Free *)ar->listof.Ints - OFFSET;
        ar->listof.Ints = NULL;
        break;
    case ARENA_DOUBLES:
        new = (Free *)ar->listof.Doubles - OFFSET;
        ar->listof.Doubles = NULL;
        break;
    case ARENA_LONGS:
        new = (Free *)ar->listof.Longs - OFFSET;
        ar->listof.Longs = NULL;
        break;
    case ARENA_STRS:
    case ARENA_BOOLS:
    case ARENA_BYTE:
    case ARENA_INT:
    case ARENA_DOUBLE:
    case ARENA_LONG:
    case ARENA_CHAR:
    case ARENA_BOOL:
    case ARENA_NULL:
        return;
    }

    if (!new)
        return;

    for (; new->next; new = new->next)
        ;

    for (Free *f = mem.mem; f; f = f->next)

        if (new == f->next)
        {

            Free *next = f->next->next;
            size_t tmp = new_size + f->next->size;

            f->next = new;
            f->next->size = new_size;
            f->next->prev = f;

            if (!next)
            {
                f->next->size = tmp;
                f->next->next = NULL;
            }
            else
            {
                f->next->next = next;
                f->next->next->prev = f->next;
            }

            mem.remains += new_size;
            mem.current -= new_size;

            return;
        }

    Free *next = mem.mem->next;

    mem.mem->next = new;
    mem.mem->next->size = new_size;
    mem.mem->next->prev = mem.mem;
    mem.mem->next->next = next;
    if (next)
        mem.mem->next->next->prev = mem.mem->next;

    mem.current -= new_size;
    mem.remains += new_size;

    ar = NULL;
}

void destroy_global_memory()
{
    munmap(mem.mem, PAGE_SIZE);
}

void *alloc_ptr(size_t size)
{

    size_t new_size = size + OFFSET;

    mem.current += new_size;
    for (Free *f = mem.mem; f; f = f->next)
        if (f->size >= new_size)
        {

            void *ptr = f + OFFSET;
            Free *prev = f->prev;

            size_t tmp =
                (!f->next)
                    ? f->size - new_size
                    : f->next->size + (f->size - new_size);

            f->next = f + new_size;
            f->next->size = tmp;
            f = f->next;

            prev->next = f;
            f->prev = prev;

            mem.remains -= size;
            return (void *)ptr;
        }

    return NULL;
}
Arena *arena_alloc_arena(size_t size)
{
    Arena *p = alloc_ptr((size * sizeof(Arena)) + sizeof(Arena));

    p->size = size;

    return p + 1;
}

Arena *arena_realloc_arena(Arena *ar, size_t size)
{
    Arena *ptr = NULL;
    if (!ar && size != 0)
    {
        ptr = arena_alloc_arena(size);
        return ptr;
    }
    if (size == 0)
    {
        arena_free_arena(ar);
        return NULL;
    }

    ptr = arena_alloc_arena(size);

    size_t new_size = (size <= (ar - 1)->size) ? size : (ar - 1)->size;

    for (size_t i = 0; i < new_size; i++)
        ptr[i] = ar[i];

    arena_free_arena(ar);
    return ptr;
}

void arena_free_arena(Arena *ar)
{
    if (!(ar - 1))
        return;
    size_t new_size = ((ar - 1)->size * sizeof(Arena)) + sizeof(Arena) + OFFSET;

    for (size_t i = 0; i < (ar - 1)->size; i++)
        switch (ar[i].type)
        {
        case ARENA_BYTES:
        case ARENA_INTS:
        case ARENA_DOUBLES:
        case ARENA_LONGS:
        case ARENA_BOOLS:
        case ARENA_STR:
        case ARENA_STRS:
        case ARENA_FUNC:
        case ARENA_NATIVE:
        case ARENA_VAR:
            ARENA_FREE(&ar[i]);
            break;
        case ARENA_BYTE:
        case ARENA_INT:
        case ARENA_DOUBLE:
        case ARENA_LONG:
        case ARENA_CHAR:
        case ARENA_BOOL:
        case ARENA_NULL:
            break;
        }

    Free *f = (Free *)(ar - 1) - OFFSET;

    if (!f)
        return;

    for (; f->next; f = f->next)
        ;

    for (Free *p = mem.mem; p; p = p->next)
    {
        if (p == f)
        {
            Free *next = p->next;

            size_t tmp = new_size + p->size;

            // if (next)
            // tmp += p->next->size;
            p = f;
            p->size = new_size;
            p->prev = p;
            // p->prev->next = p->next;

            if (!next)
            {
                p->size = tmp;
                p->next = NULL;
            }
            else
            {
                p->next = next;
                p->next->prev = p->next;
            }

            mem.remains += new_size;
            mem.current -= new_size;

            return;
        }
    }
}

Arena arena_alloc(size_t size, T type)
{

    void *ptr = alloc_ptr(size);
    return arena_init(ptr, size, type);
}

Arena arena_realloc(Arena *ar, size_t size, T type)
{

    if (size == 0)
    {
        arena_free(ar);
        return Null();
    }

    void *ptr = alloc_ptr(size);
    if (!ar && size != 0)
        return arena_init(ptr, size, type);

    size_t new_size = (size >= ar->size) ? ar->size : size;

    switch (type)
    {
    case ARENA_BYTES:
        if (!ar->listof.Bytes)
            return arena_init(ptr, size, type);
        memcpy(ptr, ar->listof.Bytes, new_size);
        break;
    case ARENA_STR:
    case ARENA_VAR:
    case ARENA_FUNC:
    case ARENA_NATIVE:
        if (!ar->as.String)
            return arena_init(ptr, size, type);
        memcpy(ptr, ar->as.String, new_size);
        break;
    case ARENA_INTS:
        if (!ar->listof.Ints)
            return arena_init(ptr, size, type);
        memcpy(ptr, ar->listof.Ints, new_size);
        break;
    case ARENA_DOUBLES:
    case ARENA_LONGS:
    case ARENA_BOOLS:
    case ARENA_BYTE:
    case ARENA_INT:
    case ARENA_DOUBLE:
    case ARENA_LONG:
    case ARENA_CHAR:
    case ARENA_STRS:
    case ARENA_BOOL:
    case ARENA_NULL:
        break;
    }

    Arena a = arena_init(ptr, size, type);
    a.count = (ar->count > a.len)
                  ? a.len
                  : ar->count;
    arena_free(ar);
    return a;
}
Arena Char(char Char)
{
    Arena ar;
    ar.type = ARENA_CHAR;
    ar.as.Char = Char;
    ar.size = sizeof(char);
    return ar;
}
Arena Int(int Int)
{
    Arena ar;
    ar.type = ARENA_INT;
    ar.as.Int = Int;
    ar.size = sizeof(int);
    return ar;
}
Arena Byte(uint8_t Byte)
{
    Arena ar;
    ar.type = ARENA_BYTE;
    ar.as.Byte = Byte;
    ar.size = sizeof(uint8_t);
    return ar;
}
Arena Long(long long int Long)
{
    Arena ar;
    ar.type = ARENA_LONG;
    ar.as.Long = Long;
    ar.size = sizeof(long long int);
    return ar;
}
Arena Double(double Double)
{
    Arena ar;
    ar.type = ARENA_DOUBLE;
    ar.as.Double = Double;
    ar.size = sizeof(double);
    return ar;
}
Arena String(const char *str)
{
    size_t size = strlen(str);
    Arena ar = arena_alloc(size, ARENA_STR);
    strcpy(ar.as.String, str);
    ar.as.String[size] = '\0';
    ar.size = size;
    return ar;
}

Arena Bool(bool Bool)
{
    Arena ar;
    ar.type = ARENA_BOOL;
    ar.as.Bool = Bool;
    ar.size = sizeof(bool);
    return ar;
}
Arena Null()
{
    Arena ar;
    ar.type = ARENA_NULL;
    ar.size = sizeof(void);
    return ar;
}

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
        case NULL_OBJ:
        case SCRIPT:
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

    size_t new_size = (((tmp - 1)->size * sizeof(Stack)) + sizeof(Stack)) + OFFSET;

    if (!(tmp - 1))
        return;
    for (size_t i = 0; i < (tmp - 1)->size; i++)
        switch (tmp[i].as.type)
        {
        case ARENA:
            ARENA_FREE(&tmp[i].as.arena);
            break;
        case NATIVE:
            FREE_NATIVE(tmp[i].as.native);
            break;
        case CLOSURE:
            FREE_CLOSURE(tmp[i].as.closure);
            break;
        case SCRIPT:
        case NULL_OBJ:
            break;
        }

    Free *f = (Free *)((*stack) - 1) - OFFSET;

    if (!f)
        return;

    for (; f->next; f = f->next)
        ;

    for (Free *p = mem.mem; p; p = p->next)
    {
        if (p == f)
        {
            Free *next = p->next;

            size_t tmp = new_size + p->size;

            p = f;
            p->size = new_size;
            p->prev = p;

            if (!next)
            {
                p->size = tmp;
                p->next = NULL;
            }
            else
            {
                p->next = next;
                p->next->prev = p->next;
            }

            mem.remains += new_size;
            mem.current -= new_size;

            return;
        }
    }
    tmp = NULL;
}

Upval **indices(size_t size)
{
    Upval **up = ALLOC((sizeof(Upval *) * size) + sizeof(Upval *));

    for (size_t i = 0; i < size; i++)
        up[i] = NULL;

    return up;
}

void free_indices(Upval **up)
{
    for (size_t i = 0; i < ((*up) - 1)->size; i++)
        FREE_STACK((*up)[i].index);

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
Element null_obj()
{
    Element el;
    el.null = NULL;
    el.type = NULL_OBJ;
    return el;
}

Function *function(Arena name)
{
    Function *func = ALLOC(sizeof(Function));
    func->arity = 0;
    func->upvalue_count = 0;
    func->name = name;
    init_chunk(&func->ch);
    func->params = GROW_TABLE(NULL, TABLE_SIZE);

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
    up->closed = *index;
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

Table *arena_alloc_table(size_t size)
{
    Table *t = ALLOC((size * sizeof(Table)) + sizeof(Table));

    size_t n = (size_t)size + 1;

    for (size_t i = 1; i < n; i++)
        t[i] = arena_entry(Null(), Null());

    t->size = n;
    t->len = (int)size;
    t->count = 0;
    return t + 1;
}

Table *arena_realloc_table(Table *t, size_t size)
{

    Table *ptr = NULL;

    if (!t && size != 0)
    {
        ptr = arena_alloc_table(size);
        return ptr;
    }
    if (size == 0)
    {
        arena_free_table(t);
        return NULL;
    }

    ptr = arena_alloc_table(size);

    size_t new_size = (size <= (t - 1)->size) ? size : (t - 1)->size;

    for (size_t i = 0; i < new_size; i++)
        ptr[i] = new_entry(t[i]);

    arena_free_table(t);
    return ptr;
}

void arena_free_table(Table *t)
{
    if (!(t - 1))
        return;

    size_t size = (t - 1)->size;

    for (size_t i = 0; i < size; i++)
    {
        FREE_TABLE_ENTRY(&t[i]);
        t[i].size = 0;
    }

    (t - 1)->size = 0;
    --t;
    t = NULL;
}

void arena_free_entry(Table *entry)
{
    if (entry->type == ARENA)
        FREE_ARRAY(&entry->val.a);
    else if (entry->type == NATIVE)
        FREE_NATIVE(entry->val.n);
    else
        FREE_CLOSURE(entry->val.c);

    FREE_ARRAY(&entry->key);
    entry->next = NULL;
    entry->prev = NULL;
    entry = NULL;
}

void alloc_entry(Table **e, Table el)
{
    Table *tmp = *e;

    if (!tmp)
    {
        *e = ALLOC(sizeof(Table));
        **e = el;
        return;
    }
    for (; tmp->next; tmp = tmp->next)
        ;
    tmp->next = ALLOC(sizeof(Table));
    *tmp->next = el;
    tmp->next->prev = tmp;
}

Table new_entry(Table t)
{
    Table el;
    el.key = t.key;
    switch (t.type)
    {
    case ARENA:
        el.val.a = t.val.a;
        break;
    case NATIVE:
        el.val.n = t.val.n;
        break;
    case CLOSURE:
        el.val.c = t.val.c;
        break;
    case SCRIPT:
    case NULL_OBJ:
        break;
    }
    el.next = t.next;
    el.prev = t.prev;
    el.type = t.type;
    return el;
}

Table Entry(Arena key, Element val)
{
    switch (val.type)
    {
    case ARENA:
        return arena_entry(key, val.arena);
    case NATIVE:
        return native_entry(val.native);
    case CLOSURE:
        return func_entry(val.closure);
    case SCRIPT:
    case NULL_OBJ:
        break;
    }
    return func_entry(NULL);
}

Table arena_entry(Arena key, Arena val)
{
    Table el;
    el.key = key;
    el.val.a = val;
    el.next = NULL;
    el.prev = NULL;
    el.size = key.size + val.size;
    el.type = ARENA;
    return el;
}
Table func_entry(Closure *clos)
{
    Table el;
    el.key = clos->func->name;
    el.val.c = clos;
    el.next = NULL;
    el.prev = NULL;
    el.size = el.key.size;
    el.type = CLOSURE;
    return el;
}
Table native_entry(Native *func)
{
    Table el;
    el.key = func->obj;
    el.val.n = func;
    el.next = NULL;
    el.prev = NULL;
    el.size = el.key.size;
    el.type = NATIVE;
    return el;
}

Arena Var(const char *str)
{
    size_t size = strlen(str);
    Arena ar = GROW_ARRAY(NULL, size, ARENA_VAR);
    memcpy(ar.as.String, str, size);
    ar.as.String[size] = '\0';
    size_t h = hash(ar);
    ar.as.hash = h;
    ar.type = ARENA_VAR;
    return ar;
}
Arena func_name(const char *str)
{
    size_t size = strlen(str);
    Arena ar = GROW_ARRAY(NULL, size, ARENA_FUNC);
    memcpy(ar.as.String, str, size);
    ar.as.String[size] = '\0';
    size_t h = hash(ar);
    ar.as.hash = h;
    ar.type = ARENA_FUNC;
    return ar;
}
Arena native_name(const char *str)
{
    size_t size = strlen(str);
    Arena ar = GROW_ARRAY(NULL, size, ARENA_NATIVE);
    memcpy(ar.as.String, str, size);
    ar.as.String[size] = '\0';
    size_t h = hash(ar);
    ar.as.hash = h;
    ar.type = ARENA_NATIVE;
    return ar;
}

size_t hash(Arena key)
{
    size_t index = 2166136261u;

    switch (key.type)
    {
    case ARENA_VAR:
    case ARENA_FUNC:
    case ARENA_NATIVE:
        for (char *s = key.as.String; *s; s++)
        {
            index ^= (int)*s;
            index *= 16777619;
        }
        break;
    case ARENA_INT:
        index ^= key.as.Int;
        index = (index * 16777669);
        break;
    case ARENA_DOUBLE:
        index ^= ((int)key.as.Double);
        index = (index * 16777420);
        break;
    case ARENA_LONG:
        index ^= key.as.Long;
        index = (index * 16776969);
        break;
    case ARENA_CHAR:
        index ^= key.as.Char;
        index = (index * 16742069);
        break;
    case ARENA_BYTE:
    case ARENA_STR:
    case ARENA_BOOL:
    case ARENA_NULL:
    case ARENA_BYTES:
    case ARENA_INTS:
    case ARENA_DOUBLES:
    case ARENA_LONGS:
    case ARENA_BOOLS:
    case ARENA_STRS:
        break;
    }
    return index;
}

void print_arena(Arena ar)
{
    switch (ar.type)
    {
    case ARENA_CHAR:
        printf("%c\n", ar.as.Char);
        break;
    case ARENA_BYTES:
        if (!ar.listof.Bytes)
            return;
        printf("Byte ptr len: %d\n", ar.count);
        for (int i = 0; i < ar.count; i++)
            printf("%d ", ar.listof.Bytes[i]);
        printf("\n");
        break;
    case ARENA_INTS:
        if (!ar.listof.Ints)
            return;
        printf("Int ptr len: %d\n", ar.count);
        for (int i = 0; i < ar.count; i++)
            printf("%d ", ar.listof.Ints[i]);
        printf("\n");
        break;
    case ARENA_DOUBLES:
        if (!ar.listof.Doubles)
            return;
        printf("Double ptr len: %d\n", ar.count);
        for (int i = 0; i < ar.count; i++)
            printf("%f ", ar.listof.Doubles[i]);
        printf("\n");
        break;
    case ARENA_LONGS:
        if (!ar.listof.Longs)
            return;
        printf("Llint ptr len: %d\n", ar.count);
        for (int i = 0; i < ar.count; i++)
            printf("%lld ", ar.listof.Longs[i]);
        printf("\n");
        break;
    case ARENA_STR:
        if (!ar.as.String)
            return;
        printf("%s\n", ar.as.String);
        break;
    case ARENA_BYTE:
        printf("%d\n", ar.as.Byte);
        break;
    case ARENA_INT:
        printf("%d\n", ar.as.Int);
        break;
    case ARENA_DOUBLE:
        printf("%f\n", ar.as.Double);
        break;
    case ARENA_LONG:
        printf("%lld\n", ar.as.Long);
        break;

    case ARENA_BOOL:
    case ARENA_NULL:
    case ARENA_BOOLS:
    case ARENA_STRS:
    case ARENA_FUNC:
    case ARENA_NATIVE:
    case ARENA_VAR:
        break;
    }
}
