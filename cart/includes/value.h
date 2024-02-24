#ifndef _VALUE_H
#define _VALUE_H

#include "common.h"
#include "arena_memory.h"

typedef double fval;

struct value
{
    int capacity;
    int count;
    arena *vals;
};

typedef struct Type Type;
typedef struct value value;
typedef value *Value;

void init_value_array(Value v);
void write_value_array(Value v, arena ar);

void free_value_array(Value v);
void print(arena ar);

#endif