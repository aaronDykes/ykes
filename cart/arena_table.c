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
    Table t = alloc_ptr(size + sizeof(table));

    size_t n = ((size + sizeof(table)) / sizeof(table));

    for (size_t i = 1; i < n; i++)
        t[i] = entry(Null(), Null());

    t->size = n;
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

    size_t new_size = (size <= (t - 1)->size) ? size / sizeof(table) : (t - 1)->size;
    ptr = arena_alloc_table(size);

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
        arena_free_entry(&t[i]);
        t[i].size = 0;
    }

    (t - 1)->size = 0;
    --t;
    t = NULL;
}

void arena_free_entry(Table entry)
{
    arena_free(&entry->key);
    arena_free(&entry->val);
    entry->next = NULL;
    entry->prev = NULL;
    entry = NULL;
}

void insert_entry(Table *t, table entry)
{
    Table tmp = *t;
    table e = tmp[entry.key.hash];
    arena f;
    char *c = e.key.as.String;

    if (e.key.type == ARENA_NULL)
    {
        tmp[entry.key.hash] = entry;
        return;
    }

    if (strcmp(e.key.as.String, entry.key.as.String) == 0)
    {
        tmp[entry.key.hash] = entry;
        return;
    }

    alloc_entry(&tmp[entry.key.hash].next, entry);
}
void delete_entry(Table *t, arena key)
{
    Table a = *t;
    size_t index = key.hash;
    table e = a[index];

    if (key.type == ARENA_NULL)
    {
        log_err("Unable to delete element");
        return;
    }

    if (!e.next && (strcmp(e.key.as.String, key.as.String) == 0))
    {
        arena_free_entry(&a[index]);
        a[index] = entry(Null(), Null());
        return;
    }

    Table tmp = e.next;
    Table del = NULL;

    if (!tmp->next)
    {
        arena_free_entry(tmp);
        return;
    }

    for (; tmp->next; tmp = tmp->next)
        switch (tmp->key.type)
        {
        case ARENA_STR:
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
    case ARENA_STR:
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
    table entry = a[index];

    if (entry.key.type == ARENA_NULL)
        return Null();

    if (strcmp(entry.key.as.String, hash->as.String) == 0)
        return entry.val;

    Table tmp = entry.next;

    for (; tmp; tmp = tmp->next)
        switch (tmp->key.type)
        {
        case ARENA_VAR:
        case ARENA_STR:
            if (strcmp(tmp->key.as.String, hash->as.String) == 0)
                return tmp->val;
            break;
        case ARENA_INT:
            if (tmp->key.as.Int == hash->as.Int)
                return tmp->val;
            break;
        case ARENA_DOUBLE:
            if (tmp->key.as.Double == hash->as.Double)
                return tmp->val;
            break;
        case ARENA_CHAR:
            if (tmp->key.as.Char == hash->as.Char)
                return tmp->val;
            break;
        }

    return Null();
}

void alloc_entry(Table *e, table el)
{
    Table tmp = *e;

    if (!tmp)
    {
        *e = alloc_ptr(sizeof(table));
        **e = el;
        return;
    }
    for (; tmp->next; tmp = tmp->next)
        ;
    tmp->next = alloc_ptr(sizeof(table));
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
    el.size = t.key.size + t.val.size;
    return el;
}

table entry(arena key, arena val)
{
    table el;
    el.key = key;
    el.val = val;
    el.next = NULL;
    el.prev = NULL;
    el.size = key.size + val.size;
    return el;
}

arena Var(const char *str, size_t table_size)
{
    size_t size = strlen(str);
    arena ar = arena_alloc(size, ARENA_VAR);
    memcpy(ar.as.String, str, size);
    ar.as.String[size] = '\0';
    size_t h = hash(ar, table_size - 1);
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
    return index % size;
}