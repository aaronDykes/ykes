#ifndef _TABLE_H
#define _TABLE_H
#include "arena_table.h"

#define LOAD_FACTOR 0.75

struct machine_table
{
    int capacity;
    int count;
    Table map;
};

typedef struct machine_table dict;
typedef dict *Dict;

void init_dict(Dict d);
void write_dict(Dict d, arena ar1, arena ar2, size_t size);
void write_func_dict(Dict d, arena a, Function *func, size_t size);
void free_dict(Dict d);

#endif