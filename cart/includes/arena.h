#ifndef _MEMORY_ARENA_H
#define _MEMORY_ARENA_H
#include <stdlib.h>
#include <stdbool.h>

typedef enum
{

    ARENA_BYTE,
    ARENA_INT,
    ARENA_DOUBLE,
    ARENA_LONG,
    ARENA_CHAR,
    ARENA_STR,
    ARENA_BOOL,
    ARENA_NULL,
    ARENA_BYTES,
    ARENA_INTS,
    ARENA_DOUBLES,
    ARENA_LONGS,
    ARENA_BOOLS,
    ARENA_STRS,
    ARENA_FUNC,
    ARENA_NATIVE,
    ARENA_VAR

} T;

typedef enum
{
    TABLE,
    STACK
} DataType;

typedef union Vector Vector;
typedef union Value Value;
typedef struct Arena Arena;
typedef struct Data Data;

union Vector
{
    uint8_t *Bytes;
    int *Ints;
    double *Doubles;
    long long int *Longs;
    char **Strings;
    bool *Bools;
    void *Void;
};

union Value
{

    struct
    {
        size_t hash;
        int len;
        int count;
        char *String;
    };

    uint8_t Byte;
    int Int;
    double Double;
    long long int Long;
    char Char;
    bool Bool;
    void *Void;
};

struct Arena
{
    size_t size;
    T type;

    union
    {
        struct
        {
            int count;
            int len;
            Vector listof;
        };

        Value as;
    };
};

#endif

/*

switch (type)
{
    case ARENA_BYTE:
    case ARENA_INT:
    case ARENA_DOUBLE:
    case ARENA_LONG:
    case ARENA_CHAR:
    case ARENA_STR:
    case ARENA_BOOL:
    case ARENA_NULL:
    case ARENA_BYTES:
    case ARENA_INTS:
    case ARENA_DOUBLES:
    case ARENA_LONGS:
    case ARENA_BOOLS:
    case ARENA_STRS:
    case ARENA_FUNC:
    case ARENA_NATIVE:
    case ARENA_VAR:
break;
}

*/