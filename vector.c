
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
void push_2d_vector(_3d_vector **v, element *obj)
{
	if ((*v)->len < (*v)->count + 1)
		*v = _realloc_3d_vector(v, (*v)->len * INC);

	if ((*v)->type == T_GEN)
		(*v)->type = _2D_VECTOR((*obj))->type;

	else if ((*v)->type != _2D_VECTOR((*obj))->type)
		exit_error("Pushing invalid vector type");

	*((*v)->of + (*v)->count++) = _2D_VECTOR((*obj));
}

void push_obj(element **vect, element *obj)
{

	vector     *v  = NULL;
	_2d_vector *v2 = NULL;

	switch ((*vect)->type)
	{
	case T_VECTOR:
		v = VECTOR((**vect));
		push_value(&v, obj);
		break;
	case T_VECTOR_2D:
		v2 = _2D_VECTOR((**vect));
		push_vector(&v2, obj);
		break;
	default:
		error("Pushing to invalid object type");
		exit(1);
	}
}

static void insert_value(vector **v, element *obj, int index)
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

	tmp = ALLOC(size * sizeof(value));

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
static void insert_vector(_2d_vector **v, vector *obj, int index)
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

	int      size = (*v)->len + 1;
	vector **tmp  = NULL;

	tmp = ALLOC(size * sizeof(vector));

	for (int i = 0; i < index; i++)
		*(tmp + i) = *((*v)->of + i);

	*(tmp + index) = obj;

	for (int i = index; i < (*v)->len; i++)
		*(tmp + i + 1) = *((*v)->of + i);

	FREE((*v)->of);
	(*v)->of = NULL;
	(*v)->of = tmp;
	(*v)->count++;
	(*v)->len++;
}
static void insert_char(value **v, char Char, int index)
{
	if (index > (*v)->len)
		exit_error(
		    "Vector index out of range, current length: %d, provided "
		    "index: %d",
		    (*v)->len, index
		);

	int   size = (*v)->len + 1;
	char *tmp  = NULL;

	tmp = ALLOC(size);

	for (int i = 0; i < index; i++)
		*(tmp + i) = *((*v)->String + i);

	*(tmp + index) = Char;

	for (int i = index; i < (*v)->len; i++)
		*(tmp + i + 1) = *((*v)->String + i);

	FREE((*v)->String);
	(*v)->String = NULL;
	(*v)->String = tmp;
	(*v)->len++;
}

void _insert(element **vect, element *obj, int index)
{
	vector     *v  = NULL;
	_2d_vector *v2 = NULL;
	value      *va = NULL;

	switch ((*vect)->type)
	{
	case T_VECTOR:
		v = VECTOR((**vect));
		insert_value(&v, obj, index);
		break;
	case T_VECTOR_2D:
		v2 = _2D_VECTOR((**vect));
		insert_vector(&v2, VECTOR((*obj)), index);
		break;
	case T_STR:
		va = &(*vect)->val;
		insert_char(&va, obj->val.Char, index);
		break;
	default:
		error("Invalid data structure insertion");
		exit(1);
	}
}

static void delete_value_index(vector **v, Long index)
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

static void delete_vector_index(_2d_vector **v, Long index)
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
static void delete_string_index(value **String, Long index)
{
	if (index > (*String)->len)
		exit_error(
		    "Vector index out of range, current length: %d, provided "
		    "index: %d",
		    (*String)->len, index
		);

	for (int i = index; i < (*String)->len - 1; i++)
		*((*String)->String + i) = *((*String)->String + i + 1);
}

void delete_index(element **obj, Long index)
{

	vector     *v  = NULL;
	_2d_vector *v2 = NULL;
	value      *av = NULL;

	switch ((*obj)->type)
	{
	case T_VECTOR:
		v = VECTOR((**obj));
		delete_value_index(&v, index);
		break;
	case T_VECTOR_2D:
		v2 = _2D_VECTOR((**obj));
		delete_vector_index(&v2, index);
		break;
	case T_STR:
		av = &(*obj)->val;
		delete_string_index(&av, index);
		break;
	default:
		error("Attempting to access invalid object");
		exit(1);
	}
}

static void replace_value_index(value **of, int index, value value)
{
	*((*of) + index) = value;
}
static void replace_vector_index(vector ***of, int index, vector *vector)
{
	*((*of) + index) = NULL;
	*((*of) + index) = vector;
}
static void replace_string_index(char **String, int index, char Char)
{
	*((*String) + index) = Char;
}

static void set_vector_index(int index, element *obj, vector **v)
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
static void set_2d_vector_index(int index, vector *obj, _2d_vector **v)
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

	replace_vector_index(&(*v)->of, index, obj);
}

static void set_string_index(int index, char Char, value **v)
{
	if (index > (*v)->len)
		exit_error(
		    "Vector index out of range, current length: %d, provided "
		    "index: %d",
		    (*v)->len, index
		);

	replace_string_index(&(*v)->String, index, Char);
}

void _set_index(int index, element *obj, element **vect)
{
	vector     *v  = NULL;
	_2d_vector *v2 = NULL;
	value      *av = NULL;

	switch ((*vect)->type)
	{
	case T_VECTOR:
		v = VECTOR((**vect));
		set_vector_index(index, obj, &v);
		break;
	case T_VECTOR_2D:
		v2 = _2D_VECTOR((**vect));
		set_2d_vector_index(index, VECTOR((*obj)), &v2);
		break;
	case T_STR:
		av = &(*vect)->val;
		set_string_index(index, obj->val.Char, &av);
		break;
	default:
		error("Attempting to access invalid object");
		exit(1);
	}
}
static element get_vector_index(int index, vector *v)
{

	if (index > v->len)
	{
		error("Array index: %d, out of bounds", index);
		return Null();
	}

	return OBJ(*(v->of + index), v->type);
}
static element get_2d_vector_index(int index, _2d_vector *v)
{
	if (index > v->len)
	{
		error("Array index: %d, out of bounds", index);
		return Null();
	}

	return GEN(*(v->of + index), T_VECTOR);
}

static element get_string_index(int index, value v)
{
	if (index > v.len)
	{
		error("String index: %d, out of bounds", index);
		return Null();
	}
	return Char(*(v.String + index));
}

element _get_index(int index, element *obj)
{
	switch (obj->type)
	{
	case T_VECTOR:
		return get_vector_index(index, VECTOR((*obj)));
	case T_VECTOR_2D:
		return get_2d_vector_index(index, _2D_VECTOR((*obj)));
	case T_STR:
		return get_string_index(index, obj->val);
	default:
		error("Attempting to access invalid object");
		return Null();
	}
}

static element pop_value(vector **v)
{
	return ((*v)->count == 0) ? Null()
	                          : OBJ(*((*v)->of + --(*v)->count), (*v)->type);
}
static element pop_vector(_2d_vector **v)
{
	return ((*v)->count == 0) ? Null()
	                          : GEN(*((*v)->of + --(*v)->count), T_VECTOR);
}

element pop_obj(element **vect)
{
	vector     *v  = NULL;
	_2d_vector *v2 = NULL;

	switch ((*vect)->type)
	{
	case T_VECTOR:
		v = VECTOR((**vect));
		return pop_value(&v);
	case T_VECTOR_2D:
		v2 = _2D_VECTOR((**vect));
		return pop_vector(&v2);
	default:
		error("Unable to pop value from invalid object");
		exit(1);
	}
}

vector *_realloc_vector(vector **v, size_t size)
{
	if (!*v && size == 0)
		return NULL;
	if (!*v && size != 0)
		return _vector(size, T_GEN);

	value *of = NULL;
	of        = ALLOC(sizeof(value) * size);

	for (int i = 0; i < (*v)->count; i++)
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

	for (int i = 0; i < (*v)->count; i++)
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

	_2d_vector **of = ALLOC(sizeof(_2d_vector *) * size);

	for (int i = 0; i < (*v)->count; i++)
		*(of + i) = *((*v)->of + i);

	FREE((*v)->of);
	(*v)->of = NULL;
	(*v)->of = of;
	(*v)->len *= INC;

	v = NULL;

	return *v;
}
