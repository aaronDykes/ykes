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
    case ARENA_BYTE_CONST:
        printf("[ %d ]\n", ar.as.byte);
        break;
    case ARENA_CHAR_CONST:
        printf("[ %c ]\n", ar.as.ch);
        break;
    case ARENA_DOUBLE_CONST:
        printf("[ %f ]\n", ar.as.dval);
        break;
    case ARENA_INT_CONST:
        printf("[ %d ]\n", ar.as.ival);
        break;
    case ARENA_LLINT_CONST:
        printf("[ %lld ]\n", ar.as.llint);
        break;
    case ARENA_BOOL:
        printf("[ %s ]\n", (ar.as.boolean == true) ? "true" : "false");
        break;
    // case ARENA_VAR:
    case ARENA_STR:
        printf("[ %s ]\n", ar.as.string);
        break;
    case ARENA_INT_PTR:
        printf("[ ");
        for (int i = 0; i < ar.length; i++)
            if (i == ar.length - 1)
                printf("%d ]\n", ar.as.ints[i]);
            else
                printf("%d, ", ar.as.ints[i]);
        break;
    case ARENA_NULL:
        printf("[ null ]\n");
        break;
    }
}
