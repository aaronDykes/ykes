
#ifndef _OBJECT_H
#define _OBJECT_H
#include "mem.h"
#include "object_util.h"

#define OBJ(o, type) value_obj(o, type)
#define GEN(o, type) generic_obj(o, type)

int hash_key(char *str);

element value_obj(value ar, obj_t type);
element generic_obj(void *obj, obj_t type);

element StringEl(_string *s);
element KeyEl(_key *s);
_key   *Key(const char *str, size_t size);
element KeyObj(const char *str, size_t size);

element StringCpy(const char *str, size_t size);
element String(const char *str, size_t size);
element Char(char ch);
element Num(double Num);
element NumType(double Num, obj_t type);
element Bool(bool boolean);
element Null(void);

void print(element ar);

#endif
