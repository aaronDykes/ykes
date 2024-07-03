#include "arena_math.h"
#include "arena_math_util.h"
#include "arena_string.h"
#include "arena_table.h"
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

static arena Num(long long int range)
{
    if (range < INT32_MAX && range > INT32_MIN)
        return Int((int)range);
    if (range < INT64_MAX && range > INT64_MIN)
        return Long(range);
    log_err("ERROR: numeric value out of range");
    return Null();
}

static arena add_arena_size(size_t size, arena ar)
{
    switch (ar.type)
    {
    case ARENA_CSTR:
    case ARENA_STR:
        return prepend_int_to_str(Int((int)size), ar);
    case ARENA_INT:
        return Size(size + ar.as.Int);
    case ARENA_DOUBLE:
        return Size(size + (int)ar.as.Double);
    case ARENA_LONG:
        return Size(size + ar.as.Long);
    case ARENA_CHAR:
        return Size(size + ar.as.Char);
    case ARENA_SIZE:
        return Size(size + ar.as.Size);

    default:
        log_err("ERROR: addition type mismatch\n");
        exit(1);
    }
}
static arena sub_arena_size(size_t size, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Size(size - ar.as.Int);
    case ARENA_DOUBLE:
        return Size(size - (int)ar.as.Double);
    case ARENA_LONG:
        return Size(size - ar.as.Long);
    case ARENA_CHAR:
        return Size(size - ar.as.Char);
    case ARENA_SIZE:
        return Size(size - ar.as.Size);

    default:
        log_err("ERROR: subtraction type mismatch\n");
        exit(1);
    }
}
static arena mul_arena_size(size_t size, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Size(size * ar.as.Int);
    case ARENA_DOUBLE:
        return Size(size * ar.as.Double);
    case ARENA_LONG:
        return Size(size * ar.as.Long);
    case ARENA_CHAR:
        return Size(size * ar.as.Char);
    case ARENA_SIZE:
        return Size(size * ar.as.Size);

    default:
        log_err("ERROR: multiplication type mismatch\n");
        exit(1);
    }
}
static arena div_arena_size(size_t size, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        if (ar.as.Int == 0)
            goto ERR;
        return Size(size / ar.as.Int);
    case ARENA_CHAR:
        if (ar.as.Char == 0)
            goto ERR;
        return Size(size / ar.as.Char);
    case ARENA_DOUBLE:
        if (ar.as.Double == 0)
            goto ERR;
        return Size(size / (int)ar.as.Double);
    case ARENA_LONG:
        if (ar.as.Long == 0)
            goto ERR;
        return Size(size / ar.as.Long);
    case ARENA_SIZE:
        if (ar.as.Size == 0)
            goto ERR;
        return Size(size / ar.as.Size);
    default:
        log_err("ERROR: division type mismatch\n");
        exit(1);
    }
ERR:
    log_err("ERROR: divide by zero\n");

    return ar;
}
static arena mod_arena_size(size_t size, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Size(size % ar.as.Int);
    case ARENA_DOUBLE:
        return Size(size % (int)ar.as.Double);
    case ARENA_LONG:
        return Size(size % ar.as.Long);
    case ARENA_CHAR:
        return Size(size % ar.as.Char);
    case ARENA_SIZE:
        return Size(size % ar.as.Size);
    default:
        log_err("ERROR: modulo type mismatch\n");
        exit(1);
    }
    return Null();
}

static arena add_arena_char(char ch, arena ar)
{
    long long int test = 0;
    switch (ar.type)
    {
    case ARENA_INT:
        test = (long long int)(ch + ar.as.Int);
        return (test < 255 && test > 0) ? Char((char)test) : Num(test);
    case ARENA_CHAR:
        test = (long long int)(ch + ar.as.Char);
        return (test < 255 && test > 0) ? Char((char)test) : Int((int)test);
    case ARENA_DOUBLE:
        return Double((double)ch + ar.as.Double);
    case ARENA_LONG:
        return Num(ch + ar.as.Long);
    case ARENA_CSTR:
    case ARENA_STR:
        return prepend_char_to_str(Char(ch), ar);
    case ARENA_SIZE:
        return Num(ch + ar.as.Size);
    default:
        log_err("ERROR: addition type mismatch\n");
        exit(1);
    }
    return Null();
}
static arena sub_arena_char(char ch, arena ar)
{
    long long int test = 0;
    switch (ar.type)
    {
    case ARENA_INT:
        test = (ch - ar.as.Int);
        return (test > 0) ? Char((char)test) : Int((int)test);
    case ARENA_CHAR:
        test = (ch - ar.as.Char);
        return (test > 0) ? Char((char)test) : Int((int)test);
    case ARENA_DOUBLE:
        return Double((double)ch - ar.as.Double);
    case ARENA_LONG:
        test = (ch - ar.as.Long);
        return (test > 0) ? Char((char)test) : Num(test);
    case ARENA_SIZE:
        test = (ch - ar.as.Size);
        return (test > 0) ? Char((char)test) : Num(test);
    default:
        log_err("ERROR: subtraction type mismatch\n");
        exit(1);
    }
    return ar;
}
static arena mul_arena_char(char ch, arena ar)
{
    long long int test = 0;
    switch (ar.type)
    {
    case ARENA_INT:
        test = (ch * ar.as.Int);
        return (test < 255 && test > 0) ? Char((char)test) : Num(test);
    case ARENA_CHAR:
        test = (ch * ar.as.Char);
        return (test < 255 && test > 0) ? Char((char)test) : Int((int)test);
    case ARENA_DOUBLE:
        return Double((double)ch * ar.as.Double);
    case ARENA_LONG:
        return Num(ch * ar.as.Long);
    case ARENA_SIZE:
        break;
    default:
        log_err("ERROR: multiplication type mismatch\n");
        exit(1);
    }
    return ar;
}
static arena div_arena_char(char ch, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        if (ar.as.Int == 0)
            goto ERR;
        return Double((double)ch / ar.as.Int);
    case ARENA_CHAR:
        if (ar.as.Char == 0)
            goto ERR;
        return Double((double)ch / ar.as.Char);
    case ARENA_DOUBLE:
        if (ar.as.Double == 0)
            goto ERR;
        return Double((double)ch / ar.as.Double);
    case ARENA_LONG:
        if (ar.as.Long == 0)
            goto ERR;
        return Double((double)ch / ar.as.Long);
    case ARENA_SIZE:
        if (ar.as.Long == 0)
            goto ERR;
        return Double((double)ch / ar.as.Size);
    default:
        log_err("ERROR: division type mismatch\n");
        exit(1);
    }
ERR:
    log_err("ERROR: divide by zero\n");

    return ar;
}
static arena mod_arena_char(char ch, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Int(ch % ar.as.Char);
    case ARENA_CHAR:
        return Char(ch % ar.as.Char);
    case ARENA_LONG:
        return Num((long long int)ch % ar.as.Long);
    case ARENA_SIZE:
        return Num(ch % ar.as.Size);
    default:
        log_err("ERROR: modulo type mismatch\n");
        exit(1);
    }
    return ar;
}

static arena add_arena_int(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_CSTR:
    case ARENA_STR:
        return prepend_int_to_str(Int(ival), ar);
    case ARENA_INT:
        return Num((long long int)ival + ar.as.Int);
    case ARENA_DOUBLE:
        return Double((double)ival + ar.as.Double);
    case ARENA_LONG:
        return Num((long long int)ival + ar.as.Long);
    case ARENA_CHAR:
        return Num((long long int)ival + ar.as.Char);
    case ARENA_SIZE:
        return Num((long long int)ival + ar.as.Size);
    default:
        log_err("ERROR: addition type mismatch\n");
        exit(1);
    }
    return ar;
}
static arena sub_arena_int(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Num(ival - ar.as.Int);
    case ARENA_DOUBLE:
        return Double((double)ival - ar.as.Double);
    case ARENA_LONG:
        return Num(ival - ar.as.Long);
    case ARENA_CHAR:
        return Num(ival - ar.as.Char);
    case ARENA_SIZE:
        return Num(ival - ar.as.Size);
    default:
        log_err("ERROR: subtraction type mismatch\n");
        exit(1);
    }
    return ar;
}
static arena mul_arena_int(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Num((long long int)ival * ar.as.Int);
    case ARENA_DOUBLE:
        return Double((double)ival * ar.as.Double);
    case ARENA_LONG:
        return Num((long long int)ival * ar.as.Long);
    case ARENA_CHAR:
        return Num(ival * ar.as.Char);
    case ARENA_SIZE:
        return Num(ival * ar.as.Size);
    default:
        log_err("ERROR: multiplication type mismatch\n");
        exit(1);
    }
    return ar;
}
static arena div_arena_int(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        if (ar.as.Int == 0)
            goto ERR;
        return Double((double)ival / ar.as.Int);
    case ARENA_CHAR:
        if (ar.as.Char == 0)
            goto ERR;
        return Double((double)ival / ar.as.Char);
    case ARENA_DOUBLE:
        if (ar.as.Double == 0)
            goto ERR;
        return Double((double)ival / ar.as.Double);
    case ARENA_LONG:
        if (ar.as.Long == 0)
            goto ERR;
        return Double((double)ival / ar.as.Long);
    case ARENA_SIZE:
        if (ar.as.Size == 0)
            goto ERR;
        return Double((double)ival / ar.as.Size);
    default:
        log_err("ERROR: division type mismatch\n");
        exit(1);
    }
ERR:
    log_err("ERROR: divide by zero\n");

    return ar;
}
static arena mod_arena_int(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Int(ival % ar.as.Int);
    case ARENA_CHAR:
        return Char(ival % ar.as.Char);
    case ARENA_LONG:
        return Long((long long int)ival % ar.as.Long);
    case ARENA_SIZE:
        return Int(ival % ar.as.Size);
    default:
        log_err("ERROR: modulo type mismatch\n");
        exit(1);
    }
    return ar;
}

static arena add_arena_long(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_CHAR:
        return Num(llint + ar.as.Char);
    case ARENA_INT:
        return Num(llint + ar.as.Int);
    case ARENA_DOUBLE:
        return Double((double)llint + ar.as.Double);
    case ARENA_LONG:
        return Num(llint + ar.as.Long);
    case ARENA_CSTR:
    case ARENA_STR:
        return prepend_long_to_str(Long(llint), ar);
    case ARENA_SIZE:
        return Num(llint + ar.as.Size);
    default:
        log_err("ERROR: subtraction type mismatch\n");
        exit(1);
    }
    return ar;
}
static arena sub_arena_long(long long int llint, arena ar)
{

    switch (ar.type)
    {
    case ARENA_INT:
        return Long(llint - ar.as.Int);
    case ARENA_CHAR:
        return Long(llint - ar.as.Char);
    case ARENA_DOUBLE:
        return Double((double)llint - ar.as.Double);
    case ARENA_LONG:
        return Long(llint - ar.as.Long);
    case ARENA_SIZE:
        return Long(llint - ar.as.Size);
    default:
        log_err("ERROR: subtraction type mismatch\n");
        exit(1);
    }
    return ar;
}
static arena mul_arena_long(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_CHAR:
        return Long(llint * ar.as.Char);
    case ARENA_INT:
        return Long(llint * ar.as.Int);
    case ARENA_DOUBLE:
        return Double((double)llint * ar.as.Double);
    case ARENA_LONG:
        return Long(llint * ar.as.Long);
    case ARENA_SIZE:
        return Long(llint * ar.as.Size);
    default:
        log_err("ERROR: multiplication type mismatch\n");
        exit(1);
    }
    return ar;
}
static arena div_arena_long(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        if (ar.as.Int == 0)
            goto ERR;
        return Double((double)llint / ar.as.Int);
    case ARENA_CHAR:
        if (ar.as.Char == 0)
            goto ERR;
        return Double((double)llint / ar.as.Char);
    case ARENA_DOUBLE:
        if (ar.as.Double == 0)
            goto ERR;
        return Double((double)llint / ar.as.Double);
    case ARENA_LONG:
        if (ar.as.Long == 0)
            goto ERR;
        return Double((double)llint / ar.as.Long);
    case ARENA_SIZE:
        return Double((double)llint / ar.as.Size);
    default:
        log_err("ERROR: division type mismatch\n");
        exit(1);
    }
ERR:
    log_err("ERROR: divide by zero\n");
    return Null();
}
static arena mod_arena_long(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Int(llint % ar.as.Int);
    case ARENA_CHAR:
        return Char(llint % ar.as.Char);
    case ARENA_LONG:
        return Long(llint % ar.as.Long);
    case ARENA_SIZE:
        return Long(llint % ar.as.Size);
    default:
        log_err("ERROR: modulo type mismatch\n");
        exit(1);
    }
}

static arena add_arena_double(double dval, arena ar)
{
    switch (ar.type)
    {
    case ARENA_CHAR:
        return Double(dval + ar.as.Char);
    case ARENA_INT:
        return Double(dval + ar.as.Int);
    case ARENA_DOUBLE:
        return Double(dval + ar.as.Double);
    case ARENA_LONG:
        return Double(dval + ar.as.Long);
    case ARENA_SIZE:
        return Double(dval + ar.as.Size);
    default:
        log_err("ERROR: addition type mismatch\n");
        exit(1);
    }
}
static arena sub_arena_double(double dval, arena ar)
{
    switch (ar.type)
    {
    case ARENA_CHAR:
        return Double(dval - ar.as.Char);
    case ARENA_INT:
        return Double(dval - ar.as.Int);
    case ARENA_DOUBLE:
        return Double(dval - ar.as.Double);
    case ARENA_LONG:
        return Double(dval - ar.as.Long);
    case ARENA_SIZE:
        return Double(dval - ar.as.Size);
    default:
        log_err("ERROR: subtraction type mismatch\n");
        exit(1);
    }
}
static arena mul_arena_double(double dval, arena ar)
{
    switch (ar.type)
    {
    case ARENA_CHAR:
        return Double(dval * ar.as.Char);
    case ARENA_INT:
        return Double(dval * ar.as.Int);
    case ARENA_DOUBLE:
        return Double(dval * ar.as.Double);
    case ARENA_LONG:
        return Double(dval * ar.as.Long);
    case ARENA_SIZE:
        return Double(dval * ar.as.Size);

    default:
        log_err("ERROR: multiplication type mismatch\n");
        exit(1);
    }
    return ar;
}
static arena div_arena_double(double dval, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        if (ar.as.Int == 0)
            goto ERR;
        return Double(dval / ar.as.Int);
    case ARENA_CHAR:
        if (ar.as.Char == 0)
            goto ERR;
        return Double(dval / ar.as.Char);
    case ARENA_DOUBLE:
        if (ar.as.Double == 0)
            goto ERR;
        return Double(dval / ar.as.Double);
    case ARENA_LONG:
        if (ar.as.Long == 0)
            goto ERR;
        return Double(dval / ar.as.Long);
    case ARENA_SIZE:
        if (ar.as.Long == 0)
            goto ERR;
        return Double(dval / ar.as.Size);
    default:
        log_err("ERROR: division type mismatch\n");
        exit(1);
    }
ERR:
    log_err("ERROR: divide by zero\n");

    return ar;
}

static arena char_eq(char ch, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(ch == ar.as.Int);
    case ARENA_DOUBLE:
        return Bool(ch == ar.as.Double);
    case ARENA_LONG:
        return Bool(ch == ar.as.Long);
    case ARENA_CHAR:
        return Bool(ch == ar.as.Char);
    case ARENA_NULL:
        return Bool(ch == '\0');
    case ARENA_SIZE:
        return Bool(ch == (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch\n");
        exit(1);
    }
    return Bool(false);
}
static arena char_ne(char ch, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(ch != ar.as.Int);
    case ARENA_DOUBLE:
        return Bool(ch != ar.as.Double);
    case ARENA_LONG:
        return Bool(ch != ar.as.Long);
    case ARENA_CHAR:
        return Bool(ch != ar.as.Char);
    case ARENA_NULL:
        return Bool(ch != '\0');
    case ARENA_SIZE:
        return Bool(ch != (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch\n");
        exit(1);
    }
    return Bool(false);
}
static arena char_lt(char ch, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(ch < ar.as.Int);
    case ARENA_DOUBLE:
        return Bool(ch < ar.as.Double);
    case ARENA_LONG:
        return Bool(ch < ar.as.Long);
    case ARENA_CHAR:
        return Bool(ch < ar.as.Char);
    case ARENA_SIZE:
        return Bool(ch < (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
    }
}
static arena char_le(char ch, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(ch <= ar.as.Int);
    case ARENA_DOUBLE:
        return Bool(ch <= ar.as.Double);
    case ARENA_LONG:
        return Bool(ch <= ar.as.Long);
    case ARENA_CHAR:
        return Bool(ch <= ar.as.Char);
    case ARENA_SIZE:
        return Bool(ch <= (int)ar.as.Size);

    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
    }
}
static arena char_gt(char ch, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(ch > ar.as.Int);
    case ARENA_DOUBLE:
        return Bool(ch > ar.as.Double);
    case ARENA_LONG:
        return Bool(ch > ar.as.Long);
    case ARENA_CHAR:
        return Bool(ch > ar.as.Char);
    case ARENA_SIZE:
        return Bool(ch > (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
    }
}
static arena char_ge(char ch, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(ch >= ar.as.Int);
    case ARENA_DOUBLE:
        return Bool(ch >= ar.as.Double);
    case ARENA_LONG:
        return Bool(ch >= ar.as.Long);
    case ARENA_CHAR:
        return Bool(ch >= ar.as.Char);
    case ARENA_SIZE:
        return Bool(ch >= (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
    }
}

static arena int_eq(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(ival == ar.as.Int);
    case ARENA_DOUBLE:
        return Bool(ival == ar.as.Double);
    case ARENA_LONG:
        return Bool(ival == ar.as.Long);
    case ARENA_CHAR:
        return Bool(ival == ar.as.Char);
    case ARENA_CSTR:
    case ARENA_STR:
        return itoa_eqcmp(ival, ar);
    case ARENA_SIZE:
        return Bool(ival == (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
    }
}
static arena int_ne(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(ival != ar.as.Int);
    case ARENA_DOUBLE:
        return Bool(ival != ar.as.Double);
    case ARENA_LONG:
        return Bool(ival != ar.as.Long);
    case ARENA_CHAR:
        return Bool(ival != ar.as.Char);
    case ARENA_CSTR:
    case ARENA_STR:
        return itoa_neqcmp(ival, ar);
    case ARENA_SIZE:
        return Bool(ival != (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
    }
}
static arena int_lt(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(ival < ar.as.Int);
    case ARENA_DOUBLE:
        return Bool(ival < ar.as.Double);
    case ARENA_LONG:
        return Bool(ival < ar.as.Long);
    case ARENA_CHAR:
        return Bool(ival < ar.as.Char);
    case ARENA_SIZE:
        return Bool(ival < (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
    }
}
static arena int_le(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(ival <= ar.as.Int);
    case ARENA_DOUBLE:
        return Bool(ival <= ar.as.Double);
    case ARENA_LONG:
        return Bool(ival <= ar.as.Long);
    case ARENA_CHAR:
        return Bool(ival <= ar.as.Char);
    case ARENA_SIZE:
        return Bool(ival <= (int)ar.as.Size);

    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
    }
}
static arena int_gt(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(ival > ar.as.Int);
    case ARENA_DOUBLE:
        return Bool(ival > ar.as.Double);
    case ARENA_LONG:
        return Bool(ival > ar.as.Long);
    case ARENA_CHAR:
        return Bool(ival > ar.as.Char);
    case ARENA_SIZE:
        return Bool(ival > (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
    }
}
static arena int_ge(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(ival >= ar.as.Int);
    case ARENA_DOUBLE:
        return Bool(ival >= ar.as.Double);
    case ARENA_LONG:
        return Bool(ival >= ar.as.Long);
    case ARENA_CHAR:
        return Bool(ival >= ar.as.Char);
    case ARENA_SIZE:
        return Bool(ival >= (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
    }
}

static arena long_eq(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(llint == ar.as.Int);
    case ARENA_DOUBLE:
        return Bool(llint == ar.as.Double);
    case ARENA_CHAR:
        return Bool(llint == ar.as.Char);
    case ARENA_LONG:
        return Bool(llint == ar.as.Long);
    case ARENA_CSTR:
    case ARENA_STR:
        return ltoa_eqcmp(llint, ar);
    case ARENA_SIZE:
        return Bool(llint == (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
    }
}
static arena long_ne(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(llint != ar.as.Int);
    case ARENA_DOUBLE:
        return Bool(llint != ar.as.Double);
    case ARENA_CHAR:
        return Bool(llint != ar.as.Char);
    case ARENA_LONG:
        return Bool(llint != ar.as.Long);
    case ARENA_CSTR:
    case ARENA_STR:
        return ltoa_neqcmp(llint, ar);
    case ARENA_SIZE:
        return Bool(llint != (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
    }
}
static arena long_lt(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(llint < ar.as.Int);
    case ARENA_DOUBLE:
        return Bool(llint < ar.as.Double);
    case ARENA_CHAR:
        return Bool(llint < ar.as.Char);
    case ARENA_LONG:
        return Bool(llint < ar.as.Long);
    case ARENA_SIZE:
        return Bool(llint < (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
    }
}
static arena long_le(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(llint <= ar.as.Int);
    case ARENA_DOUBLE:
        return Bool(llint <= ar.as.Double);
    case ARENA_CHAR:
        return Bool(llint <= ar.as.Char);
    case ARENA_LONG:
        return Bool(llint <= ar.as.Long);
    case ARENA_SIZE:
        return Bool(llint <= (int)ar.as.Size);

    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
    }
}
static arena long_gt(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(llint > ar.as.Int);
    case ARENA_DOUBLE:
        return Bool(llint > ar.as.Double);
    case ARENA_CHAR:
        return Bool(llint > ar.as.Char);
    case ARENA_LONG:
        return Bool(llint > ar.as.Long);
    case ARENA_SIZE:
        return Bool(llint > (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
    }
}
static arena long_ge(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(llint >= ar.as.Int);
    case ARENA_DOUBLE:
        return Bool(llint >= ar.as.Double);
    case ARENA_CHAR:
        return Bool(llint >= ar.as.Char);
    case ARENA_LONG:
        return Bool(llint >= ar.as.Long);

    case ARENA_SIZE:
        return Bool(llint >= (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
    }
}

static arena double_eq(double dval, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(dval == ar.as.Int);
    case ARENA_CHAR:
        return Bool(dval == ar.as.Char);
    case ARENA_LONG:
        return Bool(dval == ar.as.Long);
    case ARENA_DOUBLE:
        return Bool(dval == ar.as.Double);
    case ARENA_SIZE:
        return Bool(dval == (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
    }
}
static arena double_ne(double dval, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(dval != ar.as.Int);
    case ARENA_CHAR:
        return Bool(dval != ar.as.Char);
    case ARENA_LONG:
        return Bool(dval != ar.as.Long);
    case ARENA_DOUBLE:
        return Bool(dval != ar.as.Double);
    case ARENA_SIZE:
        return Bool(dval != (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
    }
}
static arena double_lt(double dval, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(dval < ar.as.Int);
    case ARENA_CHAR:
        return Bool(dval < ar.as.Char);
    case ARENA_LONG:
        return Bool(dval < ar.as.Long);
    case ARENA_DOUBLE:
        return Bool(dval < ar.as.Double);
    case ARENA_SIZE:
        return Bool(dval < (int)ar.as.Size);

    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
    }
}
static arena double_le(double dval, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(dval <= ar.as.Int);
    case ARENA_CHAR:
        return Bool(dval <= ar.as.Char);
    case ARENA_LONG:
        return Bool(dval <= ar.as.Long);
    case ARENA_DOUBLE:
        return Bool(dval <= ar.as.Double);
    case ARENA_SIZE:
        return Bool(dval <= (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
        return Bool(false);
    }
}
static arena double_gt(double dval, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(dval > ar.as.Int);
    case ARENA_CHAR:
        return Bool(dval > ar.as.Char);
    case ARENA_LONG:
        return Bool(dval > ar.as.Long);
    case ARENA_DOUBLE:
        return Bool(dval > ar.as.Double);
    case ARENA_SIZE:
        return Bool(dval > (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
        return Bool(false);
    }
}
static arena double_ge(double dval, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(dval >= ar.as.Int);
    case ARENA_CHAR:
        return Bool(dval >= ar.as.Char);
    case ARENA_LONG:
        return Bool(dval >= ar.as.Long);
    case ARENA_DOUBLE:
        return Bool(dval >= ar.as.Double);
    case ARENA_SIZE:
        return Bool(dval >= (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
        return Bool(false);
    }
}

static arena size_eq(size_t size, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(size == (size_t)ar.as.Int);
    case ARENA_CHAR:
        return Bool(size == (size_t)ar.as.Char);
    case ARENA_LONG:
        return Bool(size == (size_t)ar.as.Long);
    case ARENA_DOUBLE:
        return Bool(size == (size_t)ar.as.Double);
    case ARENA_SIZE:
        return Bool(size == ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
        return Bool(false);
    }
}
static arena size_ne(size_t size, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(size != (size_t)ar.as.Int);
    case ARENA_CHAR:
        return Bool(size != (size_t)((int)ar.as.Char));
    case ARENA_LONG:
        return Bool(size != (size_t)ar.as.Long);
    case ARENA_DOUBLE:
        return Bool(size != (size_t)((int)ar.as.Double));
    case ARENA_SIZE:
        return Bool(size != ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
        return Bool(false);
    }
}
static arena size_lt(size_t size, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(size < (size_t)ar.as.Int);
    case ARENA_CHAR:
        return Bool(size < (size_t)ar.as.Char);
    case ARENA_LONG:
        return Bool(size < (size_t)ar.as.Long);
    case ARENA_DOUBLE:
        return Bool(size < (size_t)(ar.as.Double));
    case ARENA_SIZE:
        return Bool(size < ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
        return Bool(false);
    }
}
static arena size_le(size_t size, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(size <= (size_t)ar.as.Int);
    case ARENA_CHAR:
        return Bool(size <= (size_t)ar.as.Char);
    case ARENA_LONG:
        return Bool(size <= (size_t)ar.as.Long);
    case ARENA_DOUBLE:
        return Bool(size <= (size_t)(ar.as.Double));
    case ARENA_SIZE:
        return Bool(size <= ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
        return Bool(false);
    }
}

static arena size_gt(size_t size, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(size > (size_t)ar.as.Int);
    case ARENA_CHAR:
        return Bool(size > (size_t)ar.as.Char);
    case ARENA_LONG:
        return Bool(size > (size_t)ar.as.Long);
    case ARENA_DOUBLE:
        return Bool(size > (size_t)(ar.as.Double));
    case ARENA_SIZE:
        return Bool(size > ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
        return Bool(false);
    }
}
static arena size_ge(size_t size, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Bool(size >= (size_t)ar.as.Int);
    case ARENA_CHAR:
        return Bool(size >= (size_t)ar.as.Char);
    case ARENA_LONG:
        return Bool(size >= (size_t)ar.as.Long);
    case ARENA_DOUBLE:
        return Bool(size >= (size_t)(ar.as.Double));
    case ARENA_SIZE:
        return Bool(size >= ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        exit(1);
        return Bool(false);
    }
}

static arena _access_ints(int *Ints, arena a, int len)
{
    switch (a.type)
    {
    case ARENA_INT:
        if (a.as.Int > len - 1)
            goto ERR;
        return Int(Ints[a.as.Int]);
    case ARENA_LONG:
        if (a.as.Long > len - 1)
            goto ERR;
        return Int(Ints[a.as.Long]);
    case ARENA_BYTE:
        if (a.as.Byte > len - 1)
            goto ERR;
        return Int(Ints[a.as.Byte]);
    case ARENA_CHAR:
        if ((int)a.as.Char > len - 1)
            goto ERR;
        return Int(Ints[(int)a.as.Char]);
    case ARENA_SIZE:
        if ((int)a.as.Size > len - 1)
            goto ERR;
        return Int(Ints[a.as.Size]);
    default:
        log_err("Invalid indexing type.");
        exit(1);
        return Null();
    }

ERR:
    log_err("ERROR: Array index out of bounds.");
    exit(1);
}
static arena _access_longs(long long int *Longs, arena a, int len)
{
    switch (a.type)
    {
    case ARENA_INT:
        if (a.as.Int > len - 1)
            goto ERR;
        return Long(Longs[a.as.Int]);
    case ARENA_LONG:
        if (a.as.Long > len - 1)
            goto ERR;
        return Long(Longs[a.as.Long]);
    case ARENA_BYTE:
        if (a.as.Double > len - 1)
            goto ERR;
        return Long(Longs[a.as.Byte]);
    case ARENA_CHAR:
        if ((int)a.as.Char > len - 1)
            goto ERR;
        return Long(Longs[(int)a.as.Char]);
    case ARENA_SIZE:
        if ((int)a.as.Size > len - 1)
            goto ERR;
        return Long(Longs[a.as.Size]);
    default:
        log_err("Invalid indexing type.");
        exit(1);
        return Null();
    }

ERR:
    log_err("ERROR: Array index out of bounds.");
    exit(1);
}
static arena _access_doubles(double *Doubles, arena a, int len)
{
    switch (a.type)
    {
    case ARENA_INT:
        if (a.as.Int > len - 1)
            goto ERR;
        return Double(Doubles[a.as.Int]);
    case ARENA_LONG:
        if (a.as.Long > len - 1)
            goto ERR;
        return Double(Doubles[a.as.Long]);
    case ARENA_BYTE:
        if (a.as.Byte > len - 1)
            goto ERR;
        return Double(Doubles[a.as.Byte]);
    case ARENA_CHAR:
        if ((int)a.as.Char > len - 1)
            goto ERR;
        return Double(Doubles[(int)a.as.Char]);
    case ARENA_SIZE:
        if ((int)a.as.Size > len - 1)

            goto ERR;
        return Double(Doubles[a.as.Size]);
    default:
        log_err("Invalid indexing type.");
        exit(1);
        return Null();
    }
ERR:
    log_err("ERROR: Array index out of bounds.");
    exit(1);
}
static arena _access_string(char *String, arena a, int len)
{
    switch (a.type)
    {
    case ARENA_INT:
        if (a.as.Int > len - 1)
            goto ERR;
        return Char(String[a.as.Int]);
    case ARENA_LONG:
        if (a.as.Long > len - 1)
            goto ERR;
        return Char(String[a.as.Long]);
    case ARENA_BYTE:
        if (a.as.Byte > len - 1)
            goto ERR;
        return Char(String[a.as.Byte]);
    case ARENA_CHAR:
        if ((int)a.as.Char > len - 1)
            goto ERR;
        return Char(String[(int)a.as.Char]);
    case ARENA_SIZE:
        if ((int)a.as.Size > len - 1)
            goto ERR;
        return Char(String[a.as.Size]);
    default:
        log_err("Invalid indexing type.");
        exit(1);
        return Null();
    }
ERR:
    log_err("ERROR: Array index out of bounds.");
    exit(1);
}
static arena _access_strings(char **Strings, arena a, int len)
{
    switch (a.type)
    {
    case ARENA_INT:
        if (a.as.Int > len - 1)
            goto ERR;
        return CString(Strings[a.as.Int]);
    case ARENA_LONG:
        if (a.as.Long > len - 1)
            goto ERR;
        return CString(Strings[a.as.Long]);
    case ARENA_BYTE:
        if (a.as.Byte > len - 1)
            goto ERR;
        return CString(Strings[a.as.Byte]);
    case ARENA_CHAR:
        if ((int)a.as.Char > len - 1)
            goto ERR;
        return CString(Strings[(int)a.as.Char]);
    case ARENA_SIZE:
        if ((int)a.as.Size > len - 1)
            goto ERR;
        return CString(Strings[a.as.Size]);
    default:
        log_err("Invalid indexing type.");
        exit(1);
        return Null();
    }
ERR:
    log_err("ERROR: Array index out of bounds.");
    exit(1);
}

static arena _access_arena(arena *vect, arena index, int len)
{
    switch (index.type)
    {
    case ARENA_INT:
        if (index.as.Int > len - 1)
            goto ERR;
        return vect[index.as.Int];
    case ARENA_LONG:
        if (index.as.Long > len - 1)
            goto ERR;
        return vect[index.as.Long];
    case ARENA_BYTE:
        if (index.as.Byte > len - 1)
            goto ERR;
        return vect[index.as.Byte];
    case ARENA_CHAR:
        if ((int)index.as.Char > len - 1)
            goto ERR;
        return vect[(int)index.as.Char];
    case ARENA_SIZE:
        if ((int)index.as.Size > len - 1)
            goto ERR;
        return vect[(int)index.as.Size];
    default:
        log_err("Invalid indexing type.");
        exit(1);
        return Null();
    }
ERR:
    log_err("ERROR: Array index out of bounds.");
    exit(1);
}
static element _access_stack(stack *vect, arena index, int len)
{
    switch (index.type)
    {
    case ARENA_INT:
        if (index.as.Int > len - 1)
            goto ERR;
        return vect[index.as.Int].as;
    case ARENA_LONG:
        if (index.as.Long > len - 1)
            goto ERR;
        return vect[index.as.Long].as;
    case ARENA_BYTE:
        if (index.as.Byte > len - 1)
            goto ERR;
        return vect[index.as.Byte].as;
    case ARENA_CHAR:
        if ((int)index.as.Char > len - 1)
            goto ERR;
        return vect[(int)index.as.Char].as;
    case ARENA_SIZE:
        if ((int)index.as.Size > len - 1)
            goto ERR;
        return vect[(int)index.as.Size].as;
    default:
        log_err("Invalid indexing type.");
        exit(1);
    }
ERR:
    log_err("ERROR: Array index out of bounds.");
    exit(1);
}

static void _set_int_index(arena *Ints, arena index, int Int)
{
    int count = Ints->count,
        len = Ints->len;

    switch (index.type)
    {
    case ARENA_INT:
        if (index.as.Int > count - 1 && index.as.Int < len)
            Ints->count = index.as.Int;
        else if (index.as.Int > count - 1 && index.as.Int >= len)
            goto ERR;
        Ints->listof.Ints[index.as.Int] = Int;
        return;
    case ARENA_LONG:
        if (index.as.Long > count - 1 && index.as.Long < len)
            Ints->count = (int)index.as.Long;
        else if (index.as.Long > count - 1 && index.as.Long >= len)
            goto ERR;
        Ints->listof.Ints[index.as.Long] = Int;
        return;
    case ARENA_BYTE:
        if (index.as.Byte > count - 1 && index.as.Byte < len)
            Ints->count = (int)index.as.Byte;
        else if (index.as.Byte > count - 1 && index.as.Byte >= len)
            goto ERR;
        Ints->listof.Ints[index.as.Byte] = Int;
        return;
    case ARENA_CHAR:
        if ((int)index.as.Char > count - 1 && (int)index.as.Char < len)
            Ints->count = (int)index.as.Char;
        else if ((int)index.as.Char > count - 1 && (int)index.as.Char >= len)
            goto ERR;
        Ints->listof.Ints[(int)index.as.Char] = Int;
        return;
    case ARENA_SIZE:
        if ((int)index.as.Size > count - 1 && (int)index.as.Size < len)
            Ints->count = (int)index.as.Size;
        else if ((int)index.as.Size > count - 1 && (int)index.as.Size >= len)
            goto ERR;
        Ints->listof.Ints[(int)index.as.Size] = Int;
        return;
    default:
        log_err("Invalid indexing type.");
        exit(1);
    }
ERR:
    log_err("ERROR: Array index out of bounds.");
    exit(1);
}
static void _set_long_index(arena *Longs, arena index, long long int Long)
{
    int count = Longs->count,
        len = Longs->len;

    switch (index.type)
    {
    case ARENA_INT:
        if (index.as.Int > count - 1 && index.as.Int < len)
            Longs->count = index.as.Int;
        else if (index.as.Int > count - 1 && index.as.Int >= len)
            goto ERR;
        Longs->listof.Longs[index.as.Int] = Long;
        return;
    case ARENA_LONG:
        if (index.as.Long > count - 1 && index.as.Long < len)
            Longs->count = (int)index.as.Long;
        else if (index.as.Long > count - 1 && index.as.Long >= len)
            goto ERR;
        Longs->listof.Longs[index.as.Long] = Long;
        return;
    case ARENA_BYTE:
        if (index.as.Byte > count - 1 && index.as.Byte < len)
            Longs->count = (int)index.as.Byte;
        else if (index.as.Byte > count - 1 && index.as.Byte >= len)
            goto ERR;
        Longs->listof.Longs[index.as.Byte] = Long;
        return;
    case ARENA_CHAR:
        if ((int)index.as.Char > count - 1 && (int)index.as.Char < len)
            Longs->count = (int)index.as.Char;
        else if ((int)index.as.Char > count - 1 && (int)index.as.Char >= len)
            goto ERR;
        Longs->listof.Longs[(int)index.as.Char] = Long;
        return;
    case ARENA_SIZE:
        if ((int)index.as.Size > count - 1 && (int)index.as.Size < len)
            Longs->count = (int)index.as.Size;
        else if ((int)index.as.Size > count - 1 && (int)index.as.Size >= len)
            goto ERR;
        Longs->listof.Longs[(int)index.as.Size] = Long;
        return;
    default:
        log_err("Invalid indexing type.");
        exit(1);
    }
ERR:
    log_err("ERROR: Array index out of bounds.");
    exit(1);
}
static void _set_double_index(arena *Doubles, arena index, double Double)
{
    int count = Doubles->count,
        len = Doubles->len;

    switch (index.type)
    {
    case ARENA_INT:
        if (index.as.Int > count - 1 && index.as.Int < len)
            Doubles->count = index.as.Int;
        else if (index.as.Int > count - 1 && index.as.Int >= len)
            goto ERR;
        Doubles->listof.Doubles[index.as.Int] = Double;
        return;
    case ARENA_LONG:
        if (index.as.Long > count - 1 && index.as.Long < len)
            Doubles->count = (int)index.as.Long;
        else if (index.as.Long > count - 1 && index.as.Long >= len)
            goto ERR;
        Doubles->listof.Doubles[index.as.Long] = Double;
        return;
    case ARENA_BYTE:
        if (index.as.Byte > count - 1 && index.as.Byte < len)
            Doubles->count = (int)index.as.Byte;
        else if (index.as.Byte > count - 1 && index.as.Byte >= len)
            goto ERR;
        Doubles->listof.Doubles[index.as.Byte] = Double;
        return;
    case ARENA_CHAR:
        if ((int)index.as.Char > count - 1 && (int)index.as.Char < len)
            Doubles->count = (int)index.as.Char;
        else if ((int)index.as.Char > count - 1 && (int)index.as.Char >= len)
            goto ERR;
        Doubles->listof.Doubles[(int)index.as.Char] = Double;
        return;
    case ARENA_SIZE:
        if ((int)index.as.Size > count - 1 && (int)index.as.Size < len)
            Doubles->count = (int)index.as.Size;
        else if ((int)index.as.Size > count - 1 && (int)index.as.Size >= len)
            goto ERR;
        Doubles->listof.Doubles[(int)index.as.Size] = Double;
        return;
    default:
        log_err("Invalid indexing type.");
        exit(1);
    }
ERR:
    log_err("ERROR: Array index out of bounds.");
    exit(1);
}
static void _set_string_index(arena *String, arena index, char Char)
{
    int count = String->as.count,
        len = String->as.len;

    switch (index.type)
    {
    case ARENA_INT:
        if (index.as.Int > count - 1 && index.as.Int < len)
            String->as.count = index.as.Int;
        else if (index.as.Int > count - 1 && index.as.Int >= len)
            goto ERR;
        String->as.String[index.as.Int] = Char;
        return;
    case ARENA_LONG:
        if (index.as.Long > count - 1 && index.as.Long < len)
            String->as.count = (int)index.as.Long;
        else if (index.as.Long > count - 1 && index.as.Long >= len)
            goto ERR;
        String->as.String[index.as.Long] = Char;
        return;
    case ARENA_BYTE:
        if (index.as.Byte > count - 1 && index.as.Byte < len)
            String->as.count = (int)index.as.Byte;
        else if (index.as.Byte > count - 1 && index.as.Byte >= len)
            goto ERR;
        String->as.String[index.as.Byte] = Char;
        return;
    case ARENA_CHAR:
        if ((int)index.as.Char > count - 1 && (int)index.as.Char < len)
            String->as.count = (int)index.as.Char;
        else if ((int)index.as.Char > count - 1 && (int)index.as.Char >= len)
            goto ERR;
        String->as.String[(int)index.as.Char] = Char;
        return;
    case ARENA_SIZE:
        if ((int)index.as.Size > count - 1 && (int)index.as.Size < len)
            String->as.count = (int)index.as.Size;
        else if ((int)index.as.Size > count - 1 && (int)index.as.Size >= len)
            goto ERR;
        String->as.String[(int)index.as.Size] = Char;
        return;
    default:
        log_err("Invalid indexing type.");
        exit(1);
    }
ERR:
    log_err("ERROR: Array index out of bounds.");
    exit(1);
}
static void _set_strings_index(arena *Strings, arena index, char *String)
{
    int count = Strings->count,
        len = Strings->len;

    switch (index.type)
    {
    case ARENA_INT:
        if (index.as.Int > count - 1 && index.as.Int < len)
            Strings->count = index.as.Int;
        else if (index.as.Int > count - 1 && index.as.Int >= len)
            goto ERR;
        Strings->listof.Strings[index.as.Int] = String;
        return;
    case ARENA_LONG:
        if (index.as.Long > count - 1 && index.as.Long < len)
            Strings->count = (int)index.as.Long;
        else if (index.as.Long > count - 1 && index.as.Long >= len)
            goto ERR;
        Strings->listof.Strings[index.as.Long] = String;
        return;
    case ARENA_BYTE:
        if (index.as.Byte > count - 1 && index.as.Byte < len)
            Strings->count = (int)index.as.Byte;
        else if (index.as.Byte > count - 1 && index.as.Byte >= len)
            goto ERR;
        Strings->listof.Strings[index.as.Byte] = String;
        return;
    case ARENA_CHAR:
        if ((int)index.as.Char > count - 1 && (int)index.as.Char < len)
            Strings->count = (int)index.as.Char;
        else if ((int)index.as.Char > count - 1 && (int)index.as.Char >= len)
            goto ERR;
        Strings->listof.Strings[(int)index.as.Char] = String;
        return;
    case ARENA_SIZE:
        if ((int)index.as.Size > count - 1 && (int)index.as.Size < len)
            Strings->count = (int)index.as.Size;
        else if ((int)index.as.Size > count - 1 && (int)index.as.Size >= len)
            goto ERR;
        Strings->listof.Strings[(int)index.as.Size] = String;
        return;
    default:
        log_err("Invalid indexing type.");
        exit(1);
    }
ERR:
    log_err("ERROR: Array index out of bounds.");
    exit(1);
}
static void _set_stack_index(stack **vect, arena index, element val)
{
    int len = (*vect)->len,
        count = (*vect)->count;

    switch (index.type)
    {
    case ARENA_INT:
        if (index.as.Int > count - 1 && index.as.Int < len)
            (*vect)->count = index.as.Int + 1;
        else if (index.as.Int > count && index.as.Int >= len)
            goto ERR;
        (*vect)[index.as.Int].as = val;
        return;
    case ARENA_LONG:
        if (index.as.Long > count - 1 && index.as.Long < len)
            (*vect)->count = index.as.Long + 1;
        else if (index.as.Long > count && index.as.Long >= len)
            goto ERR;
        (*vect)[index.as.Long].as = val;
        return;
    case ARENA_BYTE:
        if (index.as.Byte > count - 1 && index.as.Byte < len)
            (*vect)->count = index.as.Byte + 1;
        else if (index.as.Byte > count && index.as.Byte >= len)
            goto ERR;
        (*vect)[index.as.Byte].as = val;
        return;
    case ARENA_CHAR:
        if ((int)index.as.Char > count - 1 && (int)index.as.Char < len)
            (*vect)->count = (int)index.as.Char + 1;
        else if ((int)index.as.Char > count && (int)index.as.Char >= len)
            goto ERR;
        (*vect)[(int)index.as.Char].as = val;
        return;
    case ARENA_SIZE:
        if ((int)index.as.Size > count - 1 && (int)index.as.Size < len)
            (*vect)->count = (int)index.as.Size + 1;
        else if ((int)index.as.Size > count && (int)index.as.Size >= len)
            goto ERR;
        (*vect)[(int)index.as.Size].as = val;
        return;
    default:
        log_err("Invalid indexing type.");
        exit(1);
    }
ERR:
    log_err("ERROR: Array index out of bounds.");
    exit(1);
}

static void _set_arena_index(arena **vect, arena index, arena val)
{
    int len = ((*vect) - 1)->len,
        count = ((*vect) - 1)->count;

    switch (index.type)
    {
    case ARENA_INT:
        if (index.as.Int > count - 1 && index.as.Int < len)
            ((*vect) - 1)->count = index.as.Int + 1;
        else if (index.as.Int > count && index.as.Int >= len)
            goto ERR;
        (*vect)[index.as.Int] = val;
        return;
    case ARENA_LONG:
        if (index.as.Long > count - 1 && index.as.Long < len)
            ((*vect) - 1)->count = index.as.Long + 1;
        else if (index.as.Long > count && index.as.Long >= len)
            goto ERR;
        (*vect)[index.as.Long] = val;
        return;
    case ARENA_BYTE:
        if (index.as.Byte > count - 1 && index.as.Byte < len)
            ((*vect) - 1)->count = index.as.Byte + 1;
        else if (index.as.Byte > count && index.as.Byte >= len)
            goto ERR;
        (*vect)[index.as.Byte] = val;
        return;
    case ARENA_CHAR:
        if ((int)index.as.Char > count - 1 && (int)index.as.Char < len)
            ((*vect) - 1)->count = (int)index.as.Char + 1;
        else if ((int)index.as.Char > count && (int)index.as.Char >= len)
            goto ERR;
        (*vect)[(int)index.as.Char] = val;
        return;
    case ARENA_SIZE:
        if ((int)index.as.Size > count - 1 && (int)index.as.Size < len)
            ((*vect) - 1)->count = (int)index.as.Size + 1;
        else if ((int)index.as.Size > count && (int)index.as.Size >= len)
            goto ERR;
        (*vect)[(int)index.as.Size] = val;
        return;
    default:
        log_err("Invalid indexing type.");
        exit(1);
    }
ERR:
    log_err("ERROR: Array index out of bounds.");
    exit(1);
}

arena _neg(arena n)
{
    arena ar = n;

    switch (ar.type)
    {
    case ARENA_DOUBLE:
        ar.as.Double = -ar.as.Double;
        break;
    case ARENA_INT:
        ar.as.Int = -ar.as.Int;
        break;
    case ARENA_LONG:
        ar.as.Long = -ar.as.Long;
        break;
    case ARENA_BOOL:
        ar.as.Bool = !ar.as.Bool;
        break;
    case ARENA_NULL:
        return Bool(true);
    case ARENA_SIZE:
        ar.as.Size = -ar.as.Size;
        break;
    default:
        return n;
    }
    return ar;
}
arena _add(arena a, arena b)
{

    switch (b.type)
    {
    case ARENA_CHAR:
        return add_arena_char(b.as.Char, a);
    case ARENA_DOUBLE:
        return add_arena_double(b.as.Double, a);
    case ARENA_INT:
        return add_arena_int(b.as.Int, a);
    case ARENA_LONG:
        return add_arena_long(b.as.Long, a);
    case ARENA_STR:
        return append(b, a);
    case ARENA_CSTR:
        return append_to_cstr(b, a);
    case ARENA_SIZE:
        return add_arena_size(b.as.Size, a);
    default:
        return a;
    }
}
arena _inc(arena b)
{
    switch (b.type)
    {
    case ARENA_CHAR:
        return Char(++b.as.Char);
    case ARENA_DOUBLE:
        return Double(++b.as.Double);
    case ARENA_INT:
        return Int(++b.as.Int);
    case ARENA_LONG:
        return Long(++b.as.Long);
    case ARENA_SIZE:
        return Size(++b.as.Size);
    default:
        return b;
    }
}
arena _dec(arena b)
{

    switch (b.type)
    {
    case ARENA_CHAR:
        return Char(--b.as.Char);
    case ARENA_DOUBLE:
        return Double(--b.as.Double);
    case ARENA_INT:
        return Int(--b.as.Int);
    case ARENA_LONG:
        return Long(--b.as.Long);
    case ARENA_SIZE:
        return Size(--b.as.Size);
    default:
        return b;
    }
}

arena _sub(arena a, arena b)
{

    switch (b.type)
    {
    case ARENA_DOUBLE:
        return sub_arena_double(b.as.Double, a);
    case ARENA_INT:
        return sub_arena_int(b.as.Int, a);
    case ARENA_CHAR:
        return sub_arena_char(b.as.Char, a);
    case ARENA_LONG:
        return sub_arena_long(b.as.Long, a);
    case ARENA_SIZE:
        return sub_arena_size(b.as.Size, a);
    default:
        return b;
    }
}
arena _mul(arena a, arena b)
{

    switch (a.type)
    {
    case ARENA_DOUBLE:
        return mul_arena_double(a.as.Double, b);
    case ARENA_CHAR:
        return mul_arena_char(a.as.Char, b);
    case ARENA_INT:
        return mul_arena_int(a.as.Int, b);
    case ARENA_LONG:
        return mul_arena_long(a.as.Long, b);
    case ARENA_SIZE:
        return mul_arena_size(a.as.Size, b);
    default:
        return a;
    }
}
arena _div(arena a, arena b)
{

    switch (b.type)
    {
    case ARENA_CHAR:
        return div_arena_char(b.as.Char, a);
    case ARENA_DOUBLE:
        return div_arena_double(b.as.Double, a);
    case ARENA_INT:
        return div_arena_int(b.as.Int, a);
    case ARENA_LONG:
        return div_arena_long(b.as.Long, a);
    case ARENA_SIZE:
        return div_arena_size(b.as.Size, a);
    default:
        return b;
    }
}
arena _mod(arena a, arena b)
{

    switch (b.type)
    {
    case ARENA_CHAR:
        return mod_arena_char(b.as.Char, a);
    case ARENA_INT:
        return mod_arena_int(b.as.Int, a);
    case ARENA_LONG:
        return mod_arena_long(b.as.Long, a);
    case ARENA_SIZE:
        return mod_arena_size(b.as.Size, a);
    default:
        return b;
    }
}

arena _eq(arena a, arena b)
{

    switch (b.type)
    {
    case ARENA_INT:
        return int_eq(b.as.Int, a);
    case ARENA_DOUBLE:
        return double_eq(b.as.Double, a);
    case ARENA_LONG:
        return long_eq(b.as.Long, a);
    case ARENA_CHAR:
        return char_eq(b.as.Char, a);
    case ARENA_STR:
    case ARENA_CSTR:
        return string_eq(b, a);
    case ARENA_NULL:
        switch (a.type)
        {
        case ARENA_BOOL:
            return Bool(a.as.Bool ? false : true);
        case ARENA_NULL:
            return Bool(a.as.Bool == b.as.Bool);
        default:
            log_err("ERROR: Comparison type mismatch\n");
        }
    case ARENA_BOOL:
        switch (a.type)
        {
        case ARENA_BOOL:
            return Bool(a.as.Bool == b.as.Bool);
        case ARENA_NULL:
            return Bool(b.as.Bool ? false : true);
        default:
            log_err("ERROR: Comparison type mismatch\n");
        }
    case ARENA_SIZE:
        return size_eq(b.as.Size, a);
    default:
        return b;
    }
}
arena _ne(arena a, arena b)
{

    switch (b.type)
    {
    case ARENA_INT:
        return int_ne(b.as.Int, a);
    case ARENA_DOUBLE:
        return double_ne(b.as.Double, a);
    case ARENA_LONG:
        return long_ne(b.as.Long, a);
    case ARENA_CHAR:
        return char_ne(b.as.Char, a);
    case ARENA_CSTR:
    case ARENA_STR:
        return string_ne(b, a);
    case ARENA_NULL:
        switch (a.type)
        {
        case ARENA_BOOL:
            return Bool(a.as.Bool ? true : false);
        case ARENA_NULL:
            return Bool(a.as.Bool != b.as.Bool);
        default:
            log_err("ERROR: Comparison type mismatch\n");
        }
    case ARENA_BOOL:
        switch (a.type)
        {
        case ARENA_BOOL:
            return Bool(a.as.Bool != b.as.Bool);
        case ARENA_NULL:
            return Bool(b.as.Bool ? true : false);
        default:
            log_err("ERROR: Comparison type mismatch\n");
        }
    case ARENA_SIZE:
        return size_ne(b.as.Size, a);
    default:
        return b;
    }
}

arena _seq(arena a, arena b)
{
    if (a.type != b.type)
        return Bool(false);

    switch (b.type)
    {
    case ARENA_INT:
        return Bool(b.as.Int == a.as.Int);
    case ARENA_DOUBLE:
        return Bool(b.as.Double == a.as.Double);
    case ARENA_LONG:
        return Bool(b.as.Long == a.as.Long);
    case ARENA_CHAR:
        return Bool(b.as.Char == a.as.Char);
    case ARENA_CSTR:
    case ARENA_STR:
        return Bool(strcmp(b.as.String, a.as.String) == 0);
    case ARENA_NULL:
        return Bool(false);
    case ARENA_BOOL:
        return Bool(b.as.Bool == a.as.Bool);
    case ARENA_SIZE:
        return Bool(b.as.Size == a.as.Size);
    default:
        return b;
    }
}
arena _sne(arena a, arena b)
{
    if (a.type != b.type)
        return Bool(true);

    switch (b.type)
    {
    case ARENA_INT:
        return Bool(b.as.Int != a.as.Int);
    case ARENA_DOUBLE:
        return Bool(b.as.Double != a.as.Double);
    case ARENA_LONG:
        return Bool(b.as.Long != a.as.Long);
    case ARENA_CHAR:
        return Bool(b.as.Char != a.as.Char);
    case ARENA_CSTR:
    case ARENA_STR:
        return Bool(strcmp(b.as.String, a.as.String) != 0);
    case ARENA_NULL:
        return Bool(false);
    case ARENA_BOOL:
        return Bool(b.as.Bool != a.as.Bool);
    case ARENA_SIZE:
        return Bool(b.as.Size != a.as.Size);
    default:
        return b;
    }
}

arena _lt(arena a, arena b)
{

    switch (b.type)
    {
    case ARENA_INT:
        return int_lt(b.as.Int, a);
    case ARENA_DOUBLE:
        return double_lt(b.as.Double, a);
    case ARENA_LONG:
        return long_lt(b.as.Long, a);
    case ARENA_CHAR:
        return char_lt(b.as.Char, a);
    case ARENA_CSTR:
    case ARENA_STR:
        return string_lt(b, a);
    case ARENA_SIZE:
        return size_lt(b.as.Size, a);
    default:
        return b;
    }
}
arena _le(arena a, arena b)
{

    switch (b.type)
    {
    case ARENA_INT:
        return int_le(b.as.Int, a);
    case ARENA_DOUBLE:
        return double_le(b.as.Double, a);
    case ARENA_LONG:
        return long_le(b.as.Long, a);
    case ARENA_CHAR:
        return char_le(b.as.Char, a);
    case ARENA_CSTR:
    case ARENA_STR:
        return string_le(b, a);
    case ARENA_SIZE:
        return size_le(b.as.Size, a);
    default:
        return b;
    }
}
arena _gt(arena a, arena b)
{

    switch (b.type)
    {
    case ARENA_INT:
        return int_gt(b.as.Int, a);
    case ARENA_DOUBLE:
        return double_gt(b.as.Double, a);
    case ARENA_LONG:
        return long_gt(b.as.Long, a);
    case ARENA_CHAR:
        return char_gt(b.as.Char, a);
    case ARENA_CSTR:
    case ARENA_STR:
        return string_gt(b, a);
    case ARENA_SIZE:
        return size_gt(b.as.Size, a);
    default:
        return b;
    }
}
arena _ge(arena a, arena b)
{

    switch (b.type)
    {
    case ARENA_INT:
        return int_ge(b.as.Int, a);
    case ARENA_DOUBLE:
        return double_ge(b.as.Double, a);
    case ARENA_LONG:
        return long_ge(b.as.Long, a);
    case ARENA_CHAR:
        return char_ge(b.as.Char, a);
    case ARENA_CSTR:
    case ARENA_STR:
        return string_ge(b, a);
    case ARENA_SIZE:
        return size_ge(b.as.Size, a);
    default:
        return b;
    }
}

arena _or(arena a, arena b)
{
    return Bool(a.as.Bool || b.as.Bool);
}
arena _and(arena a, arena b)
{
    return Bool(b.as.Bool && a.as.Bool);
}

element _get_each_access(element b, int index)
{
    element nil = null_obj();
    switch (b.type)
    {
    case ARENA:
        switch (b._arena.type)
        {
        case ARENA_INTS:
            if (index > b._arena.count - 1)
                return nil;
            return OBJ(Int(*(b._arena.listof.Ints + index)));
        case ARENA_LONGS:
            if (index > b._arena.count - 1)
                return nil;
            return OBJ(Long(*(b._arena.listof.Longs + index)));
        case ARENA_DOUBLES:
            if (index > b._arena.count - 1)
                return nil;
            return OBJ(Double(*(b._arena.listof.Doubles + index)));
        case ARENA_STR:
        case ARENA_CSTR:
            if (index > b._arena.count - 1)
                return nil;
            return OBJ(Char(*(b._arena.as.String + index)));
        case ARENA_STRS:
            if (index > b._arena.count - 1)
                return nil;
            return OBJ(CString(*(b._arena.listof.Strings + index)));
        default:
            goto ERR;
        }
    case VECTOR:
        if (index > (b._vector - 1)->count - 1)
            return nil;
        return OBJ(*(b._vector + index));
    case STACK:
        if (index > b.stack->count - 1)
            return nil;
        return b.stack[index].as;
    default:
    ERR:
        log_err("ERROR: access type mismatch.");
        return OBJ(Null());
    }
}

element _get_access(element a, element b)
{
    switch (b.type)
    {
    case ARENA:
        switch (b._arena.type)
        {
        case ARENA_INTS:
            return OBJ(_access_ints(b._arena.listof.Ints, a._arena, b._arena.count));
        case ARENA_LONGS:
            return OBJ(_access_longs(b._arena.listof.Longs, a._arena, b._arena.count));
        case ARENA_DOUBLES:
            return OBJ(_access_doubles(b._arena.listof.Doubles, a._arena, b._arena.count));
        case ARENA_STR:
        case ARENA_CSTR:
            return OBJ(_access_string(b._arena.as.String, a._arena, b._arena.count));
        case ARENA_STRS:
            return OBJ(_access_strings(b._arena.listof.Strings, a._arena, b._arena.count));
        default:
            goto ERR;
        }
    case TABLE:
        if (a.type != ARENA)
            goto ERR;
        return find_entry(&b.table, &a._arena);

    case VECTOR:
        return OBJ(_access_arena(b._vector, a._arena, (b._vector - 1)->count));
    case STACK:
        return _access_stack(b.stack, a._arena, b.stack->count);
    default:
    ERR:
        log_err("ERROR: access type mismatch.");
        return null_obj();
    }
}
void _set_access(element val, arena index, element b)
{
    switch (b.type)
    {
    case ARENA:
    {
        arena ar = b._arena;
        switch (ar.type)
        {
        case ARENA_INTS:
            if (val._arena.type != ARENA_INT)
                goto ERR;
            _set_int_index(&ar, index, val._arena.as.Int);
            return;
        case ARENA_LONGS:
            if (val._arena.type != ARENA_LONG)
                goto ERR;
            _set_long_index(&ar, index, val._arena.as.Long);
            return;
        case ARENA_DOUBLES:
            if (val._arena.type != ARENA_DOUBLE)
                goto ERR;
            _set_double_index(&ar, index, val._arena.as.Double);
            return;
        case ARENA_STR:
        case ARENA_CSTR:
            if (val._arena.type != ARENA_CHAR)
                goto ERR;
            _set_string_index(&ar, index, val._arena.as.Char);
            return;
        case ARENA_STRS:
            if (val._arena.type != ARENA_STR && val._arena.type != ARENA_CSTR)
                goto ERR;
            _set_strings_index(&ar, index, val._arena.as.String);
            return;
        default:
            break;
        }
        break;
    }
    case TABLE:
        write_table(b.table, index, val);
        break;
    case CLOSURE:
    case INSTANCE:
        break;
    case VECTOR:
        if (val.type != ARENA)
            goto ERR;
        _set_arena_index(&b._vector, index, val._arena);
        break;
    case STACK:
        _set_stack_index(&b.stack, index, val);
        break;
    default:
    ERR:
        log_err("ERROR: access type mismatch.");
        exit(1);
    }
}

element _push_array_val(element val, element el)
{
    switch (el.type)
    {
    case ARENA:
    {
        arena ar = val._arena;
        if (val.type != ARENA)
            goto ERR;
        switch (el._arena.type)
        {
        case ARENA_INTS:
            if (ar.type != ARENA_INT)
                goto ERR;
            push_int(&el, ar.as.Int);
            return el;
        case ARENA_LONGS:
            if (ar.type != ARENA_LONG)
                goto ERR;
            push_long(&el, ar.as.Long);
            return el;
        case ARENA_DOUBLES:
            if (ar.type != ARENA_DOUBLE)
                goto ERR;
            push_double(&el, ar.as.Double);
            return el;
        case ARENA_STRS:
            if (ar.type != ARENA_STR && ar.type != ARENA_CSTR)
                goto ERR;
            push_string(&el, ar.as.String);
            return el;
        default:
            goto ERR;
        }
    }
    case VECTOR:
        push_arena(&el, val._arena);
        return el;
    case STACK:

        push(&el.stack, val);
        return el;
    default:
    ERR:
        log_err("ERROR: pushing to invalid data structure.");
        return null_obj();
    }
}
element _pop_array_val(element val)
{
    switch (val.type)
    {
    case ARENA:
    {
        arena ar = val._arena;
        if (val.type != ARENA)
            goto ERR;
        switch (ar.type)
        {
        case ARENA_INTS:
            return pop_int(&val);
        case ARENA_LONGS:
            return pop_long(&val);
        case ARENA_DOUBLES:
            return pop_double(&val);
        case ARENA_STRS:
            return pop_string(&val);
        default:
            goto ERR;
        }
        break;
    }
    case VECTOR:
        return pop_arena(&val);
    case STACK:
        --val.stack->count;
        return (--val.stack->top)->as;
    default:
    ERR:
        return null_obj();
    }
}

arena _len(element el)
{
    switch (el.type)
    {
    case ARENA:
    {
        arena a = el._arena;
        switch (a.type)
        {
        case ARENA_INTS:
        case ARENA_LONGS:
        case ARENA_DOUBLES:
        case ARENA_STRS:
            return Int(a.count);
        case ARENA_CSTR:
        case ARENA_STR:
            return Int(a.as.len);
        default:
            return Int(0);
        }
    }
    case VECTOR:
        return Int((el._vector - 1)->count);
    case STACK:
        return Int(el.stack->count);
    default:
        return Int(0);
    }
}

arena _sqr(arena a)
{
    switch (a.type)
    {
    case ARENA_INT:
        return Double(sqrt(a.as.Int));
    case ARENA_DOUBLE:
        return Double(sqrt(a.as.Double));
    case ARENA_LONG:
        return Double(sqrt(a.as.Long));
    case ARENA_BYTE:
        return Double(sqrt(a.as.Byte));
    case ARENA_CHAR:
        return Double(sqrt(a.as.Char));
    case ARENA_SIZE:
        return Double(sqrt(a.as.Size));
    default:
        return Int(0);
    }
}

arena _prime(arena a)
{
    double max = 0;
    switch (a.type)
    {
    case ARENA_INT:
        max = sqrt(a.as.Int);
        for (int i = 2; i < max; i++)
            if (a.as.Int % i == 0)
                return Bool(false);
        break;
    case ARENA_DOUBLE:
        max = sqrt(a.as.Double);
        for (int i = 2; i < max; i++)
            if ((int)a.as.Double % i == 0)
                return Bool(false);
        break;
    case ARENA_LONG:
        max = sqrt(a.as.Long);
        for (int i = 2; i < max; i++)
            if (a.as.Long % i == 0)
                return Bool(false);
        break;
    case ARENA_BYTE:
        max = sqrt(a.as.Long);
        for (int i = 2; i < max; i++)
            if (a.as.Byte % i == 0)
                return Bool(false);
        break;
    case ARENA_CHAR:
        max = sqrt(a.as.Long);
        for (int i = 2; i < max; i++)
            if (a.as.Char % i == 0)
                return Bool(false);
        break;
    default:
        return Bool(true);
    }
    return Bool(true);
}
