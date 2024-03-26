#ifndef _ARENA_TABLE_H
#define _ARENA_TABLE_H
#include "stack.h"

#define FREE_ENTRY(e) \
    free_entry(e)

void insert_entry(Table **t, Table entry);
void delete_arena_entry(Table **t, Arena key);
void delete_func_entry(Table **t, Arena key);

void free_entry(Element el);
Element find_entry(Table **t, Arena *hash);
Arena find_arena_entry(Table **t, Arena *key);
Closure *find_func_entry(Table **t, Arena *key);
Class *find_class_entry(Table **t, Arena *key);
Native *find_native_entry(Table **t, Arena *key);

#endif
