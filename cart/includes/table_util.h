#ifndef _TABLE_UTIL_H
#define _TABLE_UTIL_H

#include "arena.h"

static void insert_entry(table **t, table entry);
static table Entry(arena key, element val);
static table arena_entry(arena key, arena val);
static table class_entry(class *c);
static table instance_entry(arena ar, instance *c);
static table table_entry(arena ar, table *t);
static table func_entry(closure *c);
static table native_entry(native *func);
static table vector_entry(arena ar, arena *_vector);
static table stack_entry(arena ar, stack *s);
static table new_entry(table t);
static table null_entry(void);
static void alloc_entry(table **e, table el);

#endif
