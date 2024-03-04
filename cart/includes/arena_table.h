#ifndef _ARENA_TABLE_H
#define _ARENA_TABLE_H
#include "arena_memory.h"
#include "stack.h"

#define GROW_TABLE(ar, size) \
    arena_realloc_table(ar, size)

#define FREE_TABLE(ar) \
    arena_realloc_table(ar, 0)

#define HASH_SIZE(X) \
    (250 * sizeof(X))

typedef enum
{
    ARENA_TABLE,
    FUNC_TABLE
} TableType;

struct hash_arena
{
    size_t size;
    arena key;
    TableType type;
    union
    {
        Function *f;
        arena a;
    } val;

    struct hash_arena *next;
    struct hash_arena *prev;
};

typedef struct hash_arena table;
typedef table *Table;

void insert_entry(Table *t, table entry);
void delete_arena_entry(Table *t, arena key);
void delete_func_entry(Table *t, arena key);

arena find_arena_entry(Table *t, arena *key);
Function *find_func_entry(Table *t, arena *key);

table arena_entry(arena key, arena val);
table func_entry(arena key, Function *f);
table new_entry(table t);

Table arena_alloc_table(size_t size);
Table arena_realloc_table(Table t, size_t size);

void alloc_entry(Table *e, table el);
void arena_free_table(Table t);
void arena_free_entry(Table entry);

arena Var(const char *str);
arena func_name(const char *str);
size_t hash(arena key);

#endif
