#include "table.h"

static record *alloc_entry(record *el)
{
	record *tmp = NULL;
	tmp         = ALLOC(sizeof(record));
	tmp->next   = NULL;
	tmp->key    = el->key;
	tmp->val    = el->val;
	return tmp;
}
static void insert_entry(table **t, record entry)
{
	size_t index = entry.key.hash & ((*t)->len - 1);

	if (!(*t)->records[index].key.val)
	{
		(*t)->records[index] = entry;
		return;
	}

	if ((*t)->records[index].key.hash == entry.key.hash)
	{
		FREE_OBJ((*t)->records[index].val);
		(*t)->records[index].val = entry.val;
		return;
	}

	for (record *ptr = (*t)->records[index].next; ptr; ptr = ptr->next)
		if (ptr->key.hash == entry.key.hash)
		{
			FREE_OBJ(ptr->val);
			ptr->val = entry.val;
			return;
		}

	record *ptr               = NULL;
	ptr                       = alloc_entry(&entry);
	ptr->next                 = (*t)->records[index].next;
	(*t)->records[index].next = ptr;
}

element find_entry(table **t, _key *hash)
{
	size_t index = hash->hash & ((*t)->len - 1);

	if (!(*t)->records[index].key.val)
		return Null();

	if ((*t)->records[index].key.hash == hash->hash)
		return (*t)->records[index].val;

	for (record *tmp = (*t)->records[index].next; tmp; tmp = tmp->next)
		if (tmp->key.hash == hash->hash)
			return tmp->val;

	return Null();
}

static record new_entry(record t)
{
	record el;
	el.key  = t.key;
	el.val  = t.val;
	el.next = NULL;
	el.next = t.next;
	return el;
}

static record Entry(_key key, element val)
{
	record el;
	el.key  = key;
	el.val  = val;
	el.next = NULL;
	return el;
}

table *copy_table(table *t)
{

	table *ptr = NULL;

	ptr = alloc_table(t->len);

	for (size_t i = 0; i < t->len; i++)
		if ((t->records + i)->key.val)
		{
			size_t index = (t->records + i)->key.hash & (t->len - 1);
			*(ptr->records + index) = *(t->records + i);
		}

	return ptr;
}
table *realloc_table(table *t, size_t size)
{

	table *ptr = NULL;

	if (!t && size != 0)
		return alloc_table(size);

	size_t new_size = (size > t->len) ? t->len : size;

	ptr = alloc_table(size);

	for (size_t i = 0; i < new_size; i++)
		if (t->records[i].key.val)
		{
			size_t index        = t->records[i].key.hash & (size - 1);
			ptr->records[index] = new_entry(t->records[i]);
		}

	FREE(t->records);
	t->records = NULL;
	FREE(t);
	t = NULL;
	return ptr;
}

void write_table(table *t, _key a, element b)
{

	if (find_entry(&t, &a).type != T_NULL)
		goto OVERWRITE;

	int load_capacity = (int)(t->len * LOAD_FACTOR);

	if (load_capacity < t->count + 1)
	{
		size_t size = t->len * INC;
		t           = GROW_TABLE(t, size);
	}
	t->count++;

OVERWRITE:
	insert_entry(&t, Entry(a, b));
}

table *alloc_table(size_t size)
{
	table *t = NULL;
	t        = ALLOC(sizeof(table));

	t->count = 0;
	t->len   = (int)size;

	t->records = NULL;
	t->records = ALLOC(sizeof(record) * size);

	for (int i = 0; i < t->len; i++)
		t->records[i].key.val = NULL;

	return t;
}
