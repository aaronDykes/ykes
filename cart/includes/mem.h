
#ifndef _MEM_H
#define _MEM_H
#include <stdlib.h>

#define ALLOC(size) \
    _malloc_(size + OFFSET)

#define REALLOC(ptr, old_size, size) \
    _realloc_(ptr, old_size, size)

#define CALLOC(val, size) \
    _calloc_(val, size)

#define FREE(ptr) \
    _free_(ptr)

#define PTR(ptr) \
    (((_free *)ptr) - 1)

#define INC 2
#define ARM64_PAGE 16384
#define OFFSET sizeof(_free)

typedef union _free _free;
typedef long long int Long;
union _free
{
    struct
    {
        size_t size;
        _free *next;
    };
    Long align;
};

static _free *mem;

void initialize_global_memory(void);
void destroy_global_memory(void);

void *_malloc_(size_t size);
void *_realloc_(void *ptr, size_t old_size, size_t size);
void *_calloc_(int val, size_t size);
void _free_(void *new);

#endif
