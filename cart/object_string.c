#include "error.h"
#include "object_memory.h"
#include "object_string.h"
#include <string.h>

static void str_swap(char *from, char *to)
{
	char tmp = *from;
	*from    = *to;
	*to      = tmp;
}

static void string_rev(char *c)
{
	char *to   = c + strlen(c) - 1;
	char *from = c;

	for (; from < to; from++, to--)
		str_swap(from, to);
}

static int longlen(long long int n)
{
	int count = 0;
	do
	{
		count++;
	} while (n /= 10);
	return count;
}

element lltoa(long long int n)
{
	long long tmp  = n;
	char     *ch   = NULL;
	char     *ptr  = NULL;
	int       size = longlen(n) + 1;
	ch             = ALLOC(size);

	ptr = ch;

	if (n < 0)
		tmp = -n;
	do
	{
		*ptr++ = tmp % 10 + '0';
	} while (tmp /= 10);

	if (n < 0)
		*ptr++ = '-';
	*ptr = '\0';
	string_rev(ch);

	return StringCpy(ch, size);
}
element char_to_str(char ch)
{

	char *tmp  = NULL;
	tmp        = ALLOC(2);
	*tmp       = ch;
	*(tmp + 1) = '\0';

	return StringCpy(tmp, 1);
}
element str_to_num(element *a)
{
	Long ll = atoll(a->val.String);
	free_obj(*a);
	return Num(ll);
}
element str_to_bool(element *a)
{
	element obj;

	if (strcmp(a->val.String, "true") == 0)
		obj = Bool(1);
	else if (strcmp(a->val.String, "false") == 0)
		obj = Bool(0);
	else
	{
		error("Invalid cast to boolean type");
		exit(1);
	}

	free_obj(*a);
	return obj;
}

element str_to_char(element *a)
{
	char ch = *a->val.String;
	free_obj(*a);
	return Char(ch);
}

static element realloc_string(value ar, size_t size)
{
	ar.String = REALLOC(ar.String, ar.len, size);
	ar.len    = size;
	return OBJ(ar, T_STR);
}
static element append_str_to_str(element *s, element *str)
{
	int new = s->val.len + str->val.len;
	*s      = realloc_string(s->val, new * sizeof(char));
	strcat(s->val.String, str->val.String);
	// s->val.String[new] = '\0';
	FREE(str->val.String);
	return *s;
}

element append(element *s, element *ar)
{
	if (s->type != ar->type || ((s->type != T_STR) && (ar->type != T_STR)))
	{
		error("Invalid string comparison operation");
		exit(1);
	}

	return append_str_to_str(s, ar);
}

element string_eq(element *s, element *c)
{

	if (s->type != c->type || ((s->type != T_STR) && (c->type != T_STR)))
	{
		error("Invalid string comparison operation");
		exit(1);
	}

	return Bool(strcmp(s->val.String, c->val.String) == 0);
}
element string_ne(element *s, element *c)
{

	if (s->type != c->type || ((s->type != T_STR) && (c->type != T_STR)))
	{
		error("Invalid string comparison operation");
		exit(1);
	}

	return Bool(strcmp(s->val.String, c->val.String) != 0);
}
element string_gt(element *s, element *c)
{

	if (s->type != c->type || ((s->type != T_STR) && (c->type != T_STR)))
	{
		error("Invalid string comparison operation");
		exit(1);
	}

	return Bool(strcmp(s->val.String, c->val.String) > 0);
}
element string_ge(element *s, element *c)
{

	if (s->type != c->type || ((s->type != T_STR) && (c->type != T_STR)))
	{
		error("Invalid string comparison operation");
		exit(1);
	}

	return Bool(strcmp(s->val.String, c->val.String) >= 0);
}
element string_lt(element *s, element *c)
{

	if (s->type != c->type || ((s->type != T_STR) && (c->type != T_STR)))
	{
		error("Invalid string comparison operation");
		exit(1);
	}

	return Bool(strcmp(s->val.String, c->val.String) < 0);
}
element string_le(element *s, element *c)
{

	if (s->type != c->type || ((s->type != T_STR) && (c->type != T_STR)))
	{
		error("Invalid string comparison operation");
		exit(1);
	}

	return Bool(strcmp(s->val.String, c->val.String) <= 0);
}

buffer _buffer(size_t size)
{
	buffer b;
	b.bytes = NULL;
	b.count = 0;
	b.len   = size;
	b.bytes = ALLOC(size);
	return b;
}
void write_buffer(buffer *buf, char byte)
{
	if (!buf->bytes)
		return;

	if (buf->count + 1 > buf->len)
	{

		buf->bytes = REALLOC(buf->bytes, buf->len, buf->len * INC);
		buf->len += MIN_SIZE;
	}

	*(buf->bytes + buf->count++) = byte;
}
void free_buffer(buffer *buf)
{
	if (!buf->bytes)
		return;
	FREE(buf->bytes);
	buf->bytes = NULL;
}
