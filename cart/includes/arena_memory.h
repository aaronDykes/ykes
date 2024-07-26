#ifndef _ARENA_MEMORY_H
#define _ARENA_MEMORY_H

#include "arena.h"
#include "common.h"

#define STACK_SIZE 64
#define MIN_SIZE 8

#define GROW_CAPACITY(capacity) \
    ((capacity) < MIN_SIZE ? MIN_SIZE : capacity * INC)

#define GROW_STACK(st, size) \
    realloc_stack(st, size)
#define FREE_STACK(st) \
    realloc_stack(st, 0)

#define FREE_OBJ(el) \
    free_obj(el)

stack *_stack(size_t size);
stack *realloc_stack(stack *stack, size_t size);

upval **upvals(size_t size);
closure *_closure(function *func);
upval *_upval(element *index);
native *_native(NativeFn native, _key ar);
class *_class(_key name);
instance *_instance(class *c);

void free_table(table *t);
void free_obj(element el);

#endif
