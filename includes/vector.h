#ifndef _VECTOR_H
#define _VECTOR_H
#include "object_memory.h"

void    push_value(vector **v, element *obj);
void    insert_value(vector **v, element *obj, int index);
element pop_value(vector **v);

vector *_realloc_vector(vector **v, size_t size);

#endif
