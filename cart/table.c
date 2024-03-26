#include "table.h"

void write_table(Table *t, Arena a, Element b)
{

    if (b.type == CLOSURE)
    {
        b.closure->func->name.as.hash %= (t - 1)->len;
        if (find_entry(&t, &b.closure->func->name).type != NULL_OBJ)
            goto OVERWRITE;
    }
    else if (b.type == NATIVE)
    {
        b.native->obj.as.hash %= (t - 1)->len;
        if (find_entry(&t, &b.native->obj).type != NULL_OBJ)
            goto OVERWRITE;
    }
    else if (b.type == CLASS)
    {
        b.classc->name.as.hash %= (t - 1)->len;
        if (find_entry(&t, &b.classc->name).type != NULL_OBJ)
            goto OVERWRITE;
    }
    else
    {
        a.as.hash %= (t - 1)->len;
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