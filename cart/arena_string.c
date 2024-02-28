#include "arena_memory.h"
#include <stdio.h>
#include <stdarg.h>

void log_err(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
}

void str_swap(char *from, char *to)
{
    char tmp = *from;
    *from = *to;
    *to = tmp;
}
void string_rev(char *c)
{
    char *to = c + strlen(c) - 1;
    char *from = c;

    for (; from < to; from++, to--)
        str_swap(from, to);
}

int intlen(int n)
{
    int count = 0;
    do
    {
        count++;
    } while (n /= 10);
    return count;
}
int longlen(long long int n)
{
    int count = 0;
    do
    {
        count++;
    } while (n /= 10);
    return count;
}

char *itoa(char *c, int n)
{
    int tmp = n;
    char *ch = c;

    if (n < 0)
        tmp = -n;
    do
    {
        *ch++ = tmp % 10 + '0';
    } while (tmp /= 10);

    if (n < 0)
        *ch++ = '-';
    *ch = '\0';
    string_rev(c);
    return c;
}
char *lltoa(char *c, long long int n)
{
    long long tmp = n;
    char *ch = c;

    if (n < 0)
        tmp = -n;
    do
    {
        *ch++ = tmp % 10 + '0';
    } while (tmp /= 10);

    if (n < 0)
        *ch++ = '-';
    *ch = '\0';
    string_rev(c);
    return c;
}

arena prepend_int_to_str(arena s, arena a)
{
    int len = intlen(s.as.Int);
    int ival = s.as.Int;
    arena_free(&s);
    s = arena_alloc(sizeof(char) * (len + 1 + a.length), ARENA_STR);
    s.as.String = itoa(s.as.String, ival);
    strcat(s.as.String, a.as.String);
    arena_free(&a);
    return s;
}
arena prepend_char_to_str(arena s, arena a)
{
    char c = s.as.Char;
    arena_free(&s);
    s = arena_alloc(sizeof(char) * (a.length + 1), ARENA_STR);
    s.as.String[0] = c;
    strcat(s.as.String, a.as.String);
    arena_free(&a);
    return s;
}
arena prepend_long_to_str(arena s, arena a)
{
    int len = longlen(s.as.Long);
    long long int llint = s.as.Long;
    if (llint < 0)
        len++;
    arena_free(&s);
    s = arena_alloc(sizeof(char) * (len + 1 + a.length), ARENA_STR);
    s.as.String = lltoa(s.as.String, llint);
    strcat(s.as.String, a.as.String);
    arena_free(&a);
    return s;
}

static arena append_int_to_str(arena s, arena i)
{
    int len = intlen(i.as.Int);
    int ival = i.as.Int;
    int new = len + 1 + s.length;
    if (ival < 0)
        ++len;
    arena_free(&i);
    i = arena_alloc(sizeof(char) * (len + 1), ARENA_STR);
    i.as.String = itoa(i.as.String, ival);
    s = arena_realloc(&s, new * sizeof(char));
    strcat(s.as.String, i.as.String);
    arena_free(&i);
    return s;
}
static arena append_str_to_str(arena s, arena str)
{
    int new = s.length + str.length + 1;
    s = arena_realloc(&s, new * sizeof(char));
    strcat(s.as.String, str.as.String);
    s.as.String[new] = '\0';
    arena_free(&str);
    return s;
}
static arena append_char_to_str(arena s, arena c)
{
    char ch = c.as.Char;
    arena_free(&c);
    c = arena_alloc(sizeof(char) * 2, ARENA_STR);
    c.as.String[0] = ch;
    c.as.String[1] = '\0';
    s = arena_realloc(&s, sizeof(char) * (s.length + 1));
    strcat(s.as.String, c.as.String);
    arena_free(&c);
    return s;
}
static arena append_long_to_str(arena s, arena i)
{
    int len = longlen(i.as.Long);
    long long int llint = i.as.Long;
    int new = len + 1 + s.length;

    if (llint < 0)
        ++len;

    arena_free(&i);
    i = arena_alloc(sizeof(char) * (len + 1), ARENA_STR);
    i.as.String = lltoa(i.as.String, llint);
    s = arena_realloc(&s, new * sizeof(char));
    strcat(s.as.String, i.as.String);
    arena_free(&i);
    return s;
}
arena append(arena s, arena ar)
{
    switch (ar.type)
    {
    case ARENA_STR:
        return append_str_to_str(s, ar);
    case ARENA_CHAR:
        return append_char_to_str(s, ar);
    case ARENA_INT:
        return append_int_to_str(s, ar);
    case ARENA_LONG:
        return append_long_to_str(s, ar);
        break;
    }
    return ar;
}

arena ltoa_eqcmp(long long int llint, arena ar)
{
    arena a;
    arena res;
    int len = longlen(llint);
    a = arena_alloc(sizeof(char) * len + 1, ARENA_STR);
    res = Bool(strcmp(lltoa(a.as.String, llint), ar.as.String) == 0);
    arena_free(&a);
    return res;
}
arena ltoa_neqcmp(long long int llint, arena ar)
{
    arena a;
    arena res;
    int len = longlen(llint);
    a = arena_alloc(sizeof(char) * len + 1, ARENA_STR);
    res = Bool(strcmp(lltoa(a.as.String, llint), ar.as.String) != 0);
    arena_free(&a);
    return res;
}
arena itoa_eqcmp(int ival, arena ar)
{
    arena a;
    arena res;
    int len = intlen(ival);
    a = arena_alloc(sizeof(char) * len + 1, ARENA_STR);
    res = Bool(strcmp(itoa(a.as.String, ival), ar.as.String) == 0);
    arena_free(&a);
    return res;
}
arena itoa_neqcmp(int ival, arena ar)
{
    arena a;
    arena res;
    int len = intlen(ival);
    a = arena_alloc(sizeof(char) * len + 1, ARENA_STR);
    res = Bool(strcmp(itoa(a.as.String, ival), ar.as.String) != 0);
    arena_free(&a);
    return res;
}

arena string_eq(arena s, arena c)
{

    switch (c.type)
    {
    case ARENA_NULL:
        return Bool(*s.as.String == '\0');
    case ARENA_STR:
        return Bool(strcmp(s.as.String, c.as.String) == 0);
    case ARENA_INT:
        return itoa_eqcmp(c.as.Int, s);
    case ARENA_LONG:
        return ltoa_eqcmp(c.as.Int, s);
    }
    return Bool(false);
}
arena string_ne(arena s, arena c)
{
    switch (c.type)
    {
    case ARENA_NULL:
        return Bool(*s.as.String != '\0');
    case ARENA_STR:
        return Bool(strcmp(s.as.String, c.as.String) != 0);
    case ARENA_INT:
        return itoa_eqcmp(c.as.Int, s);
    case ARENA_LONG:
        return ltoa_eqcmp(c.as.Int, s);
    }
    return Bool(true);
}
arena string_gt(arena s, arena c)
{
    if (c.type != ARENA_STR)
    {
        log_err("ERROR: string comparison type mismatch\n");
        return Bool(false);
    }
    return Bool(strcmp(s.as.String, c.as.String) > 0);
}
arena string_ge(arena s, arena c)
{
    if (c.type != ARENA_STR)
    {
        log_err("ERROR: string comparison type mismatch\n");
        return Bool(false);
    }
    return Bool(strcmp(s.as.String, c.as.String) >= 0);
}
arena string_lt(arena s, arena c)
{
    if (c.type != ARENA_STR)
    {
        log_err("ERROR: string comparison type mismatch\n");
        return Bool(false);
    }
    return Bool(strcmp(s.as.String, c.as.String) < 0);
}
arena string_le(arena s, arena c)
{
    if (c.type != ARENA_STR)
    {
        log_err("ERROR: string comparison type mismatch\n");
        return Bool(false);
    }
    return Bool(strcmp(s.as.String, c.as.String) <= 0);
}
