#include "error.h"
#include "object_math.h"
#include "object_string.h"
#include <math.h>

element _neg(element *a)
{

	switch (a->type)
	{
	case T_NUM:
		return Num(-a->as.Num);
	case T_CHAR:
		return Char(-a->as.Char);
	case T_BOOL:
		return Bool(!a->as.Bool);
	default:
		error("Invalid type for `++` operation");
		exit(1);
	}
}

element _add(element *a, element *b)
{

	switch (b->type)
	{
	case T_NUM:
		return Num(b->as.Num + a->as.Num);
	case T_CHAR:
		return Char(b->as.Char + a->as.Num);
	case T_STR:
		return append(b, a);
	default:
		error("Invalid addition operation");
		exit(1);
	}
}
element _to_str(element *a)
{
	switch (a->type)
	{
	case T_NUM:
		return lltoa(a->as.Num);
	case T_CHAR:
		return char_to_str(a->as.Char);
	case T_STRUCT:
		return Null();
	default:
		error("Invalid string conversion");
		exit(1);
	}
}

element _cast(element *a, cast_t type)
{

	switch (type)
	{
	case CAST_NUM_CHAR:
		return Char((char)a->as.Num);
	case CAST_NUM_STR:
		return lltoa((Long)a->as.Num);
	case CAST_CHAR_NUM:
		return Num((Long)a->as.Char);
	case CAST_CHAR_STR:
		return char_to_str(a->as.Char);
	case CAST_BOOL_NUM:
		return Num(a->as.Bool);
	case CAST_BOOL_STR:
		return String(
		    (a->as.Bool) ? "true" : "false", (a->as.Bool) ? 4 : 5
		);
	case CAST_STR_NUM:
		return str_to_num(a);
	case CAST_STR_BOOL:
		return str_to_bool(a);
	case CAST_STR_CHAR:
		return str_to_char(a);
	default:
		error("Invalid cast type: %d\n", type);
		exit(1);
	}
}
element _inc(element *a)
{

	switch (a->type)
	{
	case T_NUM:
		return Num(++a->as.Num);
	case T_CHAR:
		return Char(++a->as.Char);
	default:
		error("Invalid type for `++` operation");
		exit(1);
	}
}
element _dec(element *a)
{

	switch (a->type)
	{
	case T_NUM:
		return Num(--a->as.Num);
	case T_CHAR:
		return Char(--a->as.Char);
	default:
		error("Invalid type for `--` operation");
		exit(1);
	}
}
element _sub(element *a, element *b)
{

	if (a->type != b->type)
	{
		error("Invalid comparison");
		exit(1);
	}
	switch (b->type)
	{
	case T_NUM:
		return Num(b->as.Num - a->as.Num);
	case T_CHAR:
		return Char(b->as.Char - a->as.Char);
	default:
		error("Invalid type for `-` operation");
		exit(1);
	}
}
element _mul(element *a, element *b)
{

	if (a->type != b->type)
	{
		error("Invalid comparison");
		exit(1);
	}
	switch (b->type)
	{
	case T_NUM:
		return Num(b->as.Num * a->as.Num);
	case T_CHAR:
		return Num((Long)b->as.Char * a->as.Char);
	default:
		error("Invalid type for `*` operation");
		exit(1);
	}
}
element _div(element *a, element *b)
{

	if (a->type != b->type)
	{
		error("Invalid comparison");
		exit(1);
	}
	switch (b->type)
	{
	case T_NUM:
		return Num(b->as.Num / a->as.Num);
	case T_CHAR:
		return Num((double)b->as.Char / (double)a->as.Char);
	default:
		error("Invalid type for `/` operation");
		exit(1);
	}
}
element _mod(element *a, element *b)
{

	if (a->type != b->type)
	{
		error("Invalid comparison");
		exit(1);
	}
	switch (b->type)
	{
	case T_NUM:
		return Num((Long)b->as.Num % (Long)a->as.Num);
	case T_CHAR:
		return Char(b->as.Char % a->as.Char);
	default:
		error("Invalid type for `%` operation");
		exit(1);
	}
}
element _eq(element *a, element *b)
{

	if (a->type != b->type)
	{
		error("Invalid comparison");
		exit(1);
	}
	switch (b->type)
	{
	case T_NUM:
		return Bool(b->as.Num == a->as.Num);
	case T_CHAR:
		return Bool(b->as.Char == a->as.Char);
	case T_STR:
		return string_ne(b, a);
	default:
		error("Invalid type for `==` operation");
		exit(1);
	}
}
element _ne(element *a, element *b)
{

	if (a->type != b->type)
	{
		error("Invalid comparison");
		exit(1);
	}
	switch (b->type)
	{
	case T_NUM:
		return Bool(b->as.Num != a->as.Num);
	case T_CHAR:
		return Bool(b->as.Char != a->as.Char);
	case T_STR:
		return string_ne(b, a);
	default:
		error("Invalid type for `!=` operation");
		exit(1);
	}
}
element _lt(element *a, element *b)
{

	if (a->type != b->type)
	{
		error("Invalid comparison");
		exit(1);
	}
	switch (b->type)
	{
	case T_NUM:
		return Bool(b->as.Num < a->as.Num);
	case T_CHAR:
		return Bool(b->as.Char < a->as.Char);
	case T_STR:
		return string_lt(b, a);
	default:
		error("Invalid type for `<` operation");
		exit(1);
	}
}
element _le(element *a, element *b)
{

	if (a->type != b->type)
	{
		error("Invalid comparison");
		exit(1);
	}
	switch (b->type)
	{
	case T_NUM:
		return Bool(b->as.Num <= a->as.Num);
	case T_CHAR:
		return Bool(b->as.Char <= a->as.Char);
	case T_STR:
		return string_le(b, a);
	default:
		error("Invalid type for `<=` operation");
		exit(1);
	}
}
element _gt(element *a, element *b)
{

	if (a->type != b->type)
	{
		error("Invalid comparison");
		exit(1);
	}
	switch (b->type)
	{
	case T_NUM:
		return Bool(b->as.Num > a->as.Num);
	case T_CHAR:
		return Bool(b->as.Char > a->as.Char);
	case T_STR:
		return string_gt(b, a);
	default:
		error("Invalid type for `>` operation");
		exit(1);
	}
}
element _ge(element *a, element *b)
{

	if (a->type != b->type)
	{
		error("Invalid comparison");
		exit(1);
	}
	switch (b->type)
	{
	case T_NUM:
		return Bool(b->as.Num >= a->as.Num);
	case T_CHAR:
		return Bool(b->as.Char >= a->as.Char);
	case T_STR:
		return string_ge(b, a);
	default:
		error("Invalid type for `>=` operation");
		exit(1);
	}
}

element _or(element *a, element *b)
{
	return Bool(a->as.Bool || b->as.Bool);
}
element _and(element *a, element *b)
{
	return Bool(b->as.Bool && a->as.Bool);
}

element _sqr(element *a)
{
	if (a->type != T_NUM)
	{
		error("Invalid square root operation. Expected type number");
		exit(1);
	}

	return Num(sqrt(a->as.Num));
}
