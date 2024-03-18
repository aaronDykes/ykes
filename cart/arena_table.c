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

    if (e.key.as.String && strcmp(e.key.as.String, entry.key.as.String) == 0)
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
    for (; ptr; ptr = ptr->next)
        switch (ptr->key.type)
        {
        case ARENA_VAR:
        case ARENA_FUNC:
        case ARENA_NATIVE:
        case ARENA_STR:
            if (strcmp(ptr->key.as.String, entry.key.as.String) == 0)
                goto END;
            break;
        case ARENA_INT:
        case ARENA_DOUBLE:
        case ARENA_CHAR:
        case ARENA_BYTE:
        case ARENA_LONG:
        case ARENA_BOOL:
        case ARENA_NULL:
        case ARENA_BYTES:
        case ARENA_INTS:
        case ARENA_DOUBLES:
        case ARENA_LONGS:
        case ARENA_BOOLS:
        case ARENA_STRS:
            break;
        }
    return;
END:
    ptr->val = entry.val;
}
void delete_func_entry(Table **t, Arena key)
{
    Table *a = *t;
    size_t index = key.as.hash;
    Table e = a[index];

    if (e.key.type == ARENA_NULL || key.type == ARENA_NULL)
        return;

    if (e.next && (strcmp(e.key.as.String, key.as.String) == 0))
    {
        Table *t = a[index].next;
        FREE_TABLE_ENTRY(&a[index]);
        a[index] = func_entry(NULL);
        t->prev = NULL;
        a[index] = new_entry(*t);

        return;
    }
    if (!e.next && (strcmp(e.key.as.String, key.as.String) == 0))
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
            if (strcmp(tmp->key.as.String, key.as.String) == 0)
                goto DEL;
            break;
        case ARENA_INT:
        case ARENA_DOUBLE:
        case ARENA_CHAR:
        case ARENA_BYTE:
        case ARENA_LONG:
        case ARENA_STR:
        case ARENA_BOOL:
        case ARENA_NULL:
        case ARENA_BYTES:
        case ARENA_INTS:
        case ARENA_DOUBLES:
        case ARENA_LONGS:
        case ARENA_BOOLS:
        case ARENA_STRS:
        case ARENA_NATIVE:
            break;
        }

    switch (tmp->key.type)
    {
    case ARENA_VAR:
    case ARENA_FUNC:
        if (strcmp(tmp->key.as.String, key.as.String) == 0)
            goto DEL_LAST;
        break;
    case ARENA_INT:
    case ARENA_DOUBLE:
    case ARENA_CHAR:
    case ARENA_BYTE:
    case ARENA_LONG:
    case ARENA_STR:
    case ARENA_BOOL:
    case ARENA_NULL:
    case ARENA_BYTES:
    case ARENA_INTS:
    case ARENA_DOUBLES:
    case ARENA_LONGS:
    case ARENA_BOOLS:
    case ARENA_STRS:
    case ARENA_NATIVE:
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

void delete_arena_entry(Table **t, Arena key)
{
    Table *a = *t;
    size_t index = key.as.hash;
    Table e = a[index];

    if (e.key.type == ARENA_NULL || key.type == ARENA_NULL)
        return;

    if (e.next && (strcmp(e.key.as.String, key.as.String) == 0))
    {
        Table *t = a[index].next;
        FREE_TABLE_ENTRY(&a[index]);
        a[index] = arena_entry(Null(), Null());
        t->prev = NULL;
        a[index] = new_entry(*t);

        return;
    }
    if (!e.next && (strcmp(e.key.as.String, key.as.String) == 0))
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
            if (strcmp(tmp->key.as.String, key.as.String) == 0)
                goto DEL;
            break;
        case ARENA_INT:
        case ARENA_DOUBLE:
        case ARENA_CHAR:
        case ARENA_BYTE:
        case ARENA_LONG:
        case ARENA_STR:
        case ARENA_BOOL:
        case ARENA_NULL:
        case ARENA_BYTES:
        case ARENA_INTS:
        case ARENA_DOUBLES:
        case ARENA_LONGS:
        case ARENA_BOOLS:
        case ARENA_STRS:
        case ARENA_NATIVE:
            break;
        }

    switch (tmp->key.type)
    {
    case ARENA_VAR:
    case ARENA_FUNC:
        if (strcmp(tmp->key.as.String, key.as.String) == 0)
            goto DEL_LAST;
        break;
    case ARENA_INT:
    case ARENA_DOUBLE:
    case ARENA_CHAR:
    case ARENA_BYTE:
    case ARENA_LONG:
    case ARENA_STR:
    case ARENA_BOOL:
    case ARENA_NULL:
    case ARENA_BYTES:
    case ARENA_INTS:
    case ARENA_DOUBLES:
    case ARENA_LONGS:
    case ARENA_BOOLS:
    case ARENA_STRS:
    case ARENA_NATIVE:
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
    return el;
}

Native *find_native_entry(Table **t, Arena *hash)
{
    Table *a = *t;
    size_t index = hash->as.hash;
    Table entry = a[index];

    if (entry.key.type == ARENA_NULL)
        return NULL;

    if (strcmp(entry.key.as.String, hash->as.String) == 0)
        return entry.val.n;

    Table *tmp = entry.next;

    for (; tmp; tmp = tmp->next)
        switch (tmp->key.type)
        {
        case ARENA_NATIVE:
        case ARENA_STR:
        case ARENA_FUNC:
        case ARENA_VAR:
            if (strcmp(tmp->key.as.String, hash->as.String) == 0)
                return tmp->val.n;
            break;
        case ARENA_INT:
        case ARENA_DOUBLE:
        case ARENA_CHAR:
        case ARENA_BYTE:
        case ARENA_LONG:
        case ARENA_BOOL:
        case ARENA_NULL:
        case ARENA_BYTES:
        case ARENA_INTS:
        case ARENA_DOUBLES:
        case ARENA_LONGS:
        case ARENA_BOOLS:
        case ARENA_STRS:
            break;
        }

    return NULL;
}

Closure *find_func_entry(Table **t, Arena *hash)
{
    Table *a = *t;
    size_t index = hash->as.hash;
    Table entry = a[index];
    Closure *c = NULL;

    if (entry.key.type == ARENA_NULL)
        return NULL;

    if (strcmp(entry.key.as.String, hash->as.String) == 0)
        return entry.val.c;

    Table *tmp = entry.next;

    for (; tmp; tmp = tmp->next)
        switch (tmp->key.type)
        {
        case ARENA_VAR:
        case ARENA_FUNC:
        case ARENA_STR:
            if (strcmp(tmp->key.as.String, hash->as.String) == 0)
                return tmp->val.c;
            break;
        case ARENA_INT:
        case ARENA_DOUBLE:
        case ARENA_CHAR:
        case ARENA_BYTE:
        case ARENA_LONG:
        case ARENA_BOOL:
        case ARENA_NULL:
        case ARENA_BYTES:
        case ARENA_INTS:
        case ARENA_DOUBLES:
        case ARENA_LONGS:
        case ARENA_BOOLS:
        case ARENA_STRS:
        case ARENA_NATIVE:
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

    if (strcmp(entry.key.as.String, hash->as.String) == 0)
        return entry.val.a;

    Table *tmp = entry.next;

    for (; tmp; tmp = tmp->next)
        switch (tmp->key.type)
        {
        case ARENA_VAR:
        case ARENA_FUNC:
        case ARENA_STR:
        case ARENA_NATIVE:
            if (strcmp(tmp->key.as.String, hash->as.String) == 0)
                return tmp->val.a;
            break;
        case ARENA_INT:
        case ARENA_DOUBLE:
        case ARENA_CHAR:
        case ARENA_BYTE:
        case ARENA_LONG:
        case ARENA_BOOL:
        case ARENA_NULL:
        case ARENA_BYTES:
        case ARENA_INTS:
        case ARENA_DOUBLES:
        case ARENA_LONGS:
        case ARENA_BOOLS:
        case ARENA_STRS:
            break;
        }

    return Null();
}
