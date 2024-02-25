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
    ARENA_LONG_PTR,
    ARENA_BOOL_PTR,
    ARENA_STR,
    ARENA_VAR,

    ARENA_BYTE,
    ARENA_INT,
    ARENA_DOUBLE,
    ARENA_LONG,
    ARENA_CHAR,
    ARENA_BOOL,
    ARENA_NULL,

} T;

struct arena_struct
{
    T type;
    size_t size;
    size_t hash;
    int length;

    union
    {

        uint8_t *Bytes;
        int *Ints;
        double *Doubles;
        long long int *Longs;
        bool *Bools;

        char *String;
        void *null;

        uint8_t Byte;
        int Int;
        double Double;
        long long int Long;
        char Char;
        bool Bool;
    } as;
};

typedef struct arena_struct arena;
typedef arena *Arena;

#endif