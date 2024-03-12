#ifndef _ARENA_MEMORY_H
#define _ARENA_MEMORY_H

#include <string.h>
#include "arena.h"

#define CAPACITY 50
#define INC 2
#define PAGE 16384 * 8
#define FREE_LIST 2056
#define ARENA_SIZE 100
#define STACK_SIZE 250
#define TABLE_SIZE 250
#define IP_SIZE 100
#define MEM_OFFSET 1

#define GROW_CAPACITY(capacity) \
    ((capacity) < CAPACITY ? CAPACITY : capacity * INC)

#define GROW_ARRAY(ar, size, type) \
    arena_realloc(ar, size, type)

#define GROW_ARENA(ar, size) \
    arena_realloc_arena(ar, size)
#define ALLOC(size) \
    alloc_ptr(size)

#define FREE_ARENA(ar) \
    arena_realloc_arena(ar, 0)

#define FREE_ARRAY(ar) \
    arena_realloc(ar, 0, ARENA_NULL)

typedef union Free Free;
typedef struct Memory Memory;

typedef long long int Align;

union Free
{
    struct
    {
        size_t size;
        Free *next;
        Free *prev;
    };
    Align align;
};

struct Memory
{
    void *glob;
    size_t current;
    size_t remains;
    Free *mem;
};

static Memory mem;

void initialize_global_memory(size_t size);
void reset_global_mem();

Arena *arena_alloc_arena(size_t size);
Arena *arena_realloc_arena(Arena *ar, size_t size);
void arena_free_arena(Arena *ar);

void *alloc_ptr(size_t size);

Arena arena_init(void *data, size_t size, T type);
Arena arena_alloc(size_t size, T type);
Arena arena_realloc(Arena *ar, size_t size, T type);

Arena Char(char ch);
Arena Int(int ival);
Arena Byte(uint8_t byte);
Arena Long(long long int llint);
Arena Double(double dval);
Arena String(const char *str);
Arena Bool(bool boolean);
Arena Null();

void arena_free(Arena *ar);
void destroy_global_memory();

void print_arena(Arena ar);

#endif
