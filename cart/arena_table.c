#include "arena_table.h"

void insert_entry(Table **t, Table entry)
{
    Table *tmp = *t;
    Table e = tmp[entry.key.as.hash];
    Table *ptr = e.next;

    if (e.key.type == ARENA_NULL)
    {
        tmp[entry.key.as.hash] = entry;
        return;
    }

    if (e.key.as.String && (e.key.as.hash == entry.key.as.hash))
    {
        tmp[entry.key.as.hash] = new_entry(entry);
        return;
    }

    if (entry.type == NATIVE)
    {
        Native *n = find_native_entry(t, &entry.key);
        if (!n)
            ALLOC_ENTRY(&tmp[entry.key.as.hash].next, entry);
        return;
    }
    else if (entry.type == CLOSURE)
    {
        Closure *c = find_func_entry(t, &entry.key);
        if (!c)
            ALLOC_ENTRY(&tmp[entry.key.as.hash].next, entry);
        return;
    }
    else if (entry.type == ARENA)
    {
        Arena f = find_arena_entry(t, &entry.key);
        if (f.type == ARENA_NULL)
            ALLOC_ENTRY(&tmp[entry.key.as.hash].next, entry);
        return;
    }
    else if (entry.type == CLASS)
    {
        Class *c = find_class_entry(t, &entry.key);
        if (!c)
            ALLOC_ENTRY(&tmp[entry.key.as.hash].next, entry);
        return;
    }
    else if (entry.type == INSTANCE)
    {
        Instance *c = find_instance_entry(t, &entry.key);
        if (!c)
            ALLOC_ENTRY(&tmp[entry.key.as.hash].next, entry);
        return;
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
    default:
        break;
    }
}

void delete_func_entry(Table **t, Arena key)
{
    Table *a = *t;
    size_t index = key.as.hash;
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
    size_t index = key.as.hash;
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
    size_t index = hash->as.hash;
    Table entry = a[index];

    Element el = null_obj();

    if (entry.key.type == ARENA_NULL)
        return el;

    if (entry.type == ARENA)
    {
        Arena ar = find_arena_entry(t, hash);
        return ar.type == ARENA_NULL ? el : OBJ(ar);
    }
    else if (entry.type == NATIVE)
    {
        Native *n = find_native_entry(t, hash);
        return n == NULL ? el : NATIVE(n);
    }
    else if (entry.type == CLOSURE)
    {
        Closure *c = find_func_entry(t, hash);
        return c == NULL ? el : CLOSURE(c);
    }
    else if (entry.type == CLASS)
    {
        Class *c = find_class_entry(t, hash);
        return !c ? el : CLASS(c);
    }
    else if (entry.type == INSTANCE)
    {
        Instance *c = find_instance_entry(t, hash);
        return !c ? el : INSTANCE(c);
    }
    return el;
}

Native *find_native_entry(Table **t, Arena *hash)
{
    Table *a = *t;
    size_t index = hash->as.hash;
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
    size_t index = hash->as.hash;
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
    size_t index = hash->as.hash;
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
    size_t index = hash->as.hash;
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
        case ARENA_STR:
            if (tmp->key.as.hash == hash->as.hash)
                return tmp->val.closure;
            break;
        default:
            break;
        }

    return NULL;
}

Arena find_arena_entry(Table **t, Arena *hash)
{
    Table *a = *t;
    size_t index = hash->as.hash;
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
