#include "arena_memory.h"
#include "chunk.h"

static void free_entry_list(record *entry);
static void free_instance(instance *ic);
static void free_stack(stack *stack);
static void free_upvals(upval **up);
static void free_closure(closure *closure);
static void free_upval(upval *up);
static void free_native(native *nat);
static void free_chunk(chunk *c);
static void free_function(function *func);
static void free_class(class *c);

class *_class(_key name)
{
    class *c = NULL;
    c = ALLOC(sizeof(class));
    c->name = name;
    c->init = NULL;
    c->closures = NULL;
    return c;
}

instance *_instance(class *classc)
{
    instance *ic = NULL;
    ic = ALLOC(sizeof(instance));
    ic->classc = classc;
    ic->fields = NULL;
    return ic;
}

stack *_stack(size_t size)
{
    stack *s = NULL;
    s = ALLOC(sizeof(stack));

    s->as = NULL;
    s->as = ALLOC(sizeof(element) * size);

    s->count = 0;
    s->len = (int)size;
    return s;
}
stack *realloc_stack(stack *st, size_t size)
{

    if (size == 0)
    {
        free_stack(st);
        st = NULL;
        return NULL;
    }
    stack *s = NULL;
    s = _stack(size);

    if (!st)
        return s;

    size_t new_size = (size > st->len) ? st->len : size;

    for (size_t i = 0; i < new_size; i++)
        *(s->as + i) = *(st->as + i);

    s->count = st->count;

    FREE(st->as);
    st->as = NULL;
    FREE(st);
    st = NULL;
    return s;
}

upval **upvals(size_t size)
{
    upval **up = NULL;
    ALLOC((sizeof(upval *) * size) + sizeof(upval *));

    *up = NULL;
    *up = ALLOC(sizeof(upval));

    (*up)->len = size;

    for (size_t i = 1; i < size; i++)
        up[i] = NULL;

    return up + 1;
}
native *_native(NativeFn func, _key ar)
{
    native *nat = NULL;
    nat = ALLOC(sizeof(native));
    nat->fn = func;
    nat->name = ar;
    return nat;
}
closure *_closure(function *func)
{
    closure *clos = NULL;
    clos = ALLOC(sizeof(closure));
    clos->func = func;
    if (!func)
    {
        clos->uargc = 0;
        return clos;
    }
    if (func->uargc > 0)
        clos->upvals = upvals(func->uargc);
    else
        clos->upvals = NULL;
    clos->uargc = func->uargc;

    return clos;
}
upval *_upval(element *index)
{
    upval *up = NULL;
    up = ALLOC(sizeof(upval));
    up->index = index;
    up->closed = *index;
    up->next = NULL;
    return up;
}
static void free_entry(record *entry)
{

    free_obj(entry->val);
    FREE(entry->key.val);

    entry = NULL;
}
static void free_entry_list(record *entry)
{
    record *tmp = NULL;
    record *next = NULL;
    tmp = entry->next;
    free_entry(entry);

    while (tmp)
    {
        next = tmp->next;
        free_entry(tmp);
        tmp = next;
    }
}
void free_table(table *t)
{
    if (!t)
        return;

    size_t size = t->len;

    if (t->count == 0)
    {
        FREE(t->records);
        t->records = NULL;
        FREE(t);
        t = NULL;
        return;
    }
    for (size_t i = 0; i < size; i++)
        free_entry_list(&t->records[i]);

    FREE(t->records);
    t->records = NULL;
    FREE(t);
    t = NULL;
}

static void free_instance(instance *ic)
{
    FREE(ic);
    free_table(ic->fields);
    ic = NULL;
}
static void free_stack(stack *stack)
{
    if (!stack)
        return;

    if (stack->count == 0)
    {
        FREE(stack->as);
        FREE(stack);
        stack = NULL;
        return;
    }

    for (size_t i = 0; i < stack->len; i++)
        FREE_OBJ(stack->as[i]);

    FREE(stack->as);
    stack->as = NULL;
    FREE(stack);
    stack = NULL;
}
static void free_upvals(upval **up)
{
    if (!up)
        return;
    if (!((*up) - 1))
        return;
    for (size_t i = 0; i < ((*up) - 1)->len; i++)
        (*up)[i].index = NULL;

    FREE(((*up) - 1));
    FREE((up - 1));
    --up;
    up = NULL;
}
static void free_closure(closure *closure)
{
    if (!closure)
        return;

    free_upvals(closure->upvals);
    FREE(closure);
    closure = NULL;
}
static void free_upval(upval *up)
{
    if (!up)
        return;
    up->index = NULL;
    up->next = NULL;
}
static void free_native(native *nat)
{
    FREE(nat->name.val);
    nat->name.val = NULL;
    FREE(nat);
    nat = NULL;
}

static void free_chunk(chunk *c)
{

    FREE(c->ip.bytes);
    FREE(c->lines.bytes);
    FREE(c->cases.bytes);
    free_stack(c->constants);
}
static void free_function(function *func)
{
    if (!func)
        return;
    FREE(func->name.val);
    free_chunk(&func->ch);

    FREE(func);
    func = NULL;
}
static void free_class(class *c)
{

    FREE(c->name.val);
    c->name.val = NULL;
    free_table(c->closures);
    FREE(c);
}

void free_obj(element el)
{

    switch (el.type)
    {
    case T_STR:
        FREE(el.val.String);
        el.val.String = NULL;
        break;
    case T_NATIVE:
        free_native(NATIVE(el));
        break;
    case T_CLASS:
        free_class(CLASS(el));
        break;
    case T_INSTANCE:
        free_instance(INSTANCE(el));
        break;
    case T_METHOD:
    case T_CLOSURE:
        free_closure(CLOSURE(el));
        break;
    case T_FUNCTION:
        free_function(FUNC(el));
        break;
    case T_UPVALS:
        free_upvals((upval **)el.upvals);
        break;
    case T_STACK:
    {

        stack *s = NULL;
        s = STACK(el);
        free_stack(s);
        break;
    }
    break;
    case T_TABLE:
        free_table(TABLE(el));
        break;
    default:
        return;
    }
}
