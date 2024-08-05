
#ifndef _MEM_H
#define _MEM_H
#include "common.h"
#include <stdlib.h>

#ifndef GLOBAL_MEM_ARENA

#define ALLOC(size) malloc(size)

#define REALLOC(ptr, size) realloc(ptr, size)

#define CALLOC(val, size) calloc(val, size)

#define FREE(ptr) free(ptr)

#else

#define ALLOC(size) _malloc_(size + OFFSET)

#define REALLOC(ptr, size) _realloc_(ptr, size)

#define CALLOC(val, size) _calloc_(val, size)

#define FREE(ptr) _free_(ptr)

#endif

#define PTR(ptr) (((_free *)ptr) - 1)

#define INC    2
#define OFFSET sizeof(_free)

typedef union _free   _free;
typedef struct _mem_  _mem_;
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

struct _mem_
{
	size_t size;
	void  *glob;
};

static _free *mem = NULL;
static _mem_  arena;

void initialize_global_mem(void);
void destroy_global_memory(void);

void *_malloc_(size_t size);
void *_realloc_(void *ptr, size_t size);
void *_calloc_(int val, size_t size);
void  _free_(void *new);

#endif
