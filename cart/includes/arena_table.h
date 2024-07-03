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
element find_entry(table **t, arena *hash);
table *alloc_table(size_t size);
table *realloc_table(table *t, size_t size);

#endif
