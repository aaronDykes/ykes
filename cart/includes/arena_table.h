#ifndef _arena_table_H
#define _arena_table_H
#include "stack.h"

#define FREE_ENTRY(e) \
    free_entry(e)

#define FREE_TABLE(ar) \
    realloc_table(ar, 0)

#define GROW_TABLE(ar, size) \
    realloc_table(ar, size)

void write_table(table *t, arena a, element b);
void delete_entry(table **t, arena key);
element find_entry(table **t, arena *hash);

void insert_entry(table **t, table entry);
table Entry(arena key, element val);
table arena_entry(arena key, arena val);
table class_entry(class *c);
table instance_entry(arena ar, instance *c);
table table_entry(arena ar, table *t);
table func_entry(closure *c);
table native_entry(native *func);
table vector_entry(arena ar, arena *_vector);
table stack_entry(arena ar, stack *s);
table new_entry(table t);

table *alloc_table(size_t size);
table *realloc_table(table *t, size_t size);
void alloc_entry(table **e, table el);
#endif
