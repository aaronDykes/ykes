#ifndef _MEMORY_ARENA_H
#define _MEMORY_ARENA_H
#include <stdlib.h>
#include <stdbool.h>

typedef double Align;

enum
{
    ARENA_BYTE_PTR,
    ARENA_INT_PTR,
    ARENA_DOUBLE_PTR,
    ARENA_LONG_PTR,
    ARENA_BOOL_PTR,
    ARENA_STR,
    ARENA_VAR,
    ARENA_FUNC,

    ARENA_BYTE,
    ARENA_INT,
    ARENA_DOUBLE,
    ARENA_LONG,
    ARENA_CHAR,
    ARENA_BOOL,
    ARENA_NULL,

};

struct Array
{

    int count;
    int len;

    union
    {
        uint8_t *Bytes;
        int *Ints;
        double *Doubles;
        long long int *Longs;
        bool *Bools;
    };
};
typedef struct Array Array;

union Value
{

    struct
    {
        size_t hash;
        int len;
        int count;
        char *String;
    };

    void *null;
    uint8_t Byte;
    int Int;
    double Double;
    long long int Long;
    char Char;
    bool Bool;
};

typedef union Value Value;

struct arena_struct
{
    int type;
    size_t size;

    union
    {
        Array listof;
        Value as;
    };
};

typedef struct arena_struct arena;
typedef arena *Arena;

#endif