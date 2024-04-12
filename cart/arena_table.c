#include "arena_table.h"

void insert_entry(Table **t, Table entry)
{
    Table *tmp = *t;
    size_t index = entry.key.as.hash & ((tmp - 1)->len - 1);
    Table e = tmp[index];
    Table *ptr = e.next;

    if (e.key.type == ARENA_NULL)
    {
        tmp[index] = entry;
        return;
    }

    if (e.key.as.String && (e.key.as.hash == entry.key.as.hash))
    {
        tmp[index] = new_entry(entry);
        return;
    }

    if (entry.type == NATIVE)
    {
        Native *n = find_native_entry(t, &entry.key);
        if (!n)
        {
            ALLOC_ENTRY(&tmp[index].next, entry);
            return;
        }
        n = NULL;
    }
    else if (entry.type == CLOSURE)
    {
        Closure *c = find_func_entry(t, &entry.key);
        if (!c)
        {
            ALLOC_ENTRY(&tmp[index].next, entry);
            return;
        }
        c = NULL;
    }
    else if (entry.type == ARENA)
    {
        Arena f = find_arena_entry(t, &entry.key);
        if (f.type == ARENA_NULL)
        {
            ALLOC_ENTRY(&tmp[index].next, entry);
            return;
        }
    }
    else if (entry.type == CLASS)
    {
        Class *c = find_class_entry(t, &entry.key);
        if (!c)
        {
            ALLOC_ENTRY(&tmp[index].next, entry);
            return;
        }
        c = NULL;
    }
    else if (entry.type == INSTANCE)
    {
        Instance *c = find_instance_entry(t, &entry.key);
        if (!c)
        {
            ALLOC_ENTRY(&tmp[index].next, entry);
            return;
        }
        c = NULL;
    }
    else if (entry.type == TABLE)
    {
        Table *tab = find_table_entry(t, &entry.key);
        if (!tab)
        {

            ALLOC_ENTRY(&tmp[index].next, entry);
            return;
        }
        tab = NULL;
    }
    else if (entry.type == VECTOR)
    {
        Arena *ar = find_vector_entry(t, &entry.key);
        if (!ar)
        {
            ALLOC_ENTRY(&tmp[index].next, entry);
            return;
        }
        ar = NULL;
    }
    else if (entry.type == STACK)
    {
        Stack *ar = find_stack_entry(t, &entry.key);
        if (!ar)
        {
            ALLOC_ENTRY(&tmp[index].next, entry);
            return;
        }
        ar = NULL;
    }

    for (; ptr; ptr = ptr->next)
        switch (ptr->key.type)
        {
        case ARENA_VAR:
        case ARENA_FUNC:
        case ARENA_NATIVE:
        case ARENA_STR:
            if (ptr->key.as.hash == entry.key.as.hash)
                goto END;
            break;
        default:
            break;
        }
    return;
END:
    FREE_ENTRY(ptr->val);
    ptr->val = entry.val;
}

void free_entry(Element el)
{

    switch (el.type)
    {
    case ARENA:
        ARENA_FREE(&el.arena);
        break;
    case NATIVE:
        FREE_NATIVE(el.native);
        break;
    case CLOSURE:
        FREE_CLOSURE(&el.closure);
        break;
    case CLASS:
        FREE_CLASS(el.classc);
        break;
    case INSTANCE:
        FREE_INSTANCE(el.instance);
        break;
    case VECTOR:
        FREE_ARENA(el.arena_vector);
        break;
    case TABLE:
        FREE_TABLE(el.table);
        break;
    case STACK:
        FREE_STACK(&el.stack);
        break;
    default:
        break;
    }
}

void delete_func_entry(Table **t, Arena key)
{
    Table *a = *t;
    size_t index = key.as.hash & ((a - 1)->len - 1);
    Table e = a[index];

    if (e.key.type == ARENA_NULL || key.type == ARENA_NULL)
        return;

    if (e.next && (e.key.as.hash == key.as.hash))
    {
        Table *t = a[index].next;
        FREE_TABLE_ENTRY(&a[index]);
        a[index] = func_entry(NULL);
        t->prev = NULL;
        a[index] = new_entry(*t);

        return;
    }
    if (!e.next && (e.key.as.hash == key.as.hash))
    {
        FREE_TABLE_ENTRY(&a[index]);
        a[index] = func_entry(NULL);

        return;
    }

    Table *tmp = e.next;
    Table *del = NULL;

    if (!tmp->next)
    {
        FREE_TABLE_ENTRY(tmp);
        return;
    }

    for (; tmp->next; tmp = tmp->next)
        switch (tmp->key.type)
        {
        case ARENA_VAR:
        case ARENA_FUNC:
            if (tmp->key.as.hash == key.as.hash)
                goto DEL;
            break;
        default:
            break;
        }

    switch (tmp->key.type)
    {
    case ARENA_VAR:
    case ARENA_FUNC:
        if (tmp->key.as.hash == key.as.hash)
            goto DEL_LAST;
        break;
    default:
        return;
    }
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

void delete_arena_entry(Table **t, Arena key)
{
    Table *a = *t;
    size_t index = key.as.hash & ((a - 1)->len - 1);
    Table e = a[index];

    if (e.key.type == ARENA_NULL || key.type == ARENA_NULL)
        return;

    if (e.next && (e.key.as.hash == key.as.hash))
    {
        Table *t = a[index].next;
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

    Table *tmp = e.next;
    Table *del = NULL;

    if (!tmp->next)
    {
        FREE_TABLE_ENTRY(tmp);
        return;
    }

    for (; tmp->next; tmp = tmp->next)
        switch (tmp->key.type)
        {
        case ARENA_VAR:
        case ARENA_FUNC:
            if (tmp->key.as.hash == key.as.hash)
                goto DEL;
            break;
        default:
            break;
        }

    switch (tmp->key.type)
    {
    case ARENA_VAR:
    case ARENA_FUNC:
        if (tmp->key.as.hash == key.as.hash)
            goto DEL_LAST;
        break;

    default:
        break;
    }
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

Element find_entry(Table **t, Arena *hash)
{

    Table *a = *t;
    size_t index = hash->as.hash & ((a - 1)->len - 1);
    Table entry = a[index];

    Element el = null_obj();

    if (entry.key.type == ARENA_NULL)
        return el;

    if (entry.type == ARENA)
    {
        Arena ar = find_arena_entry(t, hash);
        return ar.type == ARENA_NULL ? el : OBJ(ar);
    }
    if (entry.type == NATIVE)
    {
        Native *n = find_native_entry(t, hash);
        return n == NULL ? el : NATIVE(n);
    }
    if (entry.type == CLOSURE)
    {
        Closure *c = find_func_entry(t, hash);
        return c == NULL ? el : CLOSURE(c);
    }
    if (entry.type == CLASS)
    {
        Class *c = find_class_entry(t, hash);
        return !c ? el : CLASS(c);
    }
    if (entry.type == INSTANCE)
    {
        Instance *c = find_instance_entry(t, hash);
        return !c ? el : INSTANCE(c);
    }
    if (entry.type == TABLE)
    {
        Table *tab = find_table_entry(t, hash);
        return !tab ? el : TABLE(tab);
    }
    if (entry.type == VECTOR)
    {
        Arena *ar = find_vector_entry(t, hash);
        return !ar ? el : VECT(ar);
    }
    if (entry.type == STACK)
    {
        Stack *s = find_stack_entry(t, hash);
        return !s ? el : STK(s);
    }
    return el;
}

Native *find_native_entry(Table **t, Arena *hash)
{
    Table *a = *t;
    size_t index = hash->as.hash & ((a - 1)->len - 1);
    Table entry = a[index];

    if (entry.key.type == ARENA_NULL)
        return NULL;

    if (entry.key.as.hash == hash->as.hash)
        return entry.val.native;

    Table *tmp = entry.next;

    for (; tmp; tmp = tmp->next)
        switch (tmp->key.type)
        {
        case ARENA_NATIVE:
        case ARENA_STR:
        case ARENA_FUNC:
        case ARENA_VAR:
            if (tmp->key.as.String == hash->as.String)
                return tmp->val.native;
            break;
        default:
            break;
        }

    return NULL;
}

Class *find_class_entry(Table **t, Arena *hash)
{
    Table *a = *t;
    size_t index = hash->as.hash & ((a - 1)->len - 1);
    Table entry = a[index];

    if (entry.key.type == ARENA_NULL)
        return NULL;

    if (entry.key.as.hash == hash->as.hash)
        return entry.val.classc;

    Table *tmp = entry.next;

    for (; tmp; tmp = tmp->next)
        switch (tmp->key.type)
        {
        case ARENA_VAR:
        case ARENA_FUNC:
        case ARENA_NATIVE:
        case ARENA_CLASS:
            if (tmp->key.as.String == hash->as.String)
                return tmp->val.classc;
            break;
        default:
            break;
        }

    return NULL;
}

Instance *find_instance_entry(Table **t, Arena *hash)
{
    Table *a = *t;
    size_t index = hash->as.hash & ((a - 1)->len - 1);
    Table entry = a[index];

    if (entry.key.type == ARENA_NULL)
        return NULL;

    if (entry.key.as.hash == hash->as.hash)
        return entry.val.instance;

    Table *tmp = entry.next;

    for (; tmp; tmp = tmp->next)
        switch (tmp->key.type)
        {
        case ARENA_VAR:
        case ARENA_FUNC:
        case ARENA_NATIVE:
        case ARENA_CLASS:
            if (tmp->key.as.String == hash->as.String)
                return tmp->val.instance;
            break;
        default:
            break;
        }

    return NULL;
}

Closure *find_func_entry(Table **t, Arena *hash)
{
    Table *a = *t;
    size_t index = hash->as.hash & ((a - 1)->len - 1);
    Table entry = a[index];

    if (entry.key.type == ARENA_NULL)
        return NULL;

    if (strcmp(entry.key.as.String, hash->as.String) == 0)
        return entry.val.closure;

    Table *tmp = entry.next;

    for (; tmp; tmp = tmp->next)
        switch (tmp->key.type)
        {
        case ARENA_VAR:
        case ARENA_FUNC:
        case ARENA_NATIVE:
        case ARENA_CSTR:
        case ARENA_STR:
            if (tmp->key.as.hash == hash->as.hash)
                return tmp->val.closure;
            break;
        default:
            break;
        }

    return NULL;
}

Table *find_table_entry(Table **t, Arena *hash)
{
    Table *a = *t;
    size_t index = hash->as.hash & ((a - 1)->len - 1);
    Table entry = a[index];

    if (entry.key.type == ARENA_NULL)
        return NULL;

    if (strcmp(entry.key.as.String, hash->as.String) == 0)
        return entry.val.table;

    Table *tmp = entry.next;

    for (; tmp; tmp = tmp->next)
        switch (tmp->key.type)
        {
        case ARENA_VAR:
        case ARENA_FUNC:
        case ARENA_NATIVE:
        case ARENA_CSTR:
        case ARENA_STR:
            if (tmp->key.as.hash == hash->as.hash)
                return tmp->val.table;
            break;
        default:
            break;
        }

    return NULL;
}
Arena *find_vector_entry(Table **t, Arena *hash)
{
    Table *a = *t;
    size_t index = hash->as.hash & ((a - 1)->len - 1);
    Table entry = a[index];

    if (entry.key.type == ARENA_NULL)
        return NULL;

    if (entry.key.as.hash == hash->as.hash)
        return entry.val.arena_vector;

    Table *tmp = entry.next;

    for (; tmp; tmp = tmp->next)
        switch (tmp->key.type)
        {
        case ARENA_VAR:
        case ARENA_FUNC:
        case ARENA_STR:
        case ARENA_NATIVE:
            if (tmp->key.as.hash == hash->as.hash)
                return tmp->val.arena_vector;
            break;

        default:
            break;
        }

    return NULL;
}
Stack *find_stack_entry(Table **t, Arena *hash)
{
    Table *a = *t;
    size_t index = hash->as.hash & ((a - 1)->len - 1);
    Table entry = a[index];

    if (entry.key.type == ARENA_NULL)
        return NULL;

    if (entry.key.as.hash == hash->as.hash)
        return entry.val.stack;

    Table *tmp = entry.next;

    for (; tmp; tmp = tmp->next)
        switch (tmp->key.type)
        {
        case ARENA_VAR:
        case ARENA_FUNC:
        case ARENA_STR:
        case ARENA_NATIVE:
            if (tmp->key.as.hash == hash->as.hash)
                return tmp->val.stack;
            break;

        default:
            break;
        }

    return NULL;
}

Arena find_arena_entry(Table **t, Arena *hash)
{
    Table *a = *t;
    size_t index = hash->as.hash & ((a - 1)->len - 1);
    Table entry = a[index];

    if (entry.key.type == ARENA_NULL)
        return Null();

    if (entry.key.as.hash == hash->as.hash)
        return entry.val.arena;

    Table *tmp = entry.next;

    for (; tmp; tmp = tmp->next)
        switch (tmp->key.type)
        {
        case ARENA_VAR:
        case ARENA_FUNC:
        case ARENA_STR:
        case ARENA_NATIVE:
            if (tmp->key.as.hash == hash->as.hash)
                return tmp->val.arena;
            break;

        default:
            break;
        }

    return Null();
}

void alloc_entry(Table **e, Table el)
{
    Table *tmp = *e;

    if (!tmp)
    {
        *e = ALLOC(sizeof(Table));
        **e = el;
        return;
    }
    for (; tmp->next; tmp = tmp->next)
        ;
    tmp->next = ALLOC(sizeof(Table));
    *tmp->next = el;
    tmp->next->prev = tmp;
}

Table new_entry(Table t)
{
    Table el;
    el.key = t.key;
    switch (t.type)
    {
    case ARENA:
        el.val.arena = t.val.arena;
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
        el.val.arena_vector = t.val.arena_vector;
        break;
    default:
        break;
    }
    el.next = t.next;
    el.prev = t.prev;
    el.type = t.type;
    return el;
}

Table Entry(Arena key, Element val)
{
    switch (val.type)
    {
    case ARENA:
        return arena_entry(key, val.arena);
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
        return vector_entry(key, val.arena_vector);
    case STACK:
        return stack_entry(key, val.stack);
    default:
        return func_entry(NULL);
    }
}

Table arena_entry(Arena key, Arena val)
{
    Table el;
    el.key = key;
    el.val.arena = val;
    el.next = NULL;
    el.prev = NULL;
    el.size = key.size + val.size;
    el.type = ARENA;
    return el;
}
Table func_entry(Closure *clos)
{
    Table el;
    el.key = clos->func->name;
    el.val.closure = clos;
    el.next = NULL;
    el.prev = NULL;
    el.size = el.key.size;
    el.type = CLOSURE;
    return el;
}
Table native_entry(Native *func)
{
    Table el;
    el.key = func->obj;
    el.val.native = func;
    el.next = NULL;
    el.prev = NULL;
    el.size = el.key.size;
    el.type = NATIVE;
    return el;
}
Table class_entry(Class *c)
{
    Table el;
    el.key = c->name;
    el.val.classc = c;
    el.next = NULL;
    el.prev = NULL;
    el.size = el.key.size;
    el.type = CLASS;
    return el;
}
Table instance_entry(Arena ar, Instance *c)
{
    Table el;
    el.key = ar;
    el.val.instance = c;
    el.next = NULL;
    el.prev = NULL;
    el.size = el.key.size;
    el.type = INSTANCE;
    return el;
}

Table table_entry(Arena ar, Table *t)
{
    Table el;
    el.key = ar;
    el.val.table = t;
    el.next = NULL;
    el.prev = NULL;
    el.size = el.key.size;
    el.type = TABLE;
    return el;
}
Table vector_entry(Arena ar, Arena *arena_vector)
{
    Table el;
    el.key = ar;
    el.val.arena_vector = arena_vector;
    el.next = NULL;
    el.prev = NULL;
    el.size = el.key.size;
    el.type = VECTOR;
    return el;
}
Table stack_entry(Arena ar, Stack *s)
{
    Table el;
    el.key = ar;
    el.val.stack = s;
    el.next = NULL;
    el.prev = NULL;
    el.size = el.key.size;
    el.type = STACK;
    return el;
}

Table *arena_realloc_table(Table *t, size_t size)
{

    Table *ptr = NULL;

    if (!t && size != 0)
    {
        ptr = arena_alloc_table(size);
        return ptr;
    }
    if (size == 0)
    {
        arena_free_table(t);
        return NULL;
    }
    size_t new_size = 0;

    if (size > (t - 1)->size)
        new_size = (t - 1)->size;
    else
        new_size = size;

    ptr = arena_alloc_table(size);

    for (size_t i = 0; i < new_size; i++)
        ptr[i] = new_entry(t[i]);

    FREE(PTR(t - 1));
    return ptr;
}

void write_table(Table *t, Arena a, Element b)
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

Table *arena_alloc_table(size_t size)
{
    Table *t = ALLOC((size * sizeof(Table)) + sizeof(Table));

    size_t n = (size_t)size + 1;

    for (size_t i = 1; i < n; i++)
        t[i] = arena_entry(Null(), Null());

    t->size = n;
    t->len = (int)size;
    t->count = 0;
    return t + 1;
}