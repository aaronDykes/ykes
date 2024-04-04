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
Instance *find_instance_entry(Table **t, Arena *key);
Closure *find_func_entry(Table **t, Arena *key);
Class *find_class_entry(Table **t, Arena *key);
Native *find_native_entry(Table **t, Arena *key);

Table Entry(Arena key, Element val);
Table arena_entry(Arena key, Arena val);
Table class_entry(Class *c);
Table instance_entry(Arena ar, Instance *c);
Table func_entry(Closure *c);
Table native_entry(Native *func);
Table new_entry(Table t);

void write_table(Table *t, Arena a, Element b);
Table *arena_alloc_table(size_t size);
Table *arena_realloc_table(Table *t, size_t size);
void alloc_entry(Table **e, Table el);
#endif
