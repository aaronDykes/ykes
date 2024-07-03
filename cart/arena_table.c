#include "arena_table.h"
#include "table_util.h"

static void insert_entry(table **t, table entry)
{
    table *tmp = *t;
    size_t index = entry.key.as.hash & ((tmp - 1)->len - 1);
    table e = tmp[index];
    table *ptr = e.next;

    if (e.key.type == ARENA_NULL)
    {
        tmp[index] = entry;
        return;
    }

    if (e.key.as.hash == entry.key.as.hash)
    {
        FREE_ENTRY(&tmp[index]);
        tmp[index] = new_entry(entry);
        return;
    }

    element el = find_entry(t, &entry.key);

    if (el.type == NULL_OBJ)
    {
        alloc_entry(&tmp[index].next, entry);
        return;
    }

    for (; ptr; ptr = ptr->next)
        if (ptr->key.as.hash == entry.key.as.hash)
            goto END;

    return;
END:
    FREE_ENTRY(ptr);
    ptr->val = entry.val;
}

static void delete_entry(table **t, arena key)
{
    table *a = NULL;
    a = *t;
    size_t index = key.as.hash & ((a - 1)->len - 1);
    table e = a[index];

    if (e.key.type == ARENA_NULL || key.type == ARENA_NULL)
        return;

    if (e.next && (e.key.as.hash == key.as.hash))
    {
        table *t = a[index].next;
        FREE_TABLE_ENTRY(&a[index]);
        a[index] = arena_entry(Null(), Null());
        t->prev = NULL;
        a[index] = new_entry(*t);

        return;
    }
    if (!e.next && (e.key.as.hash == key.as.hash))
    {
        FREE_TABLE_ENTRY(&a[index]);
        a[index] = arena_entry(Null(), Null());
        return;
    }

    table *tmp = e.next;
    table *del = NULL;

    if (!tmp->next)
    {
        FREE_TABLE_ENTRY(tmp);
        return;
    }

    for (; tmp->next; tmp = tmp->next)
        if (tmp->key.as.hash == key.as.hash)
            goto DEL;

    if (tmp->key.as.hash == key.as.hash)
        goto DEL_LAST;

    return;
DEL:
    del = tmp->prev;
    del->next = tmp->next;
    tmp->next->prev = del;
    FREE_TABLE_ENTRY(tmp);
    return;

DEL_LAST:
    del = tmp->prev;
    del->next = NULL;
    FREE_TABLE_ENTRY(tmp);
}

element find_entry(table **t, arena *hash)
{
    table *a = *t;
    size_t index = hash->as.hash & ((a - 1)->len - 1);
    table entry = a[index];

    element null_ = null_obj();
    if (entry.key.type == ARENA_NULL)
        return null_;

    if (entry.key.as.hash == hash->as.hash)
        switch (entry.type)
        {
        case ARENA:
            return OBJ(entry.val._arena);
        case NATIVE:
            return NATIVE(entry.val.native);
        case CLOSURE:
            return CLOSURE(entry.val.closure);
        case CLASS:
            return CLASS(entry.val.classc);
        case INSTANCE:
            return INSTANCE(entry.val.instance);
        case TABLE:
            return TABLE(entry.val.table);
        case VECTOR:
            return VECT(entry.val._vector);
        case STACK:
            return STK(entry.val.stack);
        default:
            return null_;
        }

    table *tmp = entry.next;

    for (; tmp; tmp = tmp->next)
        if (tmp->key.as.hash == hash->as.hash)
            switch (entry.type)
            {
            case ARENA:
                return OBJ(entry.val._arena);
            case NATIVE:
                return NATIVE(entry.val.native);
            case CLOSURE:
                return CLOSURE(entry.val.closure);
            case CLASS:
                return CLASS(entry.val.classc);
            case INSTANCE:
                return INSTANCE(entry.val.instance);
            case TABLE:
                return TABLE(entry.val.table);
            case VECTOR:
                return VECT(entry.val._vector);
            case STACK:
                return STK(entry.val.stack);
            default:
                return null_;
            }

    return null_;
}

static void alloc_entry(table **e, table el)
{
    table *tmp = NULL;
    tmp = *e;

    if (!tmp)
    {
        *e = NULL;
        *e = ALLOC(sizeof(table));
        **e = el;
        return;
    }
    for (; tmp->next; tmp = tmp->next)
        ;
    tmp->next = NULL;
    tmp->next = ALLOC(sizeof(table));
    *tmp->next = el;
    tmp->next->prev = tmp;
}

table new_entry(table t)
{
    table el;
    el.key = t.key;
    switch (t.type)
    {
    case ARENA:
        el.val._arena = t.val._arena;
        break;
    case NATIVE:
        el.val.native = t.val.native;
        break;
    case CLOSURE:
        el.val.closure = t.val.closure;
        break;
    case TABLE:
        el.val.table = t.val.table;
        break;
    case VECTOR:
        el.val._vector = t.val._vector;
        break;
    default:
        break;
    }
    el.next = NULL;
    el.prev = NULL;
    el.next = t.next;
    el.prev = t.prev;
    el.type = t.type;
    return el;
}

table Entry(arena key, element val)
{
    switch (val.type)
    {
    case ARENA:
        return arena_entry(key, val._arena);
    case NATIVE:
        return native_entry(val.native);
    case CLOSURE:
        return func_entry(val.closure);
    case CLASS:
        return class_entry(val.classc);
    case INSTANCE:
        return instance_entry(key, val.instance);
    case TABLE:
        return table_entry(key, val.table);
    case VECTOR:
        return vector_entry(key, val._vector);
    case STACK:
        return stack_entry(key, val.stack);
    default:
        return func_entry(NULL);
    }
}

table arena_entry(arena key, arena val)
{
    table el;
    el.key = key;
    el.val._arena = val;
    el.next = NULL;
    el.prev = NULL;
    el.size = key.size + val.size;
    el.type = ARENA;
    return el;
}
static table null_entry()
{
    table el;
    el.key = Null();
    el.val = null_obj();
    el.next = NULL;
    el.prev = NULL;
    el.size = 0;
    el.type = NULL_OBJ;
    return el;
}
static table func_entry(closure *clos)
{
    table el;
    el.key = clos->func->name;
    el.val.closure = clos;
    el.next = NULL;
    el.prev = NULL;
    el.size = el.key.size;
    el.type = CLOSURE;
    return el;
}
static table native_entry(native *func)
{
    table el;
    el.key = func->obj;
    el.val.native = func;
    el.next = NULL;
    el.prev = NULL;
    el.size = el.key.size;
    el.type = NATIVE;
    return el;
}
static table class_entry(class *c)
{
    table el;
    el.key = c->name;
    el.val.classc = c;
    el.next = NULL;
    el.prev = NULL;
    el.size = el.key.size;
    el.type = CLASS;
    return el;
}
static table instance_entry(arena ar, instance *c)
{
    table el;
    el.key = ar;
    el.val.instance = c;
    el.next = NULL;
    el.prev = NULL;
    el.size = el.key.size;
    el.type = INSTANCE;
    return el;
}

static table table_entry(arena ar, table *t)
{
    table el;
    el.key = ar;
    el.val.table = t;
    el.next = NULL;
    el.prev = NULL;
    el.size = el.key.size;
    el.type = TABLE;
    return el;
}
static table vector_entry(arena ar, arena *arena_vector)
{
    table el;
    el.key = ar;
    el.val._vector = arena_vector;
    el.next = NULL;
    el.prev = NULL;
    el.size = el.key.size;
    el.type = VECTOR;
    return el;
}
static table stack_entry(arena ar, stack *s)
{
    table el;
    el.key = ar;
    el.val.stack = s;
    el.next = NULL;
    el.prev = NULL;
    el.size = el.key.size;
    el.type = STACK;
    return el;
}

table *realloc_table(table *t, size_t size)
{

    table *ptr = NULL;

    if (!t && size != 0)
    {
        ptr = alloc_table(size);
        return ptr;
    }
    if (size == 0)
    {
        free_table(t);
        --t;
        t = NULL;
        return NULL;
    }

    size_t new_size =
        (size > (t - 1)->size) ? (t - 1)->size : size;

    ptr = alloc_table(size);

    for (size_t i = 0; i < new_size; i++)
        ptr[i] = new_entry(t[i]);

    FREE(t - 1);
    --t;
    t = NULL;
    return ptr;
}

void write_table(table *t, arena a, element b)
{

    if (b.type == CLOSURE)
    {
        if (find_entry(&t, &b.closure->func->name).type != NULL_OBJ)
            goto OVERWRITE;
    }
    else if (b.type == NATIVE)
    {
        if (find_entry(&t, &b.native->obj).type != NULL_OBJ)
            goto OVERWRITE;
    }
    else if (b.type == CLASS)
    {
        if (find_entry(&t, &b.classc->name).type != NULL_OBJ)
            goto OVERWRITE;
    }
    else
    {
        if (find_entry(&t, &a).type != NULL_OBJ)
            goto OVERWRITE;
    }

    int load_capacity = (int)((t - 1)->len * LOAD_FACTOR);

    if (load_capacity < (t - 1)->count + 1)
    {
        (t - 1)->len *= INC;
        t = GROW_TABLE(t, t->len);
    }
    (t - 1)->count++;

OVERWRITE:
    insert_entry(&t, Entry(a, b));
}

table *alloc_table(size_t size)
{
    table *t = NULL;
    t = ALLOC((size * sizeof(table)) + sizeof(table));

    size_t n = (size_t)size + 1;

    for (size_t i = 1; i < n; i++)
        t[i] = null_entry();

    t->size = n;
    t->len = (int)size;
    t->count = 0;
    return t + 1;
}
