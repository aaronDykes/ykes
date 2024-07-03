
#ifndef _ARENA_H
#define _ARENA_H
#include "arena_util.h"
#include "mem.h"

stack _value(element el);
element stack_el(stack *el);
element Obj(arena ar);
element native_fn(native *native);
element closure_el(closure *clos);
element new_class(class *classc);
element new_instance(instance *ci);
element table_el(table *t);
element vector_el(arena *vect);
element null_obj(void);

arena arena_init(void *data, size_t size, T type);
arena arena_alloc(size_t size, T type);

arena Var(const char *str);
arena String(const char *str);

arena Char(char ch);
arena Int(int ival);
arena Byte(uint8_t byte);
arena Long(long long int llint);
arena Double(double dval);
arena CString(const char *str);
arena Bool(bool boolean);
arena Size(size_t Size);
arena Null(void);

void print(element ar);
void print(element ar);

#endif
