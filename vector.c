
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
void push_vector(_2d_vector **v, element *obj)
{

	if ((*v)->len < (*v)->count + 1)
		*v = _realloc_2d_vector(v, (*v)->len * INC);

	if ((*v)->type == T_GEN)
		(*v)->type = VECTOR((*obj))->type;

	else if ((*v)->type != VECTOR((*obj))->type)
		exit_error("Pushing invalid vector type");

	*((*v)->of + (*v)->count++) = VECTOR((*obj));
}
static void replace_value_index(value **of, int index, value val)
{
	*((*of) + index) = val;
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

	int    size = (*v)->len + 1;
	value *tmp  = NULL;

	tmp = ALLOC(size);

	for (int i = 0; i < index; i++)
		*(tmp + i) = *((*v)->of + i);

	*(tmp + index) = obj->val;

	for (int i = index; i < (*v)->len; i++)
		*(tmp + i + 1) = *((*v)->of + i);

	FREE((*v)->of);
	(*v)->of = NULL;
	(*v)->of = tmp;
	(*v)->count++;
	(*v)->len++;
}
void delete_index(vector **v, Long index)
{
	if (index > (*v)->len)
		exit_error(
		    "Vector index out of range, current length: %d, provided "
		    "index: %d",
		    (*v)->len, index
		);

	for (int i = index; i < (*v)->len - 1; i++)
		*((*v)->of + i) = *((*v)->of + i + 1);

	--(*v)->count;
}

void _set_index(int index, element *obj, vector **v)
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

	replace_value_index(&(*v)->of, index, obj->val);
}
element _get_index(int index, vector *v)
{
	if (!v)
		return Null();
	if (index > v->count)
	{
		error("Array index: %d, out of bounds", index);
		return Null();
	}

	return OBJ(*(v->of + index), v->type);
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
_2d_vector *_realloc_2d_vector(_2d_vector **v, size_t size)
{
	if (!*v && size == 0)
		return NULL;
	if (!*v && size != 0)
		return _2d_vector_(size, T_GEN);

	vector **of = ALLOC(sizeof(vector *) * size);

	for (size_t i = 0; i < (*v)->count; i++)
		*(of + i) = *((*v)->of + i);

	FREE((*v)->of);
	(*v)->of = NULL;
	(*v)->of = of;
	(*v)->len *= INC;

	v = NULL;

	return *v;
}
_3d_vector *_realloc_3d_vector(_3d_vector **v, size_t size)
{
	if (!*v && size == 0)
		return NULL;
	if (!*v && size != 0)
		return _3d_vector_(size, T_GEN);

	vector ***of = ALLOC(sizeof(vector **) * size);

	for (size_t i = 0; i < (*v)->count; i++)
		*(of + i) = *((*v)->of + i);

	FREE((*v)->of);
	(*v)->of = NULL;
	(*v)->of = of;
	(*v)->len *= INC;

	v = NULL;

	return *v;
}
