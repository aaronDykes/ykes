#include "arena.h"
#include <string.h>
#include <stdio.h>

static long long int hash(arena key)
{
    long long int index = 2166136261u;

    switch (key.type)
    {
    case ARENA_VAR:
    case ARENA_STR:
    case ARENA_CSTR:
        for (char *s = key.as.String; *s; s++)
        {
            index ^= (int)*s;
            index *= 16777619;
        }
        break;
    case ARENA_INT:
        index ^= key.as.Int;
        index = (index * 16777669);
        break;
    case ARENA_DOUBLE:
        index ^= ((long long int)key.as.Double);
        index = (index * 16777420);
        break;
    case ARENA_LONG:
        index ^= key.as.Long;
        index = (index * 16776969);
        break;
    case ARENA_CHAR:
        index ^= key.as.Char;
        index = (index * 16742069);
        break;
    default:
        return 0;
    }
    return index;
}

stack _value(element e)
{
    stack s;
    s.as = e;
    return s;
}

element stack_el(stack *el)
{
    element e;
    e.stack = el;
    e.type = STACK;
    return e;
}

element Obj(arena ar)
{
    element s;
    s._arena = ar;
    s.type = ARENA;

    return s;
}

element native_fn(native *native)
{
    element s;

    s.native = native;
    s.type = NATIVE;

    return s;
}

element closure_el(closure *clos)
{
    element el;
    el.closure = clos;
    el.type = CLOSURE;

    return el;
}

element new_class(class *classc)
{
    element el;
    el.classc = classc;
    el.type = CLASS;
    return el;
}

element new_instance(instance *ci)
{
    element el;
    el.instance = ci;
    el.type = INSTANCE;
    return el;
}
element table_el(table *t)
{
    element el;
    el.table = t;
    el.type = TABLE;
    return el;
}
element vector_el(arena *vect)
{
    element el;
    el._vector = vect;
    el.type = VECTOR;
    return el;
}
element null_obj(void)
{
    element el;
    el.null = NULL;
    el.type = NULL_OBJ;

    return el;
}

arena arena_init(void *data, size_t size, T type)
{
    arena ar;

    switch (type)
    {
    case ARENA_BYTES:
        ar.listof.Bytes = data;
        ar.len = (int)size;
        ar.count = 0;
        break;
    case ARENA_STR:
    case ARENA_VAR:
        ar.as.String = data;
        ar.as.len = (int)size;
        ar.as.count = 0;
        break;
    case ARENA_INTS:
        ar.listof.Ints = data;
        ar.len = (int)(size / sizeof(int));
        ar.count = 0;
        break;
    case ARENA_DOUBLES:
        ar.listof.Doubles = data;
        ar.len = ((int)(size / sizeof(double)));
        ar.count = 0;
        break;
    case ARENA_LONGS:
        ar.listof.Longs = data;
        ar.len = ((int)(size / sizeof(long long int)));
        ar.count = 0;
        break;
    case ARENA_STRS:
        ar.listof.Strings = data;
        ar.len = ((int)(size / sizeof(char *)));
        ar.count = 0;
        break;
    default:
        return Null();
    }
    ar.size = size;
    ar.type = type;

    return ar;
}

arena Char(char Char)
{
    arena ar;
    ar.type = ARENA_CHAR;
    ar.as.Char = Char;
    ar.size = sizeof(char);
    long long int k = hash(ar);
    ar.as.hash = k;
    return ar;
}
arena Int(int Int)
{
    arena ar;
    ar.type = ARENA_INT;
    ar.as.Int = Int;
    ar.size = sizeof(int);
    ar.as.hash = Int;
    return ar;
}
arena Byte(uint8_t Byte)
{
    arena ar;
    ar.type = ARENA_BYTE;
    ar.as.Byte = Byte;
    ar.size = sizeof(uint8_t);
    return ar;
}
arena Long(long long int Long)
{
    arena ar;
    ar.type = ARENA_LONG;
    ar.as.Long = Long;
    ar.size = sizeof(long long int);
    long long int k = hash(ar);
    ar.as.hash = k;
    return ar;
}
arena Double(double Double)
{
    arena ar;
    ar.type = ARENA_DOUBLE;
    ar.as.Double = Double;
    ar.size = sizeof(double);
    long long int h = hash(ar);
    ar.as.hash = h;
    return ar;
}

arena CString(const char *str)
{
    size_t size = strlen(str);
    arena ar;
    ar.as.String = (char *)str;
    ar.size = size;
    ar.type = ARENA_CSTR;
    ar.as.len = (int)size;
    long long int k = hash(ar);
    ar.as.hash = k;
    return ar;
}

arena Bool(bool Bool)
{
    arena ar;
    ar.type = ARENA_BOOL;
    ar.as.Bool = Bool;
    ar.size = sizeof(bool);
    return ar;
}
arena Size(size_t Size)
{
    arena ar;
    ar.type = ARENA_SIZE;
    ar.as.Size = Size;
    ar.size = 1;
    long long int k = hash(ar);
    ar.as.hash = k;
    return ar;
}
arena Null(void)
{
    arena ar;
    ar.type = ARENA_NULL;
    ar.as.Void = NULL;
    return ar;
}

arena arena_alloc(size_t size, T type)
{

    void *ptr = NULL;
    ptr = ALLOC(size);
    return arena_init(ptr, size, type);
}
arena String(const char *str)
{
    size_t size = strlen(str);
    arena ar = arena_alloc(size, ARENA_STR);
    memcpy(ar.as.String, str, size);
    ar.as.String[size] = '\0';
    ar.size = size;
    long long int k = hash(ar);
    ar.as.hash = k;
    return ar;
}
arena Var(const char *str)
{
    size_t size = strlen(str);
    arena ar = arena_alloc(size, ARENA_VAR);
    memcpy(ar.as.String, str, size);
    ar.as.String[size] = '\0';
    size_t h = hash(ar);
    ar.as.hash = h;
    ar.type = ARENA_VAR;
    return ar;
}

static void parse_str(const char *str)
{
    char *s = (char *)str;

    for (; *s; s++)
        if (*s == '\\' && s[1] == 'n')
            printf("\n"), s++;
        else if (*s == '\\' && s[1] == 't')
            printf("\t"), s++;
        else
            printf("%c", *s);
}

void print(element ar)
{
    arena a = ar._arena;

    if (ar.type == NATIVE)
    {
        printf("<native: %s>\n", ar.native->obj.as.String);
        return;
    }
    if (ar.type == CLOSURE)
    {
        printf("<fn: %s>\n", ar.closure->func->name.as.String);
        return;
    }
    if (ar.type == CLASS)
    {
        if (!ar.classc)
            return;
        printf("<class: %s>\n", ar.classc->name.as.String);
        return;
    }
    if (ar.type == INSTANCE)
    {
        if (!ar.instance->classc)
            return;
        printf("<instance: %s>\n", ar.instance->classc->name.as.String);
        return;
    }
    if (ar.type == VECTOR)
    {
        printf("[");

        int count = (ar._vector - 1)->count;
        if (count == 0)
        {
            printf(" ]\n");
            return;
        }

        printf("\n");
        for (int i = 0; i < (ar._vector - 1)->count; i++)
        {
            print(OBJ(ar._vector[i]));
            if (i != (ar._vector - 1)->count - 1)
                printf(", ");
        }

        printf("\n]\n");
        return;
    }

    if (ar.type == STACK)
    {
        if (!ar.stack)
            return;
        printf("{");

        if (ar.stack->count == 0)
        {
            printf(" }\n");
            return;
        }
        printf("\n");
        for (int i = 0; i < ar.stack->count; i++)
        {
            print((ar.stack + i)->as);
            if (i != ar.stack->count - 1)
                printf(",\n");
        }

        printf("\n}\n");
        return;
    }

    switch (a.type)
    {
    case ARENA_BYTE:
        printf("%d\n", a.as.Byte);
        break;
    case ARENA_CHAR:
        printf("%c\n", a.as.Char);
        break;
    case ARENA_DOUBLE:
        printf("%f\n", a.as.Double);
        break;
    case ARENA_INT:
        printf("%d\n", a.as.Int);
        break;
    case ARENA_LONG:
        printf("%lld\n", a.as.Long);
        break;
    case ARENA_BOOL:
        printf("%s\n", (a.as.Bool == true) ? "true" : "false");
        break;
    case ARENA_STR:
    case ARENA_VAR:
    case ARENA_CSTR:
        parse_str(a.as.String);
        printf("\n");
        break;
    case ARENA_INTS:
        printf("[ ");
        for (int i = 0; i < a.count; i++)
            if (i == a.count - 1)
                printf("%d ]\n", a.listof.Ints[i]);
            else
                printf("%d, ", a.listof.Ints[i]);
        break;
    case ARENA_DOUBLES:
        printf("[ ");
        for (int i = 0; i < a.count; i++)
            if (i == a.count - 1)
                printf("%f ]\n", a.listof.Doubles[i]);
            else
                printf("%f, ", a.listof.Doubles[i]);
        break;
    case ARENA_STRS:
        printf("[ ");
        for (int i = 0; i < a.count; i++)
            if (i == a.count - 1)
            {

                parse_str(a.listof.Strings[i]);
                printf(" ]\n");
            }
            else
            {
                parse_str(a.listof.Strings[i]);
                printf(", ");
            }
        break;
    case ARENA_NULL:
        printf("[ null ]\n");
        break;

    default:
        return;
    }
}
