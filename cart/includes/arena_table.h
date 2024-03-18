#ifndef _ARENA_TABLE_H
#define _ARENA_TABLE_H
#include "stack.h"

void insert_entry(Table **t, Table entry);
void delete_arena_entry(Table **t, Arena key);
void delete_func_entry(Table **t, Arena key);

Element find_entry(Table **t, Arena *hash);
Arena find_arena_entry(Table **t, Arena *key);
Closure *find_func_entry(Table **t, Arena *key);
Native *find_native_entry(Table **t, Arena *key);

#endif
