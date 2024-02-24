#include "arena_table.h"
#include <stdio.h>
#include <stdarg.h>

static void log_err(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
}

Table arena_alloc_table(size_t size)
{
    Table t = t = alloc_ptr(size);

    t->size = size;

    for (size_t i = 1; i < size; i++)
        t[i].entry = eentry(arena_null(), arena_null());

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

    size_t new_size = (size <= (t - 1)->size) ? size : (t - 1)->size;
    ptr = arena_alloc_table(size);

    for (size_t i = 0; i < new_size; i++)
        ptr[i].entry = t[i].entry;

    return ptr;
}
void arena_free_table(Table t)
{
    if (!(t - 1))
        return;

    size_t size = (t - 1)->size;

    for (size_t i = 0; i < size; i++)
    {
        arena_free_entry(&t[i].entry);
        t[i].size = 0;
    }

    (t - 1)->size = 0;
    --t;
    t = NULL;
}

void arena_free_entry(element *entry)
{
    arena_free(&entry->key);
    arena_free(&entry->val);
    entry->next = NULL;
    entry = NULL;
}

void insert_entry(Table *t, element entry)
{
    Table tmp = *t;
    element e = tmp[entry.key.hash].entry;
    arena f;

    if (e.key.type == ARENA_NULL)
    {
        tmp[entry.key.hash].entry = entry;
        return;
    }
    if (strcmp(e.key.as.string, entry.key.as.string) == 0)
    {
        tmp[entry.key.hash].entry = entry;
        return;
    }

    f = find_entry(t, &entry.key);
    if (f.type != ARENA_NULL)
        alloc_entry(&tmp[entry.key.hash].entry.next, entry);
}
void delete_entry(Table *t, arena key)
{
    Table a = *t;
    size_t index = key.hash;
    element e = a[index].entry;

    if (key.type == ARENA_NULL)
    {
        log_err("Unable to delete element");
        return;
    }

    if (!e.next && (strcmp(e.key.as.string, key.as.string) == 0))
    {
        arena_free_entry(&a[index].entry);
        a[index].entry = eentry(arena_null(), arena_null());
        return;
    }

    element *tmp = &e;
    element *del = NULL;

    if (!tmp->next)
    {
        arena_free_entry(tmp);
        return;
    }

    for (; tmp->next; tmp = tmp->next)
        switch (tmp->key.type)
        {
        case ARENA_STR:
            if (strcmp(tmp->key.as.string, key.as.string) == 0)
                goto DEL;
            break;
        case ARENA_INT_CONST:
            if (tmp->key.as.ival == key.as.ival)
                goto DEL;
            break;
        case ARENA_DOUBLE_CONST:
            if (tmp->key.as.dval == key.as.dval)
                goto DEL;
            break;
        case ARENA_CHAR_CONST:
            if (tmp->key.as.ch == key.as.ch)
                goto DEL;
            break;
        }

    switch (tmp->key.type)
    {
    case ARENA_STR:
        if (strcmp(tmp->key.as.string, key.as.string) == 0)
            goto DEL_LAST;
        break;
    case ARENA_INT_CONST:
        if (tmp->key.as.ival == key.as.ival)
            goto DEL_LAST;
        break;
    case ARENA_DOUBLE_CONST:
        if (tmp->key.as.dval == key.as.dval)
            goto DEL_LAST;
        break;
    case ARENA_CHAR_CONST:
        if (tmp->key.as.ch == key.as.ch)
            goto DEL_LAST;
        break;
    }

    log_err("Unable to find element");
    return;
DEL:
    del = tmp->prev;
    del->next = tmp->next;
    tmp->next->prev = del;
    arena_free_entry(tmp);
    return;

DEL_LAST:
    del = tmp->prev;
    del->next = NULL;
    arena_free_entry(tmp);
}

arena find_entry(Table *t, arena *hash)
{
    Table a = *t;
    size_t index = hash->hash;
    element entry = a[index].entry;

    if (entry.key.type == ARENA_NULL)
        return arena_null();

    if (!entry.next && (strcmp(entry.key.as.string, hash->as.string) == 0))
        return entry.val;

    element *tmp = &entry;

    for (; tmp; tmp = tmp->next)
        switch (tmp->key.type)
        {
        case ARENA_STR:
            if (strcmp(tmp->key.as.string, hash->as.string) == 0)
                return tmp->val;
            break;
        case ARENA_INT_CONST:
            if (tmp->key.as.ival == hash->as.ival)
                return tmp->val;
            break;
        case ARENA_DOUBLE_CONST:
            if (tmp->key.as.dval == hash->as.dval)
                return tmp->val;
            break;
        case ARENA_CHAR_CONST:
            if (tmp->key.as.ch == hash->as.ch)
                return tmp->val;
            break;
        }

    return arena_null();
}

void alloc_entry(element **e, element el)
{
    element *tmp = *e;

    if (!tmp)
    {
        *e = alloc_ptr(sizeof(element));
        **e = el;
        return;
    }
    for (; tmp->next; tmp = tmp->next)
        ;
    tmp->next = alloc_ptr(sizeof(element));
    *tmp->next = el;
    tmp->next->prev = tmp;
}

element *alloc_entry_ptr(size_t size)
{
    element *el = alloc_ptr(size);
    el->next = NULL;
    el->prev = NULL;
    el->size = size;
    return el + 1;
}

void free_entry_ptr(element *e)
{
    if (!(e - 1))
        return;

    size_t size = (e - 1)->size;

    for (size_t i = 0; i < size; i++)
    {
        arena_free(&e->key);
        arena_free(&e->val);
    }

    (e - 1)->size = 0;
    --e;
    e = NULL;
}

element eentry(arena key, arena val)
{
    element el;
    el.key = key;
    el.val = val;
    el.next = NULL;
    el.prev = NULL;
    el.size = 0;
    return el;
}

element new_entry(arena *key, arena *val, size_t size)
{
    element el;
    el.key = *key;
    size_t h = hash(*key, size);
    el.key.hash = h;
    key->hash = h;
    el.val = *val;
    el.next = NULL;
    el.prev = NULL;
    el.size = size;
    return el;
}

arena arena_var(const char *str)
{
    size_t size = strlen(str);
    arena ar = arena_alloc(sizeof(char) * (size), ARENA_VAR);
    memcpy(ar.as.string, str, size);
    ar.as.string[size] = '\0';
    size_t h = hash(ar, TABLE_SIZE * sizeof(table));
    ar.hash = h;
    ar.type = ARENA_VAR;
    return ar;
}

size_t hash(arena key, size_t size)
{
    size_t index = 2166136261u;

    switch (key.type)
    {
    case ARENA_VAR:
        for (char *s = key.as.string; *s; s++)
        {
            index ^= (int)*s;
            index *= 16777619;
        }
        break;
    case ARENA_INT_CONST:
        index ^= key.as.ival;
        index = (index * 16777669);
        break;
    case ARENA_DOUBLE_CONST:
        index ^= ((int)key.as.dval);
        index = (index * 16777420);
        break;
    case ARENA_LLINT_CONST:
        index ^= key.as.llint;
        index = (index * 16776969);
        break;
    case ARENA_CHAR_CONST:
        index ^= key.as.ch;
        index = (index * 16742069);
        break;
    }
    return index % size;
}