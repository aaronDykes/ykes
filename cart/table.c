#include "table.h"

void init_dict(Dict d)
{
    d->capacity = TABLE_SIZE;
    d->count = 0;
    d->map = GROW_TABLE(NULL, sizeof(table) * TABLE_SIZE);
}

void write_dict(Dict d, arena *ar1, arena *ar2)
{
    insert_entry(&d->map, eentry(*ar1, *ar2));
}

void free_dict(Dict d)
{
    FREE_TABLE(d->map);
    init_dict(d);
}