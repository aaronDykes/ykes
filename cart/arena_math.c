#include "arena_math.h"
#include "arena_string.h"
#include "error.h"
#include <math.h>

element _neg(element a)
{

    switch (a.type)
    {
    case T_NUM:
        return Num(-a.val.Num);
    case T_CHAR:
        return Char(-a.val.Char);
    case T_BOOL:
        return Bool(!a.val.Bool);
    default:
        error("Invalid type for `++` operation");
        exit(1);
    }
}

element _add(element a, element b)
{
    if (a.type != b.type)
    {
        error("Invalid comparison");
        exit(1);
    }

    switch (b.type)
    {
    case T_NUM:
        return Num(a.val.Num + b.val.Num);
    case T_CHAR:
        return Char(++a.val.Char);
    case T_STR:
        return append(b, a);
    default:
        error("Invalid type for `++` operation");
        exit(1);
    }
}
element _inc(element a)
{

    switch (a.type)
    {
    case T_NUM:
        return Num(++a.val.Num);
    case T_CHAR:
        return Char(++a.val.Char);
    default:
        error("Invalid type for `++` operation");
        exit(1);
    }
}
element _dec(element a)
{

    switch (a.type)
    {
    case T_NUM:
        return Num(--a.val.Num);
    case T_CHAR:
        return Char(--a.val.Char);
    default:
        error("Invalid type for `--` operation");
        exit(1);
    }
}
element _sub(element a, element b)
{

    if (a.type != b.type)
    {
        error("Invalid comparison");
        exit(1);
    }
    switch (b.type)
    {
    case T_NUM:
        return Num(b.val.Num - a.val.Num);
    case T_CHAR:
        return Char(b.val.Char - a.val.Char);
    default:
        error("Invalid type for `-` operation");
        exit(1);
    }
}
element _mul(element a, element b)
{

    if (a.type != b.type)
    {
        error("Invalid comparison");
        exit(1);
    }
    switch (b.type)
    {
    case T_NUM:
        return Num(b.val.Num * a.val.Num);
    case T_CHAR:
        return Num((Long)b.val.Char * a.val.Char);
    default:
        error("Invalid type for `*` operation");
        exit(1);
    }
}
element _div(element a, element b)
{

    if (a.type != b.type)
    {
        error("Invalid comparison");
        exit(1);
    }
    switch (b.type)
    {
    case T_NUM:
        return Num(b.val.Num / a.val.Num);
    case T_CHAR:
        return Num((double)b.val.Char / (double)a.val.Char);
    default:
        error("Invalid type for `/` operation");
        exit(1);
    }
}
element _mod(element a, element b)
{

    if (a.type != b.type)
    {
        error("Invalid comparison");
        exit(1);
    }
    switch (b.type)
    {
    case T_NUM:
        return Num((Long)b.val.Num % (Long)a.val.Num);
    case T_CHAR:
        return Char(b.val.Char % a.val.Char);
    default:
        error("Invalid type for `%` operation");
        exit(1);
    }
}
element _eq(element a, element b)
{

    if (a.type != b.type)
    {
        error("Invalid comparison");
        exit(1);
    }
    switch (b.type)
    {
    case T_NUM:
        return Bool(b.val.Num == a.val.Num);
    case T_CHAR:
        return Bool(b.val.Char == a.val.Char);
    case T_STR:
        return string_ne(b, a);
    default:
        error("Invalid type for `==` operation");
        exit(1);
    }
}
element _ne(element a, element b)
{

    if (a.type != b.type)
    {
        error("Invalid comparison");
        exit(1);
    }
    switch (b.type)
    {
    case T_NUM:
        return Bool(b.val.Num != a.val.Num);
    case T_CHAR:
        return Bool(b.val.Char != a.val.Char);
    case T_STR:
        return string_ne(b, a);
    default:
        error("Invalid type for `!=` operation");
        exit(1);
    }
}
element _lt(element a, element b)
{

    if (a.type != b.type)
    {
        error("Invalid comparison");
        exit(1);
    }
    switch (b.type)
    {
    case T_NUM:
        return Bool(b.val.Num < a.val.Num);
    case T_CHAR:
        return Bool(b.val.Char < a.val.Char);
    case T_STR:
        return string_lt(b, a);
    default:
        error("Invalid type for `<` operation");
        exit(1);
    }
}
element _le(element a, element b)
{

    if (a.type != b.type)
    {
        error("Invalid comparison");
        exit(1);
    }
    switch (b.type)
    {
    case T_NUM:
        return Bool(b.val.Num <= a.val.Num);
    case T_CHAR:
        return Bool(b.val.Char <= a.val.Char);
    case T_STR:
        return string_le(b, a);
    default:
        error("Invalid type for `<=` operation");
        exit(1);
    }
}
element _gt(element a, element b)
{

    if (a.type != b.type)
    {
        error("Invalid comparison");
        exit(1);
    }
    switch (b.type)
    {
    case T_NUM:
        return Bool(b.val.Num > a.val.Num);
    case T_CHAR:
        return Bool(b.val.Char > a.val.Char);
    case T_STR:
        return string_gt(b, a);
    default:
        error("Invalid type for `>` operation");
        exit(1);
    }
}
element _ge(element a, element b)
{

    if (a.type != b.type)
    {
        error("Invalid comparison");
        exit(1);
    }
    switch (b.type)
    {
    case T_NUM:
        return Bool(b.val.Num >= a.val.Num);
    case T_CHAR:
        return Bool(b.val.Char >= a.val.Char);
    case T_STR:
        return string_ge(b, a);
    default:
        error("Invalid type for `>=` operation");
        exit(1);
    }
}

element _or(element a, element b)
{
    return Bool(a.val.Bool || b.val.Bool);
}
element _and(element a, element b)
{
    return Bool(b.val.Bool && a.val.Bool);
}

element _sqr(element a)
{
    if (a.type != T_NUM)
    {
        error("Invalid square root operation. Expected type number");
        exit(1);
    }

    return Num(sqrt(a.val.Num));
}
