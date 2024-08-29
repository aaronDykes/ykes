#include "stack.h"
#include <stdio.h>

static void check_stack_size(stack **s)
{

	if (!*s || !(*s)->as)
		return;
	if ((*s)->count + 1 > (*s)->len)
		*s = GROW_STACK(s, (*s)->len * INC);
}
static void check_fstack_size(field_stack **s)
{

	if (!*s || !(*s)->fields)
		return;
	if ((*s)->count + 1 > (*s)->len)
		*s = realloc_field_stack(s);
}

element *pop(stack **s)
{

	if ((*s)->count == 0)
		return ((*s)->as);
	return ((*s)->as + --(*s)->count);
}

void popn(stack **s, int ival)
{
	(*s)->count -= ival;

	if ((*s)->count < 0)
		(*s)->count = 0;
}

void push(stack **s, element e)
{

	check_stack_size(s);

	if (!*s || !(*s)->as)
		*s = GROW_STACK(NULL, STACK_SIZE);

	*((*s)->as + (*s)->count++) = e;
}

static init_table _itable(uint8_t init, table *fields)
{
	init_table tab;
	tab.fields = fields;
	tab.init   = init;
	return tab;
}

static init_table *_itable_ptr(size_t size)
{

	init_table *ptr = NULL;
	ptr             = ALLOC(sizeof(init_table) * size);
	for (size_t i = 0; i < size; i++)
		(ptr + i)->fields = NULL;

	return ptr;
}

field_stack *_fstack(void)
{
	field_stack *f = NULL;
	f              = ALLOC(sizeof(field_stack));
	f->fields      = NULL;

	f->fields = _itable_ptr(MIN_SIZE);
	f->count  = 0;
	f->len    = MIN_SIZE;

	return f;
}
field_stack *realloc_field_stack(field_stack **f)
{
	if (!*f)
		return _fstack();

	init_table *t = NULL;

	size_t size = (*f)->len * INC;
	t           = _itable_ptr(size);

	for (size_t i = 0; i < size; i++)
		*(t + i) = *((*f)->fields + i);

	FREE((*f)->fields);
	(*f)->fields = NULL;
	(*f)->fields = t;
	(*f)->len    = size;

	return *f;
}
void free_field_stack(field_stack **f)
{
	if (!*f)
		return;

	FREE((*f)->fields);
	(*f)->fields = NULL;
	FREE(*f);
	f = NULL;
}

init_table pop_itab(field_stack **f)
{
	if ((*f)->count == 0)
		return *(*f)->fields;
	return *((*f)->fields + --(*f)->count);
}

void push_itab(field_stack **f, uint8_t init, table *field)
{
	check_fstack_size(f);

	if (!*f || !(*f)->fields)
		*f = _fstack();

	*((*f)->fields + (*f)->count++) = _itable(init, field);
}
