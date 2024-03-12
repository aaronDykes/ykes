#include "table.h"

void init_dict(Dict d)
{
    d->capacity = TABLE_SIZE;
    d->count = 0;
    d->map = GROW_TABLE(NULL, d->capacity);
}

void write_dict(Dict d, Arena ar1, Arena ar2, size_t size)
{
    int load_capacity = (int)(d->capacity * LOAD_FACTOR);

    if (load_capacity < d->count + 1)
    {
        d->capacity *= INC;
        d->map = GROW_TABLE(d->map, d->capacity);
    }
    ar1.as.hash %= size;
    insert_entry(&d->map, arena_entry(ar1, ar2));
    d->count++;
}

void write_func_dict(Dict d, Function *func, size_t size)
{
    int load_capacity = (int)(d->capacity * LOAD_FACTOR);

    if (load_capacity < d->count + 1)
    {
        d->capacity *= INC;
        d->map = GROW_TABLE(d->map, d->capacity);
    }
    func->name.as.hash %= size;
    insert_entry(&d->map, func_entry(func));
    d->count++;
}

void free_dict(Dict d)
{
    FREE_TABLE(d->map);
    init_dict(d);
}