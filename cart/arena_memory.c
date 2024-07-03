#include "arena_memory.h"

void arena_free(arena *ar)
{
    if (!ar)
        return;

    void *new = NULL;

    switch (ar->type)
    {
    case ARENA_BYTES:

        if (!ar->listof.Bytes)
            return;
        new = (void *)ar->listof.Bytes;
        ar->listof.Bytes = NULL;
        break;
    case ARENA_STR:
    case ARENA_VAR:
        if (!ar->as.String)
            return;
        new = (void *)ar->as.String;
        ar->as.String = NULL;
        break;
    case ARENA_INTS:
        if (!ar->listof.Ints)
            return;
        new = (void *)ar->listof.Ints;
        ar->listof.Ints = NULL;
        break;
    case ARENA_DOUBLES:
        if (!ar->listof.Doubles)
            return;
        new = (void *)ar->listof.Doubles;
        ar->listof.Doubles = NULL;
        break;
    case ARENA_LONGS:
        if (!ar->listof.Longs)
            return;
        new = (void *)ar->listof.Longs;
        ar->listof.Longs = NULL;
        break;
    case ARENA_STRS:
        if (!ar->listof.Strings)
            return;
        new = (void *)ar->listof.Strings;
        ar->listof.Strings = NULL;
        break;
    default:
        return;
    }

    FREE(new);
    ar = NULL;
}

arena *alloc_vector(size_t size)
{
    arena *p = NULL;
    p = ALLOC((size * sizeof(arena)) + sizeof(arena));

    p->size = size;
    p->count = 0;
    p->size = size;
    p->len = (int)size;

    return p + 1;
}
arena *realloc_vector(arena *ar, size_t size)
{

    if (!ar && size != 0)
    {
        return alloc_vector(size);
    }
    if (size == 0)
    {
        free_vector(ar);
        --ar;
        ar = NULL;
        return NULL;
    }

    arena *ptr = NULL;
    ptr = alloc_vector(size);

    size_t new_size = (size > (ar - 1)->size)
                          ? (ar - 1)->size
                          : size;

    for (size_t i = 0; i < new_size; i++)
        ptr[i] = ar[i];

    (ptr - 1)->count = (ar - 1)->count;

    FREE((ar - 1));
    --ar;
    ar = NULL;
    return ptr;
}
void free_vector(arena *ar)
{
    if (!(ar - 1))
        return;

    if ((ar - 1)->count == 0)
    {
        FREE((ar - 1));
        --ar;
        ar = NULL;
        return;
    }

    for (size_t i = 0; i < (ar - 1)->size; i++)
        switch (ar[i].type)
        {
        case ARENA_BYTES:
        case ARENA_INTS:
        case ARENA_DOUBLES:
        case ARENA_LONGS:
        case ARENA_BOOLS:
        case ARENA_STR:
        case ARENA_STRS:
        case ARENA_VAR:
            ARENA_FREE(&ar[i]);
            break;
        default:
            return;
        }

    FREE(((ar - 1)));
    --ar;
    ar = NULL;
}

static arena realloc_2d_string(arena *ar, size_t size)
{
    size_t new_size = ar->size < size ? ar->size : size;

    if (!ar->listof.Strings || ar->type == ARENA_NULL)
        return arena_init(ALLOC(size), size, ARENA_STRS);
    arena str = arena_init(ALLOC(size), size, ARENA_STRS);
    for (size_t i = 0; i < new_size; i++)
        str.listof.Strings[i] = CString(ar->listof.Strings[i]).as.String;
    str.count = ((size_t)ar->count > size) ? size : ar->count;
    return str;
}
arena arena_realloc(arena *ar, size_t size, T type)
{

    if (size == 0)
    {
        ARENA_FREE(ar);
        return Null();
    }

    if (!ar && size != 0)
        return arena_init(ALLOC(size), size, type);

    switch (type)
    {
    case ARENA_BYTES:
        ar->listof.Bytes = REALLOC(ar->listof.Bytes, ar->size, size);
        break;
    case ARENA_STR:
    case ARENA_CSTR:
    case ARENA_VAR:
        ar->as.String = REALLOC(ar->as.String, ar->size, size);
        break;
    case ARENA_INTS:
        ar->listof.Ints = REALLOC(ar->listof.Ints, ar->size, size);
        break;
    case ARENA_DOUBLES:
        ar->listof.Doubles = REALLOC(ar->listof.Doubles, ar->size, size);
        break;
    case ARENA_LONGS:
        ar->listof.Longs = REALLOC(ar->listof.Longs, ar->size, size);
        break;

    case ARENA_STRS:
        return realloc_2d_string(ar, size);

    default:
        return Null();
    }
    ar->len *= INC;
    ar->size *= INC;

    return *ar;
}
element cpy_array(element el)
{

    arena ar = el._arena;
    T type = ar.type;
    size_t size = ar.size;
    int len = ar.count;

    void *ptr = NULL;
    ptr = ALLOC(size);

    switch (type)
    {
    case ARENA_BYTES:
        if (!ar.listof.Bytes)
            return OBJ(arena_init(ptr, size, type));
        memcpy(ptr, ar.listof.Bytes, size);
        break;
    case ARENA_STR:
    case ARENA_CSTR:
    case ARENA_VAR:
        if (!ar.as.String)
            return OBJ(arena_init(ptr, size, type));
        memcpy(ptr, ar.as.String, size);
        break;
    case ARENA_INTS:
        if (!ar.listof.Ints)
            return OBJ(arena_init(ptr, size, type));
        memcpy(ptr, ar.listof.Ints, size);
        break;
    case ARENA_DOUBLES:
        if (!ar.listof.Doubles)
            return OBJ(arena_init(ptr, size, type));
        memcpy(ptr, ar.listof.Doubles, size);
        break;
    case ARENA_LONGS:
        if (!ar.listof.Doubles)
            return OBJ(arena_init(ptr, size, type));
        memcpy(ptr, ar.listof.Longs, size);
        break;

    case ARENA_STRS:
    {

        if (!ar.listof.Strings || ar.type == ARENA_NULL)
            return OBJ(arena_init(ptr, size, type));
        arena str = arena_init(ptr, size, type);
        for (int i = 0; i < len; i++)
            str.listof.Strings[i] = CString(ar.listof.Strings[i]).as.String;
    }
    break;

    default:
        return null_obj();
    }

    arena a = arena_init(ptr, size, type);
    a.count = (ar.count > a.len)
                  ? a.len
                  : ar.count;
    return OBJ(a);
}

arena Ints(int *Ints, int len)
{
    return arena_init(Ints, sizeof(int) * len, ARENA_INTS);
}
arena Doubles(double *Doubles, int len)
{
    return arena_init(Doubles, sizeof(double) * len, ARENA_DOUBLES);
}
arena Longs(long long int *Longs, int len)
{
    return arena_init(Longs, sizeof(long long int) * len, ARENA_LONGS);
}
arena Strings(void)
{
    return arena_init(NULL, 0, ARENA_STRS);
}

static void int_push(int **Ints, int index, int Int)
{
    (*Ints)[index] = Int;
}
static void double_push(double **Doubles, int index, double Double)
{
    (*Doubles)[index] = Double;
}
static void long_push(long long int **Longs, int index, long long int Long)
{
    (*Longs)[index] = Long;
}

void push_arena(element *el, arena ar)
{
    if ((el->_vector - 1)->len < (el->_vector - 1)->count + 1)
    {
        (el->_vector - 1)->len = GROW_CAPACITY((el->_vector - 1)->len);
        el->_vector = GROW_VECTOR(el->_vector, (el->_vector - 1)->len);
    }

    el->_vector[(el->_vector - 1)->count++] = ar;
}
void push_int(element *el, int Int)
{
    if (el->_arena.len < el->_arena.count + 1)
    {
        el->_arena.len = GROW_CAPACITY(el->_arena.len);

        el->_arena.size = el->_arena.len * sizeof(int);
        el->_arena = GROW_ARENA(&el->_arena, el->_arena.size, ARENA_INTS);
    }

    int_push(&el->_arena.listof.Ints, el->_arena.count++, Int);
}
void push_double(element *el, double Double)
{
    if (el->_arena.len < el->_arena.count + 1)
    {
        el->_arena.len = GROW_CAPACITY(el->_arena.len);
        el->_arena.size = el->_arena.len * sizeof(double);
        el->_arena = GROW_ARENA(&el->_arena, el->_arena.size, ARENA_DOUBLES);
    }
    double_push(&el->_arena.listof.Doubles, el->_arena.count++, Double);
}
void push_long(element *el, long long int Long)
{
    if (el->_arena.len < el->_arena.count + 1)
    {
        el->_arena.len = GROW_CAPACITY(el->_arena.len);
        el->_arena.size = el->_arena.len * sizeof(long long int);
        el->_arena = GROW_ARENA(&el->_arena, el->_arena.size, ARENA_LONGS);
    }
    long_push(&el->_arena.listof.Longs, el->_arena.count++, Long);
}
void push_string(element *el, const char *String)
{
    if (el->_arena.len < el->_arena.count + 1)
    {
        el->_arena.len = GROW_CAPACITY(el->_arena.len);
        el->_arena.size = el->_arena.len * sizeof(char *);
        el->_arena = GROW_ARENA(&el->_arena, el->_arena.size, ARENA_STRS);
    }
    el->_arena.listof.Strings[el->_arena.count++] = (char *)String;
}

element pop_arena(element *el)
{
    element tmp = OBJ(el->_vector[(el->_vector - 1)->count - 1]);
    el->_vector[--(el->_vector - 1)->count] = Null();
    return tmp;
}
element pop_int(element *el)
{

    element tmp = OBJ(Int(el->_arena.listof.Ints[el->_arena.count - 1]));
    el->_arena.listof.Ints[--el->_arena.count] = 0;
    return tmp;
}
element pop_double(element *el)
{

    element tmp = OBJ(Double(el->_arena.listof.Doubles[el->_arena.count - 1]));
    el->_arena.listof.Doubles[--el->_arena.count] = 0;
    return tmp;
}
element pop_long(element *el)
{

    element tmp = OBJ(Long(el->_arena.listof.Longs[el->_arena.count - 1]));
    el->_arena.listof.Longs[--el->_arena.count] = 0;
    return tmp;
}
element pop_string(element *el)
{

    element tmp = OBJ(CString(el->_arena.listof.Strings[el->_arena.count - 1]));
    el->_arena.listof.Strings[--el->_arena.count] = NULL;
    return tmp;
}

class *_class(arena name)
{
    class *c = NULL;
    c = ALLOC(sizeof(class));
    c->name = name;
    c->init = NULL;
    c->closures = NULL;
    return c;
}
void free_class(class *c)
{

    ARENA_FREE(&c->name);
    free_table(c->closures);
    FREE(c);
}

instance *_instance(class *classc)
{
    instance *ic = NULL;
    ic = ALLOC(sizeof(instance));
    ic->classc = classc;
    ic->fields = NULL;
    return ic;
}
void free_instance(instance *ic)
{
    FREE(ic);
    free_table(ic->fields);

    ic = NULL;
}

stack *_stack(size_t size)
{
    stack *s = NULL;
    s = ALLOC((size * sizeof(stack)) + sizeof(stack));
    s->size = size;

    (s + 1)->len = (int)size;
    (s + 1)->count = 0;
    (s + 1)->top = s + 1;
    return s + 1;
}
stack *realloc_stack(stack *st, size_t size)
{

    if (size == 0)
    {
        FREE_STACK(&st);
        --st;
        st = NULL;
        return NULL;
    }
    stack *s = NULL;
    s = NEW_STACK(size);

    if (!st)
        return s;

    size_t new_size = 0;
    if (size > (st - 1)->size)
        new_size = (st - 1)->size;
    else
        new_size = size;

    for (size_t i = 0; i < new_size; i++)
        s[i].as = st[i].as;

    s->count = st->count;
    s->top = s;
    s->top += s->count;
    FREE((st - 1));
    --st;
    st = NULL;
    return s;
}
void free_stack(stack **stack)
{
    if (!stack)
        return;
    if (!(*stack) - 1)
        return;

    if (((*stack) - 1)->count == 0)
    {
        FREE(((*stack) - 1));
        stack = NULL;
        return;
    }

    for (size_t i = 0; i < (*stack - 1)->size; i++)
        switch ((*stack)[i].as.type)
        {
        case ARENA:
            ARENA_FREE(&(*stack)[i].as._arena);
            break;
        case NATIVE:
            FREE_NATIVE((*stack)[i].as.native);
            break;
        case CLASS:
            FREE_CLASS((*stack)[i].as.classc);
            break;
        case CLOSURE:
            FREE_CLOSURE(&(*stack)[i].as.closure);
            break;
        case INSTANCE:
            FREE_INSTANCE((*stack)[i].as.instance);
            break;
        case VECTOR:
            FREE_VECTOR((*stack)[i].as._vector);
            break;
        case STACK:
            FREE_STACK(&(*stack)[i].as.stack);
            break;
        case TABLE:
            free_table((*stack)[i].as.table);
            break;
        default:
            break;
        }
    FREE(((*stack) - 1));
    stack = NULL;
}

upval **upvals(size_t size)
{
    upval **up = NULL;
    ALLOC((sizeof(upval *) * size) + sizeof(upval *));

    *up = NULL;
    *up = ALLOC(sizeof(upval));

    (*up)->size = size;

    for (size_t i = 1; i < size; i++)
        up[i] = NULL;

    return up + 1;
}
void free_upvals(upval **up)
{
    if (!up)
        return;
    if (!((*up) - 1))
        return;
    for (size_t i = 0; i < ((*up) - 1)->size; i++)
        (*up)[i].index = NULL;

    FREE(((*up) - 1));
    FREE((up - 1));
    --up;
    up = NULL;
}

function *_function(arena name)
{
    function *func = ALLOC(sizeof(function));
    func->arity = 0;
    func->upvalue_count = 0;
    func->name = name;
    init_chunk(&func->ch);

    return func;
}
void free_function(function *func)
{
    if (!func)
        return;
    if (func->name.type != ARENA_NULL)
        FREE_ARENA(&func->name);
    free_chunk(&func->ch);

    FREE(func);
    func = NULL;
}

native *_native(NativeFn func, arena ar)
{
    native *nat = NULL;
    nat = ALLOC(sizeof(native));
    nat->fn = func;
    nat->obj = ar;
    return nat;
}
void free_native(native *native)
{

    if (!native)
        return;

    ARENA_FREE(&native->obj);
    FREE(native);
    native = NULL;
}

closure *_closure(function *func)
{
    closure *clos = NULL;
    clos = ALLOC(sizeof(closure));
    clos->func = func;
    if (!func)
    {
        clos->upval_count = 0;
        return clos;
    }
    if (func->upvalue_count > 0)
        clos->upvals = upvals(func->upvalue_count);
    else
        clos->upvals = NULL;
    clos->upval_count = func->upvalue_count;

    return clos;
}
void free_closure(closure **closure)
{
    if (!(*closure))
        return;

    FREE_UPVALS((*closure)->upvals);
    FREE(closure);
    (*closure) = NULL;
    closure = NULL;
}

upval *_upval(stack *index)
{
    upval *up = NULL;
    up = ALLOC(sizeof(upval));
    up->index = index;
    up->closed = *index;
    up->next = NULL;
    return up;
}
void free_upval(upval *up)
{
    if (!up)
        return;
    up->index = NULL;
    up->next = NULL;
}

void init_chunk(chunk *c)
{
    c->lines.listof.Ints = NULL;
    c->cases.listof.Ints = NULL;
    c->op_codes.listof.Bytes = NULL;
    c->constants = NULL;

    c->lines = GROW_ARENA(NULL, STACK_SIZE * sizeof(int), ARENA_INTS);
    c->cases = GROW_ARENA(NULL, MIN_SIZE * sizeof(int), ARENA_INTS);
    c->op_codes = GROW_ARENA(NULL, MIN_SIZE * sizeof(uint8_t), ARENA_BYTES);
    c->constants = GROW_STACK(NULL, STACK_SIZE);
}
void free_chunk(chunk *c)
{
    if (!c)
    {
        init_chunk(c);
        return;
    }
    if (c->op_codes.listof.Bytes)
        FREE_ARENA(&c->op_codes);
    if (c->cases.listof.Ints)
        FREE_ARENA(&c->cases);
    if (c->lines.listof.Ints)
        FREE_ARENA(&c->lines);
    c->constants = NULL;
    init_chunk(c);
}

static void free_entry_list(table *entry)
{
    table *tmp = NULL;
    table *next = NULL;
    tmp = entry->next;
    FREE_TABLE_ENTRY(entry);

    while (tmp)
    {
        next = tmp->next;
        FREE_TABLE_ENTRY(tmp);
        tmp = next;
    }
}
void free_table(table *t)
{
    if (!(t - 1))
        return;
    if (!t)
        return;

    size_t size = (t - 1)->size;

    if ((t - 1)->count == 0)
    {
        FREE(((t - 1)));
        --t;
        t = NULL;
        return;
    }
    for (size_t i = 0; i < size; i++)
        free_entry_list(&t[i]);

    FREE(((t - 1)));
    --t;
    t = NULL;
}

void free_entry(table *entry)
{
    if (entry->type == ARENA)
        FREE_ARENA(&entry->val._arena);
    else if (entry->type == NATIVE)
        FREE_NATIVE(entry->val.native);
    else if (entry->type == CLASS)
        FREE_CLASS(entry->val.classc);
    else if (entry->type == INSTANCE)
        FREE_INSTANCE(entry->val.instance);
    else if (entry->type == TABLE)
        free_table(entry->val.table);
    else if (entry->type == VECTOR)
        FREE_VECTOR(entry->val._vector);
    else
        FREE_CLOSURE(&entry->val.closure);

    FREE_ARENA(&entry->key);

    entry = NULL;
}
