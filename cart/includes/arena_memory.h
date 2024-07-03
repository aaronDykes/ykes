#ifndef _ARENA_MEMORY_H
#define _ARENA_MEMORY_H

#include <string.h>
#include "arena.h"
#include "common.h"

#define LOAD_FACTOR 0.75
#define FRAMES_MAX 500
#define STACK_SIZE 64
#define MIN_SIZE 16

#define GROW_CAPACITY(capacity) \
    ((capacity) < MIN_SIZE ? MIN_SIZE : capacity * INC)

#define GROW_ARENA(ar, size, type) \
    arena_realloc(ar, size, type)
#define FREE_ARENA(ar) \
    arena_realloc(ar, 0, ARENA_NULL)

#define GROW_VECTOR(ar, size) \
    realloc_vector(ar, size)
#define FREE_VECTOR(ar) \
    realloc_vector(ar, 0)

#define ARENA_FREE(ar) \
    arena_free(ar)

#define FREE_TABLE_ENTRY(ar) \
    free_entry(ar)

#define GROW_STACK(st, size) \
    realloc_stack(st, size)
#define FREE_STACK(st) \
    free_stack(st)

#define FREE_FUNCTION(func) \
    free_function(func)
#define FREE_NATIVE(nat) \
    free_native(nat)
#define FREE_CLOSURE(clos) \
    free_closure(clos)
#define FREE_UPVALS(up) \
    free_upvals(up)

#define FREE_UPVAL(up) \
    free_upval(up)
#define NEW_STACK(size) \
    _stack(size)

#define FREE_CLASS(c) \
    free_class(c)
#define FREE_INSTANCE(c) \
    free_instance(c)

void initialize_global_memory(void);
void destroy_global_memory(void);

arena *alloc_vector(size_t size);
arena *realloc_vector(arena *ar, size_t size);
void free_vector(arena *ar);

arena arena_realloc(arena *ar, size_t size, T type);
element cpy_array(element el);

void arena_free(arena *ar);

arena Ints(int *ints, int len);
arena Doubles(double *doubles, int len);
arena Longs(long long int *longs, int len);
arena Strings(void);

void push_arena(element *el, arena ar);
element pop_arena(element *el);

void push_int(element *el, int Int);
element pop_int(element *el);

void push_double(element *el, double Double);
element pop_double(element *el);

void push_long(element *el, long long int Long);
element pop_long(element *el);

void push_string(element *el, const char *String);
element pop_string(element *el);

void init_chunk(chunk *c);
void free_chunk(chunk *c);

stack *_stack(size_t size);
stack *realloc_stack(stack *stack, size_t size);
void free_stack(stack **stack);

upval **upvals(size_t size);
void free_upvals(upval **up);

function *_function(arena name);
void free_function(function *func);

closure *_closure(function *func);
void free_closure(closure **closure);

upval *_upval(stack *index);
void free_upval(upval *up);

native *_native(NativeFn native, arena ar);
void free_native(native *nat);

void free_table(table *t);
void free_entry(table *entry);

class *_class(arena name);
void free_class(class *c);

instance *_instance(class *c);
void free_instance(instance *ic);

#endif
