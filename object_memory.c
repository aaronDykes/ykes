#include "chunk.h"
#include "object_memory.h"

static void free_entry_list(record entry);
static void free_instance(instance **ic);
static void free_stack(stack **stack);
static void free_upvals(upval ***up, uint8_t uargc);
static void free_closure(closure **closure);
static void free_upval(upval **up);
static void free_native(native **nat);
static void free_chunk(chunk *c);
static void free_function(function **func);
static void free_class(class **c);

class *_class(_key name)
{
	class *c    = NULL;
	c           = ALLOC(sizeof(class));
	c->name     = name;
	c->init     = NULL;
	c->closures = NULL;
	return c;
}

vector *_vector(size_t size, obj_t type)
{
	vector *v = NULL;
	v         = ALLOC(sizeof(vector));
	v->count  = 0;
	v->len    = size;
	v->type   = type;
	v->of     = NULL;
	v->of     = ALLOC(size * sizeof(value));
	return v;
}
_2d_vector *_2d_vector_(size_t size, obj_t type)
{
	_2d_vector *v = NULL;
	v             = ALLOC(sizeof(_2d_vector));
	v->count      = 0;
	v->len        = size;
	v->type       = type;
	v->of         = NULL;
	v->of         = ALLOC(size * sizeof(vector *));

	for (size_t i = 0; i < size; i++)
		*(v->of + i) = NULL;

	return v;
}
_3d_vector *_3d_vector_(size_t size, obj_t type)
{
	_3d_vector *v = NULL;
	v             = ALLOC(sizeof(_3d_vector));
	v->count      = 0;
	v->len        = size;
	v->type       = type;
	v->of         = NULL;
	v->of         = ALLOC(size * sizeof(vector **));

	for (size_t i = 0; i < size; i++)
		*(v->of + i) = NULL;

	return v;
}

instance *_instance(class *classc)
{
	instance *ic = NULL;
	ic           = ALLOC(sizeof(instance));
	ic->classc   = classc;
	ic->fields   = NULL;
	return ic;
}

stack *_stack(size_t size)
{
	stack *s = NULL;
	s        = ALLOC(sizeof(stack));

	s->as = NULL;
	s->as = ALLOC(sizeof(element) * size);

	s->count = 0;
	s->len   = (int)size;
	return s;
}
stack *realloc_stack(stack **st, size_t size)
{

	if (size == 0)
	{
		free_stack(st);
		st = NULL;
		return NULL;
	}
	if (!st)
		return _stack(size);

	if (size == (*st)->len)
		return *st;

	element *as = NULL;
	as          = ALLOC(sizeof(element) * size);

	for (size_t i = 0; i < (*st)->count; i++)
		*(as + i) = *((*st)->as + i);

	FREE((*st)->as);
	(*st)->as = NULL;
	(*st)->as = as;
	(*st)->len *= INC;
	return *st;
}

upval **upvals(size_t size)
{
	upval **up = NULL;
	up         = ALLOC((sizeof(upval *) * size));

	for (size_t i = 0; i < size; i++)
		up[i] = NULL;

	return up;
}
native *_native(NativeFn func, _key ar)
{
	native *nat = NULL;
	nat         = ALLOC(sizeof(native));
	nat->fn     = func;
	nat->name   = ar;
	return nat;
}
closure *_closure(function *func)
{
	closure *clos = NULL;
	clos          = ALLOC(sizeof(closure));
	clos->func    = func;
	clos->upvals  = NULL;
	if (!func)
	{
		clos->uargc = 0;
		return clos;
	}
	if (func->uargc > 0)
		clos->upvals = upvals(func->uargc);
	clos->uargc = func->uargc;

	return clos;
}
upval *_upval(element closed, uint8_t index)
{
	upval *up  = NULL;
	up         = ALLOC(sizeof(upval));
	up->index  = index;
	up->next   = NULL;
	up->closed = closed;
	return up;
}
void free_vector(vector **v)
{
	FREE((*v)->of);
	(*v)->of = NULL;
	FREE(*v);
	v = NULL;
}
static void free_entry(record *entry)
{

	free_obj(entry->val);
	FREE(entry->key.val);
}
static void free_entry_list(record entry)
{
	record *tmp  = NULL;
	record *next = NULL;
	tmp          = entry.next;
	free_entry(&entry);

	while (tmp)
	{
		next = tmp->next;
		free_entry(tmp);
		tmp = next;
	}
}
void free_table(table **t)
{
	if (!*t)
		return;

	if ((*t)->count == 0)
	{
		FREE((*t)->records);
		(*t)->records = NULL;
		FREE(*t);
		t = NULL;
		return;
	}
	if (!(*t)->records)
	{
		FREE(t);
		t = NULL;
		return;
	}

	for (size_t i = 0; i < (*t)->len; i++)
		if ((*t)->records[i].key.val)
			free_entry_list((*t)->records[i]);

	FREE((*t)->records);
	(*t)->records = NULL;
	FREE(*t);
	t = NULL;
}

static void free_instance(instance **ic)
{
	free_table(&(*ic)->fields);
	FREE(*ic);
	ic = NULL;
}
static void free_stack(stack **stack)
{
	if (!stack)
		return;
	if (!*stack)
	{
		stack = NULL;
		return;
	}

	if ((*stack)->count == 0)
	{
		FREE((*stack)->as);
		FREE((*stack));
		stack = NULL;
		return;
	}

	for (size_t i = 0; i < (*stack)->len; i++)
		FREE_OBJ(*((*stack)->as + i));

	FREE((*stack)->as);
	(*stack)->as = NULL;
	FREE(*stack);
	stack = NULL;
}
static void free_upval(upval **up)
{
	if (!*up)
		return;
	upval *tmp = NULL;

	while (*up)
	{
		tmp = (*up)->next;
		FREE(tmp);
		*up = tmp;
	}
	up  = NULL;
	tmp = NULL;
}

static void free_upvals(upval ***up, uint8_t uargc)
{
	if (!*up)
		return;
	for (size_t i = 0; i < uargc; i++)
		free_upval(*(up + i));

	FREE(*up);
	up = NULL;
}

static void free_closure(closure **closure)
{
	if (!closure)
		return;

	free_upvals(&(*closure)->upvals, (*closure)->uargc);
	free_function(&(*closure)->func);
	FREE(*closure);
	closure = NULL;
}
static void free_native(native **nat)
{
	FREE((*nat)->name.val);
	(*nat)->name.val = NULL;
	FREE(*nat);
	nat = NULL;
}

static void free_chunk(chunk *c)
{

	FREE(c->ip);
	FREE(c->lines);
	FREE(c->cases.bytes);
	free_stack(&c->constants);
	c->ip          = NULL;
	c->lines       = NULL;
	c->cases.bytes = NULL;
	c->constants   = NULL;
	c              = NULL;
}
static void free_function(function **func)
{
	if (!*func)
		return;
	FREE((*func)->name.val);
	free_chunk(&(*func)->ch);
	FREE(func);
	func = NULL;
}
static void free_class(class **c)
{

	FREE((*c)->name.val);
	(*c)->name.val = NULL;
	free_table(&(*c)->closures);
	FREE(c);
	c = NULL;
}

void free_obj(element el)
{
	switch (el.type)
	{
	case T_STR:
		FREE(el.val.String);
		el.val.String = NULL;
		break;
	case T_KEY:
		FREE(el.key.val);
		el.key.val = NULL;
		break;
	case T_NATIVE:
		free_native((native **)&el.obj);
		break;
	case T_CLASS:
		free_class((class **)&el.obj);
		break;
	case T_VECTOR:
		free_vector((vector **)&el.obj);
		break;
	case T_INSTANCE:
		free_instance((instance **)&el.obj);
		break;
	case T_UPVAL:
		free_upval((upval **)&el.obj);
		break;
	case T_METHOD:
	case T_CLOSURE:
		free_closure((closure **)&el.obj);
		break;
	case T_FUNCTION:
		free_function((function **)&el.obj);
		break;
	case T_STACK:
		free_stack((stack **)&el.obj);
		break;
		break;
	case T_TABLE:
		free_table((table **)&el.obj);
		break;
	default:
		return;
	}
}
