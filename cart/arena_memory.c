#include "arena_memory.h"

#include <stdio.h>
#include <stdarg.h>

static void log_err(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
}

void initialize_global_memory(size_t size)
{

    global_mem.current_size = 0;
    global_mem.max_size = size;
    global_mem.glob = malloc(size * sizeof(uint8_t));
}

arena arena_init(void *data, size_t size, T type)
{
    arena ar;

    switch (type)
    {
    case ARENA_BYTE_PTR:
        ar.as.bytes = data;
        ar.length = (int)size;
        break;
    case ARENA_VAR:
    case ARENA_STR:
        ar.as.string = data;
        ar.length = (int)size;
        break;
    case ARENA_INT_PTR:
        ar.as.ints = data;
        ar.length = (int)(size / sizeof(int));
        break;
    case ARENA_DOUBLE_PTR:
        ar.as.doubles = data;
        ar.length = ((int)(size / sizeof(double)));
        break;
    case ARENA_LLINT_PTR:
        ar.as.ints = data;
        ar.length = ((int)(size / sizeof(long long int)));
        break;
    }
    ar.size = size;
    ar.type = type;
    return ar;
}

/**
    TODO:
        free off each string in string arena
*/

void arena_free(Arena ar)
{
    switch (ar->type)
    {
    case ARENA_BYTE_PTR:
        ar->as.bytes = NULL;
        break;
    case ARENA_STR:
        ar->as.string = NULL;
        break;
    case ARENA_INT_PTR:
        ar->as.ints = NULL;
        break;
    case ARENA_DOUBLE_PTR:
        ar->as.doubles = NULL;
        break;
    case ARENA_LLINT_PTR:
        ar->as.llints = NULL;
        break;
    }

    *ar = arena_null();
}

void destroy_global_memory()
{
    global_mem.glob -= global_mem.current_size;
    free(global_mem.glob);
    global_mem.glob = NULL;
}

void *alloc_ptr(size_t size)
{
    void *ptr = global_mem.glob;
    global_mem.glob += size + MEM_OFFSET;
    global_mem.current_size += size + MEM_OFFSET;
    return ptr;
}
Arena arena_alloc_arena(size_t size)
{
    Arena p = alloc_ptr(size);

    p->type = ARENA;
    p->size = size;
    p->length = (int)(size / sizeof(arena));

    return p + 1;
}

/**
 * TODO:
 * properly implement realloc
 */
Arena arena_realloc_arena(Arena ar, size_t size)
{
    Arena ptr = NULL;
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

    size_t new_size = (size <= (ar - 1)->size) ? size : (ar - 1)->size;
    ptr = arena_alloc_arena(size);

    for (size_t i = 0; i < new_size; i++)
        ptr[i] = ar[i];

    arena_free_arena(ar);
    return ptr;
}

void arena_free_arena(Arena ar)
{
    if (!(ar - 1))
        return;

    for (size_t i = 0; i < (ar - 1)->size; i++)
        switch (ar[i].type)
        {
        case ARENA_BYTE_PTR:
        case ARENA_INT_PTR:
        case ARENA_DOUBLE_PTR:
        case ARENA_LLINT_PTR:
        case ARENA_STR:
            arena_free(&ar[i]);
            break;
        }

    (ar - 1)->size = 0;
    (ar - 1)->length = 0;
    (ar - 1)->type = ARENA_NULL;
    --ar;
    ar = NULL;
}

arena arena_alloc(size_t size, T type)
{

    void *ptr = alloc_ptr(size);
    return arena_init(ptr, size, type);
}

arena arena_realloc(Arena ar, size_t size)
{
    int type = ar->type;

    if (size == 0)
    {
        arena_free(ar);
        return *ar;
    }

    void *ptr = alloc_ptr(size);

    switch (ar->type)
    {
    case ARENA_BYTE_PTR:
        if (!ar->as.bytes)
            return arena_init(ptr, size, type);
        memcpy(ptr, ar->as.bytes, size);
        break;
    case ARENA_STR:
        if (!ar->as.string)
            return arena_init(ptr, size, type);
        memcpy(ptr, ar->as.string, size);
        break;
    case ARENA_INT_PTR:
        if (!ar->as.ints)
            return arena_init(ptr, size, type);
        memcpy(ptr, ar->as.ints, size);
        break;
    }
    arena_free(ar);
    return arena_init(ptr, size, type);
}
arena arena_char(char ch)
{
    arena ar;
    ar.type = ARENA_CHAR_CONST;
    ar.as.ch = ch;
    ar.length = 1;
    return ar;
}
arena arena_int(int ival)
{
    arena ar;
    ar.type = ARENA_INT_CONST;
    ar.as.ival = ival;
    ar.length = 1;
    return ar;
}
arena arena_byte(uint8_t byte)
{
    arena ar;
    ar.type = ARENA_BYTE_CONST;
    ar.as.byte = byte;
    ar.length = 1;
    return ar;
}
arena arena_llint(long long int llint)
{
    arena ar;
    ar.type = ARENA_LLINT_CONST;
    ar.as.llint = llint;
    ar.length = 1;
    return ar;
}
arena arena_double(double dval)
{
    arena ar;
    ar.type = ARENA_DOUBLE_CONST;
    ar.as.dval = dval;
    ar.length = 1;
    return ar;
}
arena string(const char *str)
{
    size_t size = strlen(str);
    arena ar = arena_alloc(sizeof(char) * (size), ARENA_STR);
    memcpy(ar.as.string, str, size);
    ar.as.string[size] = '\0';
    ar.length = (int)size;
    return ar;
}

arena arena_bool(bool boolean)
{
    arena ar;
    ar.type = ARENA_BOOL;
    ar.as.boolean = boolean;
    ar.length = 1;
    return ar;
}

arena arena_null()
{
    arena ar;
    ar.type = ARENA_NULL;
    ar.as.null = NULL;
    ar.length = 0;
    return ar;
}

void print_arena(arena ar)
{
    switch (ar.type)
    {
    case ARENA_CHAR_CONST:
        printf("%c\n", ar.as.ch);
        break;
    case ARENA_BYTE_PTR:
        printf("Byte ptr length: %d\n", ar.length);
        for (int i = 0; i < ar.length; i++)
            printf("%d\n", ar.as.bytes[i]);
        break;
    case ARENA_INT_PTR:
        printf("Int ptr length: %d\n", ar.length);
        for (int i = 0; i < ar.length; i++)
            printf("%d\n", ar.as.ints[i]);
        break;
    case ARENA_DOUBLE_PTR:
        printf("Double ptr length: %d\n", ar.length);
        for (int i = 0; i < ar.length; i++)
            printf("%f\n", ar.as.doubles[i]);
        break;
    case ARENA_LLINT_PTR:
        printf("Llint ptr length: %d\n", ar.length);
        for (int i = 0; i < ar.length; i++)
            printf("%lld\n", ar.as.llints[i]);
        break;
    case ARENA_STR:
        printf("%s\n", ar.as.string);
        break;
    case ARENA_BYTE_CONST:
        printf("%d\n", ar.as.byte);
        break;
    case ARENA_INT_CONST:
        printf("%d\n", ar.as.ival);
        break;
    case ARENA_DOUBLE_CONST:
        printf("%f\n", ar.as.dval);
        break;
    case ARENA_LLINT_CONST:
        printf("%lld\n", ar.as.llint);
        break;
    }
}
