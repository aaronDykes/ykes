#include "arena_memory.h"
#include <stdio.h>

void initialize_global_memory(size_t size)
{

    mem.glob = malloc(sizeof(Free) * size);

    mem.mem = mem.glob;
    mem.mem->size = sizeof(Free);
    mem.mem->prev = NULL;
    mem.mem->next = mem.glob + mem.mem->size;
    mem.mem->next->size = (size * sizeof(Free)) - (sizeof(Free) * 2);
    mem.mem->next->next = NULL;
    mem.mem->next->prev = mem.mem;
    mem.remains = mem.mem->next->size;
    mem.current = (sizeof(Free) * 2);
}

void reset_global_mem()
{
    mem.mem = mem.glob + mem.current;
    mem.mem->size = sizeof(Free);
    mem.mem->prev = NULL;
    mem.mem->next = mem.mem + sizeof(Free);
    mem.mem->next->size = mem.current - sizeof(Free);
    mem.mem->next->prev = mem.mem;
    mem.mem->next->next = NULL;
    mem.remains = mem.current;
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

#define OFFSET sizeof(Free);
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
    Free *f = mem.mem;

    for (; f; f = f->next)

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
#undef OFFSET
}

void destroy_global_memory()
{
    free(mem.glob);
}

void *alloc_ptr(size_t size)
{
#define OFFSET sizeof(Free)

    size_t new_size = size + OFFSET;

    mem.current += new_size;
    for (Free *f = mem.mem; f; f = f->next)
        if (f->size >= new_size)
        {

            void *ptr = NULL;
            Free *prev = NULL;

            ptr = f + OFFSET;
            prev = f->prev;

            if (!f->next)
            {
                f->next = f + new_size;
                f->next->size = f->size - new_size;
            }
            else
            {
                size_t tmp = f->next->size + (f->size - new_size);
                f->next = f + new_size;
                f->next->size = tmp;
            }
            f = f->next;

            if (prev)
            {
                prev->next = f;
                f->prev = prev;
            }

            prev = NULL;
            mem.remains -= size;
            return (void *)ptr;
        }

    return NULL;
    // reset_global_mem();
    // return alloc_ptr(size);
    // return mem.glob + mem.current;

#undef OFFSET
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
            arena_free(&ar[i]);
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

    (ar - 1)->size = 0;
    (ar - 1)->type = ARENA_NULL;
    --ar;
    ar = NULL;
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
