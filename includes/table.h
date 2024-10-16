#ifndef _TABLE_H
#define _TABLE_H

#include "stack.h"

#define LOAD_FACTOR 0.75

#define FREE_ENTRY(e)        free_entry(e)
#define FREE_TABLE(ar)       free_table(ar)
#define GROW_TABLE(ar, size) realloc_table(ar, size)

void    write_table(table *t, _key *key, element b);
element find_entry(table **t, _key *key);
table  *alloc_table(size_t size);
table  *realloc_table(table **t, size_t size);
table  *copy_table(table *t);

#endif
