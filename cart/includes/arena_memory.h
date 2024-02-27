#ifndef _ARENA_MEMORY_H
#define _ARENA_MEMORY_H

#include <string.h>
#include "arena.h"

#define CAPACITY 50
#define INC 2
#define PAGE 16384 * 4
#define ARENA_SIZE 100
#define STACK_SIZE 250
#define TABLE_SIZE 250
#define IP_SIZE 100
#define MEM_OFFSET 1

#define GROW_CAPACITY(capacity) \
    ((capacity) < CAPACITY ? CAPACITY : CAPACITY * INC)

#define GROW_ARRAY(ar, size) \
    arena_realloc(ar, size)

#define GROW_ARENA(ar, size) \
    arena_realloc_arena(ar, size)

#define FREE_ARENA(ar) \
    arena_realloc_arena(ar, 0)

#define FREE_ARRAY(ar) \
    arena_realloc(ar, 0)

struct memory
{
    size_t max_size;
    size_t current_size;
    void *glob;
};

typedef struct memory memory;

static memory global_mem;
void initialize_global_memory(size_t size);

Arena arena_alloc_arena(size_t size);
Arena arena_realloc_arena(Arena ar, size_t size);
void arena_free_arena(Arena ar);

void *alloc_ptr(size_t size);

arena arena_init(void *data, size_t size, int type);
arena arena_alloc(size_t size, int type);
arena arena_realloc(Arena ar, size_t size);

arena Char(char ch);
arena Int(int ival);
arena Byte(uint8_t byte);
arena Long(long long int llint);
arena Double(double dval);
arena String(const char *str);
arena Bool(bool boolean);
arena Null();

void arena_free(Arena ar);
void destroy_global_memory();

void print_arena(arena ar);

#endif
