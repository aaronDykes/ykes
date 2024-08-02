#include "object_string.h"
#include "error.h"
#include <string.h>

static void str_swap(char *from, char *to)
{
    char tmp = *from;
    *from = *to;
    *to = tmp;
}

static void string_rev(char *c)
{
    char *to = c + strlen(c) - 1;
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
    long long tmp = n;
    char *ch = NULL;
    char *ptr = NULL;
    int size = longlen(n) + 1;
    ch = ALLOC(size);

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

static element realloc_string(value ar, size_t size)
{
    ar.String = REALLOC(ar.String, size);
    ar.len = size;
    return OBJ(ar, T_STR);
}
static element append_str_to_str(element *s, element *str)
{
    int new = s->val.len + str->val.len - 1;
    *s = realloc_string(s->val, new * sizeof(char));
    strcat(s->val.String, str->val.String);
    s->val.String[new] = '\0';
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
