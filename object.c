#include "object.h"
#include <stdio.h>
#include <string.h>

/*
static int hash(element key)
{
    int index = 2166136261u;

    switch (key.type)
    {
    case T_STR:
        for (char *s = key.val.String; *s; s++)
        {
            index ^= (int)*s;
            index *= 16777619;
        }
        break;
    case T_NUM:
        index ^= (Long)key.val.Num;
        index = (index * 16777669);
        break;
    case T_CHAR:
        index ^= key.val.Char;
        index = (index * 16742069);
        break;
    default:
        return 0;
    }
    return index;
}
*/
int hash_key(char *str)
{
	int index = 2166136261u;

	for (char *s = str; *s; s++)
	{
		index ^= (int)*s;
		index *= 16777619;
	}

	return index;
}

element generic_obj(void *obj, obj_t type)
{
	element s;
	s.obj  = obj;
	s.type = type;
	return s;
}

element key_obj(_key key)
{
	element s;
	s.key  = key;
	s.type = T_KEY;
	return s;
}

element value_obj(value val, obj_t type)
{
	element s;
	s.val  = val;
	s.type = type;
	return s;
}

element Char(char Char)
{
	value ar;
	ar.Char = Char;
	return OBJ(ar, T_CHAR);
}
element Num(double Num)
{
	value ar;
	ar.Num = Num;
	return OBJ(ar, T_NUM);
}

element NumType(double Num, obj_t type)
{
	value ar;
	ar.Num = Num;
	return OBJ(ar, type);
}

element Bool(bool Bool)
{
	value ar;
	ar.Bool = Bool;
	return OBJ(ar, T_BOOL);
}

element Null(void)
{
	element ar;
	ar.type = T_NULL;
	ar.obj  = NULL;
	return ar;
}

element StringCpy(const char *str, size_t size)
{
	value ar;
	ar.String = NULL;
	ar.String = (char *)str;
	ar.len    = size;
	return OBJ(ar, T_STR);
}
element String(const char *str, size_t size)
{
	if (size <= 0)
		return Null();
	value ar;
	ar.String = NULL;
	ar.String = ALLOC(size);
	memcpy(ar.String, str, size);
	ar.String[size] = '\0';
	ar.len          = size;
	return OBJ(ar, T_STR);
}

element KeyObj(const char *str, size_t size)
{
	return KEY(Key(str, size));
}
_key Key(const char *str, size_t size)
{
	_key ar;
	ar.val = NULL;
	ar.val = ALLOC(size == 1 ? 2 : size);
	memcpy(ar.val, str, size);
	ar.val[size] = '\0';
	int k        = hash_key(ar.val);
	ar.hash      = k;
	return ar;
}

static void parse_str(const char *str)
{
	char *s = (char *)str;
	// printf("\"");

	for (; *s; s++)
		if (*s == '\\' && s[1] == 'n')
			printf("\n"), s++;
		else if (*s == '\\' && s[1] == 't')
			printf("\t"), s++;
		else
			printf("%c", *s);
	// printf("\"");
}

static void println(element ar)
{
	switch (ar.type)
	{
	case T_NATIVE:
		printf("<native: %s>", NATIVE(ar)->name.val);
		break;
	case T_CLOSURE:
		printf("<fn: %s>", CLOSURE(ar)->func->name.val);
		break;
	case T_CLASS:
		printf("<class: %s>", CLASS(ar)->name.val);
		break;
	case T_KEY:
		printf("<id: %s>", ar.key.val);
		break;
	case T_INSTANCE:
		printf("<instance: %s>", INSTANCE(ar)->classc->name.val);
		break;

	case T_VECTOR:
	{
		vector *v = NULL;
		v         = VECTOR(ar);
		printf("[ ");

		for (int i = 0; i < v->count; i++)
		{
			println(OBJ(*(v->of + i), v->type));
			printf(i == v->count - 1 ? " ]" : ", ");
		}
		break;
	}
	case T_VECTOR_2D:
	{

		_2d_vector *v = NULL;
		v             = _2D_VECTOR(ar);
		printf("[\n");

		for (int i = 0; i < v->count; i++)
		{
			printf("\t\t");
			println(GEN(*(v->of + i), T_VECTOR));

			if (i != v->count - 1)
				printf(", \n");
		}
		printf("\n]");
		break;
	}
	case T_CHAR:
		printf("'%c'", ar.val.Char);
		break;
	case T_NUM:
		printf("%f", ar.val.Num);
		break;
	case T_BOOL:
		printf("%s", (ar.val.Bool) ? "true" : "false");
		break;
	case T_STR:
		parse_str(ar.val.String);
		break;
	case T_NULL:
		printf("[ null ]");
		break;

	default:
		return;
	}
}

void print(element ar)
{

	switch (ar.type)
	{
	case T_NATIVE:
		printf("<native: %s>\n", NATIVE(ar)->name.val);
		break;
	case T_CLOSURE:
		printf("<fn: %s>\n", CLOSURE(ar)->func->name.val);
		break;
	case T_CLASS:
		printf("<class: %s>\n", CLASS(ar)->name.val);
		break;
	case T_KEY:
		printf("<id: %s>\n", ar.key.val);
		break;
	case T_INSTANCE:
		printf("<instance: %s>\n", INSTANCE(ar)->classc->name.val);
		break;

	case T_VECTOR:
	{

		vector *v = NULL;
		v         = VECTOR(ar);
		printf("[ ");

		for (int i = 0; i < v->count; i++)
		{
			println(OBJ(*(v->of + i), v->type));
			printf(i == v->count - 1 ? " ]\n" : ", ");
		}
		break;
	}
	case T_VECTOR_2D:
	{

		_2d_vector *v = NULL;
		v             = _2D_VECTOR(ar);
		printf("[\n");

		for (int i = 0; i < v->count; i++)
		{
			printf("\t");
			println(GEN(*(v->of + i), T_VECTOR));

			if (i != v->count - 1)
				printf(", \n");
		}
		printf("\n]\n");
		break;
	}

	case T_VECTOR_3D:
	{

		_3d_vector *v = NULL;
		v             = _3D_VECTOR(ar);

		printf("[\n");
		for (int i = 0; i < v->count; i++)
		{
			println(GEN(*(v->of + i), T_VECTOR_2D));
			if (i != v->count - 1)
				printf(",\n");
		}
		printf("]\n");

		break;
	}
	case T_CHAR:
		printf("'%c'\n", ar.val.Char);
		break;
	case T_NUM:
		printf("%f\n", ar.val.Num);
		break;
	case T_BOOL:
		printf("%s\n", (ar.val.Bool) ? "true" : "false");
		break;
	case T_STR:
		parse_str(ar.val.String);
		printf("\n");
		break;
	case T_NULL:
		printf("[ null ]\n");
		break;

	default:
		return;
	}
}
