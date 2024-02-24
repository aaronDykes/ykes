#ifndef _ARENA_TABLE_H
#define _ARENA_TABLE_H
#include "arena_memory.h"

#define INC_SIZE(capacity) \
    ((capacity) < CAPACITY ? CAPACITY : CAPACITY * INC)

#define GROW_TABLE(ar, size) \
    arena_realloc_table(ar, size)

#define FREE_TABLE(ar) \
    arena_realloc_table(ar, 0)

#define HASH_SIZE(X) \
    (250 * sizeof(X))

struct element
{
    size_t size;
    arena key;
    arena val;
    struct element *next;
    struct element *prev;
};

typedef struct element element;

struct hash_arena
{
    size_t size;
    element entry;
};

typedef struct hash_arena table;
typedef table *Table;

void insert_entry(Table *t, element entry);
void delete_entry(Table *t, arena key);

arena find_entry(Table *t, arena *key);
element new_entry(arena *key, arena *val, size_t size);
element eentry(arena key, arena val);

Table arena_alloc_table(size_t size);
Table arena_realloc_table(Table t, size_t size);

void alloc_entry(element **e, element el);

element *alloc_entry_ptr(size_t size);
void free_entry_ptr(element *e);
void arena_free_table(Table t);
void arena_free_entry(element *entry);

arena arena_var(const char *str);
size_t hash(arena key, size_t size);

#endif
