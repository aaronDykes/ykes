#include "arena_memory.h"
#include <stdio.h>

void initialize_global_memory(size_t size)
{

    global_mem.current_size = 0;
    global_mem.max_size = size;
    global_mem.glob = malloc(size * sizeof(uint8_t));
}

arena arena_init(void *data, size_t size, int type)
{
    arena ar;

    switch (type)
    {
    case ARENA_BYTE_PTR:
        ar.as.Bytes = data;
        ar.length = (int)size;
        break;
    case ARENA_VAR:
    case ARENA_STR:
        ar.as.String = data;
        ar.length = (int)size;
        break;
    case ARENA_INT_PTR:
        ar.as.Ints = data;
        ar.length = (int)(size / sizeof(int));
        break;
    case ARENA_DOUBLE_PTR:
        ar.as.Doubles = data;
        ar.length = ((int)(size / sizeof(double)));
        break;
    case ARENA_LONG_PTR:
        ar.as.Longs = data;
        ar.length = ((int)(size / sizeof(long long int)));
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

void arena_free(Arena ar)
{
    switch (ar->type)
    {
    case ARENA_BYTE_PTR:
        ar->as.Bytes = NULL;
        break;
    case ARENA_STR:
        ar->as.String = NULL;
        break;
    case ARENA_INT_PTR:
        ar->as.Ints = NULL;
        break;
    case ARENA_DOUBLE_PTR:
        ar->as.Doubles = NULL;
        break;
    case ARENA_LONG_PTR:
        ar->as.Longs = NULL;
        break;
    }

    *ar = Null();
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
    Arena p = alloc_ptr((size * sizeof(arena)) + sizeof(arena));

    size_t n = size + 1;
    p->size = size;
    p->length = (int)size;

    for (size_t i = 1; i < n; i++)
        p[i] = Null();

    return p + 1;
}

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

    ptr = arena_alloc_arena(size);

    size_t new_size = (size <= (ar - 1)->size) ? size : (ar - 1)->size;

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
        case ARENA_LONG_PTR:
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

arena arena_alloc(size_t size, int type)
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
        if (!ar->as.Bytes)
            return arena_init(ptr, size, type);
        memcpy(ptr, ar->as.Bytes, ar->size);
        break;
    case ARENA_STR:
        if (!ar->as.String)
            return arena_init(ptr, size, type);
        memcpy(ptr, ar->as.String, ar->size);
        break;
    case ARENA_INT_PTR:
        if (!ar->as.Ints)
            return arena_init(ptr, size, type);
        memcpy(ptr, ar->as.Ints, ar->size);
        break;
    }
    arena_free(ar);
    return arena_init(ptr, size, type);
}
arena Char(char Char)
{
    arena ar;
    ar.type = ARENA_CHAR;
    ar.as.Char = Char;
    ar.length = 1;
    ar.size = sizeof(char);
    return ar;
}
arena Int(int Int)
{
    arena ar;
    ar.type = ARENA_INT;
    ar.as.Int = Int;
    ar.length = 1;
    ar.size = sizeof(int);
    return ar;
}
arena Byte(uint8_t Byte)
{
    arena ar;
    ar.type = ARENA_BYTE;
    ar.as.Byte = Byte;
    ar.length = 1;
    ar.size = sizeof(uint8_t);
    return ar;
}
arena Long(long long int Long)
{
    arena ar;
    ar.type = ARENA_LONG;
    ar.as.Long = Long;
    ar.length = 1;
    ar.size = sizeof(long long int);
    return ar;
}
arena Double(double Double)
{
    arena ar;
    ar.type = ARENA_DOUBLE;
    ar.as.Double = Double;
    ar.length = 1;
    ar.size = sizeof(double);
    return ar;
}
arena String(const char *str)
{
    size_t size = strlen(str);
    arena ar = arena_alloc(size, ARENA_STR);
    strcpy(ar.as.String, str);
    ar.as.String[size] = '\0';
    ar.length = (int)size;
    ar.size = size;
    return ar;
}

arena Bool(bool Bool)
{
    arena ar;
    ar.type = ARENA_BOOL;
    ar.as.Bool = Bool;
    ar.length = 1;
    ar.size = sizeof(bool);
    return ar;
}

arena Null()
{
    arena ar;
    ar.type = ARENA_NULL;
    ar.as.null = NULL;
    ar.length = 0;
    ar.size = sizeof(void *);
    return ar;
}

void print_arena(arena ar)
{
    switch (ar.type)
    {
    case ARENA_CHAR:
        printf("%c\n", ar.as.Char);
        break;
    case ARENA_BYTE_PTR:
        printf("Byte ptr length: %d\n", ar.length);
        for (int i = 0; i < ar.length; i++)
            printf("%d ", ar.as.Bytes[i]);
        printf("\n");
        break;
    case ARENA_INT_PTR:
        printf("Int ptr length: %d\n", ar.length);
        for (int i = 0; i < ar.length; i++)
            printf("%d ", ar.as.Ints[i]);
        printf("\n");
        break;
    case ARENA_DOUBLE_PTR:
        printf("Double ptr length: %d\n", ar.length);
        for (int i = 0; i < ar.length; i++)
            printf("%f ", ar.as.Doubles[i]);
        printf("\n");
        break;
    case ARENA_LONG_PTR:
        printf("Llint ptr length: %d\n", ar.length);
        for (int i = 0; i < ar.length; i++)
            printf("%lld ", ar.as.Longs[i]);
        printf("\n");
        break;
    case ARENA_STR:
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
    }
}
