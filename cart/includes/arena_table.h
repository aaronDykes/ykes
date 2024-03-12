#ifndef _ARENA_TABLE_H
#define _ARENA_TABLE_H
#include "arena_memory.h"
#include "stack.h"

#define GROW_TABLE(ar, size) \
    arena_realloc_table(ar, size)

#define ALLOC_ENTRY(a, b) \
    alloc_entry(a, b)

#define FREE_TABLE(ar) \
    arena_realloc_table(ar, 0)

#define FREE_TABLE_ENTRY(ar) \
    arena_free_entry(ar)

#define HASH_SIZE(X) \
    (250 * sizeof(X))

typedef enum
{
    ARENA_TABLE,
    NATIVE_TABLE,
    FUNC_TABLE
} TableType;

struct hash_arena
{
    size_t size;
    Arena key;
    TableType type;
    union
    {
        Native *n;
        Function *f;
        Arena a;
    } val;

    struct hash_arena *next;
    struct hash_arena *prev;
};

typedef struct hash_arena table;
typedef table *Table;

void insert_entry(Table *t, table entry);
void delete_arena_entry(Table *t, Arena key);
void delete_func_entry(Table *t, Arena key);

Arena find_arena_entry(Table *t, Arena *key);
Function *find_func_entry(Table *t, Arena *key);
Native *find_native_entry(Table *t, Arena *key);

table arena_entry(Arena key, Arena val);
table func_entry(Function *f);
table native_entry(Native *func);
table new_entry(table t);

Table arena_alloc_table(size_t size);
Table arena_realloc_table(Table t, size_t size);

void alloc_entry(Table *e, table el);
void arena_free_table(Table t);
void arena_free_entry(Table entry);

Arena Var(const char *str);
Arena func_name(const char *str);
Arena native_name(const char *str);
size_t hash(Arena key);

#endif
