#include "error.h"
#include "object_math.h"
#include "object_string.h"
#include <math.h>

element _neg(element *a)
{

	switch (a->type)
	{
	case T_NUM:
		return Num(-a->val.Num);
	case T_CHAR:
		return Char(-a->val.Char);
	case T_BOOL:
		return Bool(!a->val.Bool);
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
		return Num(b->val.Num + a->val.Num);
	case T_CHAR:
		return Char(b->val.Char + a->val.Num);
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
		return lltoa(a->val.Num);
	case T_CHAR:
		return char_to_str(a->val.Char);
	case T_CLASS:
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
		return Char((char)a->val.Num);
	case CAST_NUM_STR:
		return lltoa((Long)a->val.Num);
	case CAST_CHAR_NUM:
		return Num((Long)a->val.Char);
	case CAST_CHAR_STR:
		return char_to_str(a->val.Char);
	case CAST_BOOL_NUM:
		return Num(a->val.Bool);
	case CAST_BOOL_STR:
		return String(
		    (a->val.Bool) ? "true" : "false", (a->val.Bool) ? 4 : 5
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
		return Num(++a->val.Num);
	case T_CHAR:
		return Char(++a->val.Char);
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
		return Num(--a->val.Num);
	case T_CHAR:
		return Char(--a->val.Char);
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
		return Num(b->val.Num - a->val.Num);
	case T_CHAR:
		return Char(b->val.Char - a->val.Char);
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
		return Num(b->val.Num * a->val.Num);
	case T_CHAR:
		return Num((Long)b->val.Char * a->val.Char);
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
		return Num(b->val.Num / a->val.Num);
	case T_CHAR:
		return Num((double)b->val.Char / (double)a->val.Char);
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
		return Num((Long)b->val.Num % (Long)a->val.Num);
	case T_CHAR:
		return Char(b->val.Char % a->val.Char);
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
		return Bool(b->val.Num == a->val.Num);
	case T_CHAR:
		return Bool(b->val.Char == a->val.Char);
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
		return Bool(b->val.Num != a->val.Num);
	case T_CHAR:
		return Bool(b->val.Char != a->val.Char);
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
		return Bool(b->val.Num < a->val.Num);
	case T_CHAR:
		return Bool(b->val.Char < a->val.Char);
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
		return Bool(b->val.Num <= a->val.Num);
	case T_CHAR:
		return Bool(b->val.Char <= a->val.Char);
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
		return Bool(b->val.Num > a->val.Num);
	case T_CHAR:
		return Bool(b->val.Char > a->val.Char);
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
		return Bool(b->val.Num >= a->val.Num);
	case T_CHAR:
		return Bool(b->val.Char >= a->val.Char);
	case T_STR:
		return string_ge(b, a);
	default:
		error("Invalid type for `>=` operation");
		exit(1);
	}
}

element _or(element *a, element *b)
{
	return Bool(a->val.Bool || b->val.Bool);
}
element _and(element *a, element *b)
{
	return Bool(b->val.Bool && a->val.Bool);
}

element _sqr(element *a)
{
	if (a->type != T_NUM)
	{
		error("Invalid square root operation. Expected type number");
		exit(1);
	}

	return Num(sqrt(a->val.Num));
}
