
#include "error.h"
#include "vector.h"

void push_value(vector **v, element *obj)
{

	if ((*v)->len < (*v)->count + 1)
		*v = _realloc_vector(v, (*v)->len * INC);

	if ((*v)->type == T_GEN)
		(*v)->type = obj->type;

	else if ((*v)->type != obj->type)
		exit_error("Pushing invalid type to vector");

	*((*v)->of + (*v)->count++) = obj->val;
}
void replace_value(vector **v, int index, element *obj)
{
	if (index > (*v)->len)
		exit_error(
		    "Vector index out of range, current length: %d, provided "
		    "index: %d",
		    (*v)->len, index
		);

	if ((*v)->type == T_GEN)
		(*v)->type = obj->type;

	else if ((*v)->type != obj->type)
		exit_error(
		    "Replacing vector element at index %d with invalid type", index
		);

	*((*v)->of + index) = obj->val;
}
void insert_value(vector **v, element *obj, int index)
{
	if (index > (*v)->len)
		exit_error(
		    "Vector index out of range, current length: %d, provided "
		    "index: %d",
		    (*v)->len, index
		);

	if ((*v)->type == T_GEN)
		(*v)->type = obj->type;

	else if ((*v)->type != obj->type)
		exit_error(
		    "Inserting vector element at index %d with invalid type", index
		);

	for (int i = index + 1; i < (*v)->len - 1; i++)
		*((*v)->of + i) = *((*v)->of + i + 1);

	*((*v)->of + index) = obj->val;
}

element pop_value(vector **v)
{
	return OBJ(
	    ((*v)->count == 1) ? *(*v)->of : *((*v)->of + --(*v)->count),
	    (*v)->type
	);
}

vector *_realloc_vector(vector **v, size_t size)
{
	if (!*v && size == 0)
		return NULL;
	if (!*v && size != 0)
		return _vector(size, T_GEN);

	value *of = NULL;
	of        = ALLOC(sizeof(value) * size);

	for (size_t i = 0; i < (*v)->count; i++)
		*(of + i) = *((*v)->of + i);

	FREE((*v)->of);
	(*v)->of = NULL;
	(*v)->of = of;
	(*v)->len *= INC;

	v = NULL;

	return *v;
}
