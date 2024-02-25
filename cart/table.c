#include "table.h"

void init_dict(Dict d)
{
    d->capacity = TABLE_SIZE;
    d->count = 0;
    d->map = GROW_TABLE(NULL, sizeof(table) * d->capacity);
}

void write_dict(Dict d, arena *ar1, arena *ar2)
{
    int load_capacity = (int)(d->count * LOAD_FACTOR);

    if (load_capacity > d->capacity)
    {
        d->capacity = GROW_CAPACITY(d->capacity);
        d->map = GROW_TABLE(d->map, sizeof(table) * d->capacity);
    }
    insert_entry(&d->map, entry(*ar1, *ar2));
    d->count++;
}

void free_dict(Dict d)
{
    FREE_TABLE(d->map);
    init_dict(d);
}