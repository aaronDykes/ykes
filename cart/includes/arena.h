#ifndef _MEMORY_ARENA_H
#define _MEMORY_ARENA_H
#include <stdlib.h>
#include <stdbool.h>

typedef double Align;
typedef enum
{
    ARENA_BYTE_PTR,
    ARENA_INT_PTR,
    ARENA_DOUBLE_PTR,
    ARENA_LLINT_PTR,
    ARENA_BOOL_PTR,
    ARENA_STR,
    ARENA_VAR,

    ARENA_BYTE_CONST,
    ARENA_INT_CONST,
    ARENA_DOUBLE_CONST,
    ARENA_LLINT_CONST,
    ARENA_CHAR_CONST,
    ARENA_BOOL,
    ARENA_NULL,
    ARENA_FREE,

    ARENA,
    KHASH,
    TABLE
} T;

struct arena_struct
{
    T type;
    size_t size;
    size_t hash;
    int length;

    union
    {
        uint8_t *bytes;
        char *string;
        int *ints;
        double *doubles;
        long long int *llints;
        bool *bools;

        void *null;

        uint8_t byte;
        int ival;
        double dval;
        long long int llint;
        char ch;
        bool boolean;
    } as;
};

typedef struct arena_struct arena;
typedef arena *Arena;

#endif