#ifndef _VECTOR_H
#define _VECTOR_H
#include "object_memory.h"

void push_value(vector **v, element *obj);
void push_vector(_2d_vector **v, element *obj);
void push_2d_vector(_3d_vector **v, element *obj);

void    _insert(element **v, element *obj, int index);
void    delete_index(element **v, Long index);
void    push_obj(element **v, element *obj);
element pop_obj(element **v);

void    _set_index(element *index, element *obj, element **vect);
element _get_index(element *index, element *obj);

vector     *_realloc_vector(vector **v, size_t size);
_2d_vector *_realloc_2d_vector(_2d_vector **v, size_t size);
_3d_vector *_realloc_3d_vector(_3d_vector **v, size_t size);

#endif
