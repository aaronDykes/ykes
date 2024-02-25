#include "value.h"
#include "table.h"
#include <stdio.h>

void init_value_array(Value v)
{
    v->capacity = ARENA_SIZE;
    v->count = 0;
    v->vals = GROW_ARENA(NULL, sizeof(arena) * ARENA_SIZE);
}

static void check_value_size(Value v)
{
    if (v->count + 1 >= v->capacity)
    {
        v->capacity *= INC;
        v->vals = GROW_ARENA(v->vals, sizeof(arena) * v->capacity);
    }
}

void write_value_array(Value v, arena ar)
{
    check_value_size(v);
    v->vals[v->count++] = ar;
}
void free_value_array(Value v)
{
    FREE_ARENA(v->vals);
    init_value_array(v);
}

void print(arena ar)
{
    switch (ar.type)
    {
    case ARENA_BYTE:
        printf("[ %d ]\n", ar.as.Byte);
        break;
    case ARENA_CHAR:
        printf("[ %c ]\n", ar.as.Char);
        break;
    case ARENA_DOUBLE:
        printf("[ %f ]\n", ar.as.Double);
        break;
    case ARENA_INT:
        printf("[ %d ]\n", ar.as.Int);
        break;
    case ARENA_LONG:
        printf("[ %lld ]\n", ar.as.Long);
        break;
    case ARENA_BOOL:
        printf("[ %s ]\n", (ar.as.Bool == true) ? "true" : "false");
        break;
    case ARENA_STR:
        printf("[ %s ]\n", ar.as.String);
        break;
    case ARENA_INT_PTR:
        printf("[ ");
        for (int i = 0; i < ar.length; i++)
            if (i == ar.length - 1)
                printf("%d ]\n", ar.as.Ints[i]);
            else
                printf("%d, ", ar.as.Ints[i]);
        break;
    case ARENA_NULL:
        printf("[ null ]\n");
        break;
    }
}
