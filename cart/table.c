#include "table.h"

void init_dict(Dict d)
{
    d->capacity = TABLE_SIZE;
    d->count = 0;
    d->map = GROW_TABLE(NULL, d->capacity);
}

void write_dict(Dict d, arena ar1, arena ar2)
{
    int load_capacity = (int)(d->capacity * LOAD_FACTOR);

    if (load_capacity < d->count + 1)
    {
        d->capacity = GROW_CAPACITY(d->capacity);
        d->map = GROW_TABLE(d->map, d->capacity);
    }
    insert_entry(&d->map, entry(ar1, ar2));
    d->count++;
}

void free_dict(Dict d)
{
    FREE_TABLE(d->map);
    init_dict(d);
}