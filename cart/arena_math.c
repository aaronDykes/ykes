#include "arena_math.h"
#include "arena_math_util.h"
#include "arena_string.h"
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

static Arena Num(long long int range)
{
    if (range < INT32_MAX && range > INT32_MIN)
        return Int((int)range);
    if (range < INT64_MAX && range > INT64_MIN)
        return Long(range);
    log_err("ERROR: numeric value out of range");
    return Null();
}

static Arena add_arena_size(size_t size, Arena ar)
{
    switch (ar.type)
    {
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
        return ar;
    }
}
static Arena sub_arena_size(size_t size, Arena ar)
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
    }
    return ar;
}
static Arena mul_arena_size(size_t size, Arena ar)
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
        return ar;
    }
}
static Arena div_arena_size(size_t size, Arena ar)
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
        return ar;
    }
ERR:
    log_err("ERROR: divide by zero\n");

    return ar;
}
static Arena mod_arena_size(size_t size, Arena ar)
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
        return ar;
    }
}

static Arena add_arena_char(char ch, Arena ar)
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
    case ARENA_STR:
        return prepend_char_to_str(Char(ch), ar);
    case ARENA_SIZE:
        return Num(ch + ar.as.Size);
    default:
        return ar;
    }
}
static Arena sub_arena_char(char ch, Arena ar)
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
    }
    return ar;
}
static Arena mul_arena_char(char ch, Arena ar)
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
    }
    return ar;
}
static Arena div_arena_char(char ch, Arena ar)
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
    }
ERR:
    log_err("ERROR: divide by zero\n");

    return ar;
}
static Arena mod_arena_char(char ch, Arena ar)
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
    }
    return ar;
}

static Arena add_arena_int(int ival, Arena ar)
{
    switch (ar.type)
    {
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
    }
    return ar;
}
static Arena sub_arena_int(int ival, Arena ar)
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
    }
    return ar;
}
static Arena mul_arena_int(int ival, Arena ar)
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
    }
    return ar;
}
static Arena div_arena_int(int ival, Arena ar)
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
    }
ERR:
    log_err("ERROR: divide by zero\n");

    return ar;
}
static Arena mod_arena_int(int ival, Arena ar)
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
    }
    return ar;
}

static Arena add_arena_long(long long int llint, Arena ar)
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
    case ARENA_STR:
        return prepend_long_to_str(Long(llint), ar);
    case ARENA_SIZE:
        return Num(llint + ar.as.Size);
    default:
        log_err("ERROR: subtraction type mismatch\n");
    }
    return ar;
}
static Arena sub_arena_long(long long int llint, Arena ar)
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
    }
    return ar;
}
static Arena mul_arena_long(long long int llint, Arena ar)
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
        return Null();
    }
    return ar;
}
static Arena div_arena_long(long long int llint, Arena ar)
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
        return Null();
    }
ERR:
    log_err("ERROR: divide by zero\n");
    return Null();
}
static Arena mod_arena_long(long long int llint, Arena ar)
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
        return ar;
    }
}

static Arena add_arena_double(double dval, Arena ar)
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
        return ar;
    }
}
static Arena sub_arena_double(double dval, Arena ar)
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
        return ar;
    }
}
static Arena mul_arena_double(double dval, Arena ar)
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
    }
    return ar;
}
static Arena div_arena_double(double dval, Arena ar)
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
        return ar;
    }
ERR:
    log_err("ERROR: divide by zero\n");

    return ar;
}

static Arena char_eq(char ch, Arena ar)
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
    }
    return Bool(false);
}
static Arena char_ne(char ch, Arena ar)
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
    }
    return Bool(false);
}
static Arena char_lt(char ch, Arena ar)
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
        return Bool(false);
    }
}
static Arena char_le(char ch, Arena ar)
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
        return Bool(false);
    }
}
static Arena char_gt(char ch, Arena ar)
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
        return Bool(false);
    }
}
static Arena char_ge(char ch, Arena ar)
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
        return Bool(false);
    }
}

static Arena int_eq(int ival, Arena ar)
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
    case ARENA_STR:
        return itoa_eqcmp(ival, ar);
    case ARENA_SIZE:
        return Bool(ival == (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        return Bool(false);
    }
}
static Arena int_ne(int ival, Arena ar)
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
    case ARENA_STR:
        return itoa_neqcmp(ival, ar);
    case ARENA_SIZE:
        return Bool(ival != (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        return Bool(false);
    }
}
static Arena int_lt(int ival, Arena ar)
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
        return Bool(false);
    }
}
static Arena int_le(int ival, Arena ar)
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
        return Bool(false);
    }
}
static Arena int_gt(int ival, Arena ar)
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
        return Bool(false);
    }
}
static Arena int_ge(int ival, Arena ar)
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
        return Bool(false);
    }
}

static Arena long_eq(long long int llint, Arena ar)
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
    case ARENA_STR:
        return ltoa_eqcmp(llint, ar);
    case ARENA_SIZE:
        return Bool(llint == (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        return Bool(false);
    }
}
static Arena long_ne(long long int llint, Arena ar)
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
    case ARENA_STR:
        return ltoa_neqcmp(llint, ar);
    case ARENA_SIZE:
        return Bool(llint != (int)ar.as.Size);
    default:
        log_err("ERROR: comparison type mismatch");
        return Bool(false);
    }
}
static Arena long_lt(long long int llint, Arena ar)
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
        return Bool(false);
    }
}
static Arena long_le(long long int llint, Arena ar)
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
        return Bool(false);
    }
}
static Arena long_gt(long long int llint, Arena ar)
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
        return Bool(false);
    }
}
static Arena long_ge(long long int llint, Arena ar)
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
        return Bool(false);
    }
}

static Arena double_eq(double dval, Arena ar)
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
        return Bool(false);
    }
}
static Arena double_ne(double dval, Arena ar)
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
        return Bool(false);
    }
}
static Arena double_lt(double dval, Arena ar)
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
        return Bool(false);
    }
}
static Arena double_le(double dval, Arena ar)
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
        return Bool(false);
    }
}
static Arena double_gt(double dval, Arena ar)
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
        return Bool(false);
    }
}
static Arena double_ge(double dval, Arena ar)
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
        return Bool(false);
    }
}

static Arena size_eq(size_t size, Arena ar)
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
        return Bool(false);
    }
}
static Arena size_ne(size_t size, Arena ar)
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
        return Bool(false);
    }
}
static Arena size_lt(size_t size, Arena ar)
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
        return Bool(false);
    }
}
static Arena size_le(size_t size, Arena ar)
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
        return Bool(false);
    }
}

static Arena size_gt(size_t size, Arena ar)
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
        return Bool(false);
    }
}
static Arena size_ge(size_t size, Arena ar)
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
        return Bool(false);
    }
}

Arena _neg(Arena n)
{
    Arena ar = n;

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
Arena _add(Arena a, Arena b)
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
    case ARENA_SIZE:
        return add_arena_size(b.as.Size, a);
    default:
        return a;
    }
}
Arena _inc(Arena b)
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
Arena _dec(Arena b)
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

Arena _sub(Arena a, Arena b)
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
Arena _mul(Arena a, Arena b)
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
Arena _div(Arena a, Arena b)
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
Arena _mod(Arena a, Arena b)
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

Arena _eq(Arena a, Arena b)
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
Arena _ne(Arena a, Arena b)
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

Arena _seq(Arena a, Arena b)
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
Arena _sne(Arena a, Arena b)
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

Arena _lt(Arena a, Arena b)
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
    case ARENA_STR:
        return string_lt(b, a);
    case ARENA_SIZE:
        return size_lt(b.as.Size, a);
    default:
        return b;
    }
}
Arena _le(Arena a, Arena b)
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
    case ARENA_STR:
        return string_le(b, a);
    case ARENA_SIZE:
        return size_le(b.as.Size, a);
    default:
        return b;
    }
}
Arena _gt(Arena a, Arena b)
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
    case ARENA_STR:
        return string_gt(b, a);
    case ARENA_SIZE:
        return size_gt(b.as.Size, a);
    default:
        return b;
    }
}
Arena _ge(Arena a, Arena b)
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
    case ARENA_STR:
        return string_ge(b, a);
    case ARENA_SIZE:
        return size_ge(b.as.Size, a);
    default:
        return b;
    }
}

Arena _or(Arena a, Arena b)
{
    return Bool(a.as.Bool || b.as.Bool);
}
Arena _and(Arena a, Arena b)
{
    return Bool(b.as.Bool && a.as.Bool);
}

Arena _sqr(Arena a)
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

Arena _prime(Arena a)
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