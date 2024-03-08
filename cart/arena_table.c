#include "arena_table.h"

Table arena_alloc_table(size_t size)
{
    Table t = alloc_ptr(((size_t)size * sizeof(table)) + sizeof(table));

    size_t n = (size_t)size + 1;
    for (size_t i = 1; i < n; i++)
        t[i] = arena_entry(Null(), Null());

    t->size = size;
    return t + 1;
}

Table arena_realloc_table(Table t, size_t size)
{

    Table ptr = NULL;

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

    ptr = arena_alloc_table(size);

    size_t new_size = (size <= (t - 1)->size) ? size : (t - 1)->size;

    for (size_t i = 0; i < new_size; i++)
        ptr[i] = new_entry(t[i]);

    arena_free_table(t);
    return ptr;
}
void arena_free_table(Table t)
{
    if (!(t - 1))
        return;

    size_t size = (t - 1)->size;

    for (size_t i = 0; i < size; i++)
    {
        FREE_TABLE_ENTRY(&t[i]);
        t[i].size = 0;
    }

    (t - 1)->size = 0;
    --t;
    t = NULL;
}

void arena_free_entry(Table entry)
{
    if (entry->type == ARENA_TABLE)
        FREE_ARRAY(&entry->val.a);
    else if (entry->type == ARENA_NATIVE)
        FREE_NATIVE(entry->val.n);
    else
        FREE_FUNCTION(entry->val.f);

    FREE_ARRAY(&entry->key);
    entry->next = NULL;
    entry->prev = NULL;
    entry = NULL;
}

void insert_entry(Table *t, table entry)
{
    Table tmp = *t;
    table e = tmp[entry.key.as.hash];
    Table ptr = e.next;

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

    if (entry.type == NATIVE_TABLE)
    {
        Native *n = find_native_entry(t, &entry.key);
        if (!n)
            ALLOC_ENTRY(&tmp[entry.key.as.hash].next, entry);
        return;
    }

    else if (entry.type == FUNC_TABLE)
    {
        Function *f = find_func_entry(t, &entry.key);
        if (!f)
            ALLOC_ENTRY(&tmp[entry.key.as.hash].next, entry);
        return;
    }
    else if (entry.type == ARENA_TABLE)
    {
        arena f = find_arena_entry(t, &entry.key);
        if (f.type == ARENA_NULL)
            ALLOC_ENTRY(&tmp[entry.key.as.hash].next, entry);
    }
    else

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
                if (ptr->key.as.Int == entry.key.as.Int)
                    goto END;
                break;
            case ARENA_DOUBLE:
                if (ptr->key.as.Double == entry.key.as.Double)
                    goto END;
                break;
            case ARENA_CHAR:
                if (ptr->key.as.Char == entry.key.as.Char)
                    goto END;
                break;
            }
    return;
END:
    ptr->val = entry.val;
}
void delete_func_entry(Table *t, arena key)
{
    Table a = *t;
    size_t index = key.as.hash;
    table e = a[index];

    if (e.key.type == ARENA_NULL || key.type == ARENA_NULL)
        return;

    if (e.next && (strcmp(e.key.as.String, key.as.String) == 0))
    {
        Table t = a[index].next;
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

    Table tmp = e.next;
    Table del = NULL;

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
            if (tmp->key.as.Int == key.as.Int)
                goto DEL;
            break;
        case ARENA_DOUBLE:
            if (tmp->key.as.Double == key.as.Double)
                goto DEL;
            break;
        case ARENA_CHAR:
            if (tmp->key.as.Char == key.as.Char)
                goto DEL;
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
        if (tmp->key.as.Int == key.as.Int)
            goto DEL_LAST;
        break;
    case ARENA_DOUBLE:
        if (tmp->key.as.Double == key.as.Double)
            goto DEL_LAST;
        break;
    case ARENA_CHAR:
        if (tmp->key.as.Char == key.as.Char)
            goto DEL_LAST;
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

void delete_arena_entry(Table *t, arena key)
{
    Table a = *t;
    size_t index = key.as.hash;
    table e = a[index];

    if (e.key.type == ARENA_NULL || key.type == ARENA_NULL)
        return;

    if (e.next && (strcmp(e.key.as.String, key.as.String) == 0))
    {
        Table t = a[index].next;
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

    Table tmp = e.next;
    Table del = NULL;

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
            if (tmp->key.as.Int == key.as.Int)
                goto DEL;
            break;
        case ARENA_DOUBLE:
            if (tmp->key.as.Double == key.as.Double)
                goto DEL;
            break;
        case ARENA_CHAR:
            if (tmp->key.as.Char == key.as.Char)
                goto DEL;
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
        if (tmp->key.as.Int == key.as.Int)
            goto DEL_LAST;
        break;
    case ARENA_DOUBLE:
        if (tmp->key.as.Double == key.as.Double)
            goto DEL_LAST;
        break;
    case ARENA_CHAR:
        if (tmp->key.as.Char == key.as.Char)
            goto DEL_LAST;
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

Native *find_native_entry(Table *t, arena *hash)
{
    Table a = *t;
    size_t index = hash->as.hash;
    table entry = a[index];

    if (entry.key.type == ARENA_NULL)
        return NULL;

    if (strcmp(entry.key.as.String, hash->as.String) == 0)
        return entry.val.n;

    Table tmp = entry.next;

    for (; tmp; tmp = tmp->next)
        switch (tmp->key.type)
        {
        case ARENA_NATIVE:
        case ARENA_STR:
            if (strcmp(tmp->key.as.String, hash->as.String) == 0)
                return tmp->val.n;
            break;
        case ARENA_INT:
            if (tmp->key.as.Int == hash->as.Int)
                return tmp->val.n;
            break;
        case ARENA_DOUBLE:
            if (tmp->key.as.Double == hash->as.Double)
                return tmp->val.n;
            break;
        case ARENA_CHAR:
            if (tmp->key.as.Char == hash->as.Char)
                return tmp->val.n;
            break;
        }

    return NULL;
}

Function *find_func_entry(Table *t, arena *hash)
{
    Table a = *t;
    size_t index = hash->as.hash;
    table entry = a[index];

    if (entry.key.type == ARENA_NULL)
        return NULL;

    if (strcmp(entry.key.as.String, hash->as.String) == 0)
        return entry.val.f;

    Table tmp = entry.next;

    for (; tmp; tmp = tmp->next)
        switch (tmp->key.type)
        {
        case ARENA_VAR:
        case ARENA_FUNC:
        case ARENA_STR:
            if (strcmp(tmp->key.as.String, hash->as.String) == 0)
                return tmp->val.f;
            break;
        case ARENA_INT:
            if (tmp->key.as.Int == hash->as.Int)
                return tmp->val.f;
            break;
        case ARENA_DOUBLE:
            if (tmp->key.as.Double == hash->as.Double)
                return tmp->val.f;
            break;
        case ARENA_CHAR:
            if (tmp->key.as.Char == hash->as.Char)
                return tmp->val.f;
            break;
        }

    return NULL;
}

arena find_arena_entry(Table *t, arena *hash)
{
    Table a = *t;
    size_t index = hash->as.hash;
    table entry = a[index];

    if (entry.key.type == ARENA_NULL || hash->type != ARENA_VAR)
        return Null();

    if (strcmp(entry.key.as.String, hash->as.String) == 0)
        return entry.val.a;

    Table tmp = entry.next;

    for (; tmp; tmp = tmp->next)
        switch (tmp->key.type)
        {
        case ARENA_VAR:
        case ARENA_FUNC:
        case ARENA_STR:
            if (strcmp(tmp->key.as.String, hash->as.String) == 0)
                return tmp->val.a;
            break;
        case ARENA_INT:
            if (tmp->key.as.Int == hash->as.Int)
                return tmp->val.a;
            break;
        case ARENA_DOUBLE:
            if (tmp->key.as.Double == hash->as.Double)
                return tmp->val.a;
            break;
        case ARENA_CHAR:
            if (tmp->key.as.Char == hash->as.Char)
                return tmp->val.a;
            break;
        }

    return Null();
}

void alloc_entry(Table *e, table el)
{
    Table tmp = *e;

    if (!tmp)
    {
        *e = ALLOC(sizeof(table));
        **e = el;
        return;
    }
    for (; tmp->next; tmp = tmp->next)
        ;
    tmp->next = ALLOC(sizeof(table));
    *tmp->next = el;
    tmp->next->prev = tmp;
}

table new_entry(table t)
{
    table el;
    el.key = t.key;
    el.val = t.val;
    el.next = t.next;
    el.prev = t.prev;
    el.type = t.type;
    return el;
}

table arena_entry(arena key, arena val)
{
    table el;
    el.key = key;
    el.val.a = val;
    el.next = NULL;
    el.prev = NULL;
    el.size = key.size + val.size;
    el.type = ARENA_TABLE;
    return el;
}
table func_entry(Function *func)
{
    table el;
    el.key = func->name;
    el.val.f = func;
    el.next = NULL;
    el.prev = NULL;
    el.size = el.key.size;
    el.type = FUNC_TABLE;
    return el;
}
table native_entry(Native *func)
{
    table el;
    el.key = func->obj;
    el.val.n = func;
    el.next = NULL;
    el.prev = NULL;
    el.size = el.key.size;
    el.type = NATIVE_TABLE;
    return el;
}

arena Var(const char *str)
{
    size_t size = strlen(str);
    arena ar = GROW_ARRAY(NULL, size, ARENA_VAR);
    memcpy(ar.as.String, str, size);
    ar.as.String[size] = '\0';
    size_t h = hash(ar);
    ar.as.hash = h;
    ar.type = ARENA_VAR;
    return ar;
}
arena func_name(const char *str)
{
    size_t size = strlen(str);
    arena ar = GROW_ARRAY(NULL, size, ARENA_FUNC);
    memcpy(ar.as.String, str, size);
    ar.as.String[size] = '\0';
    size_t h = hash(ar);
    ar.as.hash = h;
    ar.type = ARENA_FUNC;
    return ar;
}
arena native_name(const char *str)
{
    size_t size = strlen(str);
    arena ar = GROW_ARRAY(NULL, size, ARENA_NATIVE);
    memcpy(ar.as.String, str, size);
    ar.as.String[size] = '\0';
    size_t h = hash(ar);
    ar.as.hash = h;
    ar.type = ARENA_NATIVE;
    return ar;
}

size_t hash(arena key)
{
    size_t index = 2166136261u;

    switch (key.type)
    {
    case ARENA_VAR:
    case ARENA_FUNC:
        for (char *s = key.as.String; *s; s++)
        {
            index ^= (int)*s;
            index *= 16777619;
        }
        break;
    case ARENA_INT:
        index ^= key.as.Int;
        index = (index * 16777669);
        break;
    case ARENA_DOUBLE:
        index ^= ((int)key.as.Double);
        index = (index * 16777420);
        break;
    case ARENA_LONG:
        index ^= key.as.Long;
        index = (index * 16776969);
        break;
    case ARENA_CHAR:
        index ^= key.as.Char;
        index = (index * 16742069);
        break;
    }
    return index;
}