#include "arena_math.h"
#include <stdio.h>
#include <stdarg.h>

static void log_err(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
}
static arena Num(long long int range)
{
    if (range < INT32_MAX && range > INT32_MIN)
        return Int((int)range);
    if (range < INT64_MAX && range > INT64_MIN)
        return Long(range);
    log_err("ERROR: numeric value out of range");
    return Null();
}

arena add_arena_char(char ch, arena ar)
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
    }
    return ar;
}
arena sub_arena_char(char ch, arena ar)
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
    default:
        log_err("ERROR: subtraction type mismatch\n");
    }
    return ar;
}
arena mul_arena_char(char ch, arena ar)
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
    default:
        log_err("ERROR: multiplication type mismatch\n");
    }
    return ar;
}
arena div_arena_char(char ch, arena ar)
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
    default:
        log_err("ERROR: division type mismatch\n");
    }
ERR:
    log_err("ERROR: divide by zero\n");

    return ar;
}
arena mod_arena_char(char ch, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Int(ch % ar.as.Char);
    case ARENA_CHAR:
        return Char(ch % ar.as.Char);
    case ARENA_LONG:
        return Num((long long int)ch % ar.as.Long);
    default:
        log_err("ERROR: modulo type mismatch\n");
    }
    return ar;
}

arena add_arena_int(int ival, arena ar)
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
    default:
        log_err("ERROR: addition type mismatch\n");
    }
    return ar;
}
arena sub_arena_int(int ival, arena ar)
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
    default:
        log_err("ERROR: subtraction type mismatch\n");
    }
    return ar;
}
arena mul_arena_int(int ival, arena ar)
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
    default:
        log_err("ERROR: multiplication type mismatch\n");
    }
    return ar;
}
arena div_arena_int(int ival, arena ar)
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
    default:
        log_err("ERROR: division type mismatch\n");
    }
ERR:
    log_err("ERROR: divide by zero\n");

    return ar;
}
arena mod_arena_int(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Int(ival % ar.as.Int);
    case ARENA_CHAR:
        return Char(ival % ar.as.Char);
    case ARENA_LONG:
        return Long((long long int)ival % ar.as.Long);
    default:
        log_err("ERROR: modulo type mismatch\n");
    }
    return ar;
}

arena add_arena_long(long long int llint, arena ar)
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
    default:
        log_err("ERROR: subtraction type mismatch\n");
    }
    return ar;
}
arena sub_arena_long(long long int llint, arena ar)
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
    default:
        log_err("ERROR: subtraction type mismatch\n");
    }
    return ar;
}
arena mul_arena_long(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_CHAR:
        return Num(llint * ar.as.Char);
    case ARENA_INT:
        return Num(llint * ar.as.Int);
    case ARENA_DOUBLE:
        return Double((double)llint * ar.as.Double);
    case ARENA_LONG:
        return Num(llint * ar.as.Long);
    default:
        log_err("ERROR: multiplication type mismatch\n");
    }
    return ar;
}
arena div_arena_long(long long int llint, arena ar)
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
    default:
        log_err("ERROR: division type mismatch\n");
    }
ERR:
    log_err("ERROR: divide by zero\n");

    return ar;
}
arena mod_arena_long(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT:
        return Int(llint % ar.as.Int);
    case ARENA_CHAR:
        return Char(llint % ar.as.Char);
    case ARENA_LONG:
        return Long(llint % ar.as.Long);
    default:
        log_err("ERROR: modulo type mismatch\n");
    }
    return ar;
}

arena add_arena_double(double dval, arena ar)
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
    default:
        log_err("ERROR: addition type mismatch\n");
    }
    return ar;
}
arena sub_arena_double(double dval, arena ar)
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
    default:
        log_err("ERROR: subtraction type mismatch\n");
    }
    return ar;
}
arena mul_arena_double(double dval, arena ar)
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
    default:
        log_err("ERROR: multiplication type mismatch\n");
    }
    return ar;
}
arena div_arena_double(double dval, arena ar)
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
    default:
        log_err("ERROR: division type mismatch\n");
    }
ERR:
    log_err("ERROR: divide by zero\n");

    return ar;
}

arena char_eq(char ch, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch\n");
    }
    return Bool(false);
}
arena char_ne(char ch, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch\n");
    }
    return Bool(false);
}
arena char_lt(char ch, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return Bool(false);
}
arena char_le(char ch, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }

    return Bool(false);
}
arena char_gt(char ch, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }

    return Bool(false);
}
arena char_ge(char ch, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return Bool(false);
}

arena int_eq(int ival, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return Bool(false);
}
arena int_ne(int ival, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return Bool(false);
}
arena int_lt(int ival, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return Bool(false);
}
arena int_le(int ival, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return Bool(false);
}
arena int_gt(int ival, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return Bool(false);
}
arena int_ge(int ival, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return Bool(false);
}

arena long_eq(long long int llint, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return Bool(false);
}
arena long_ne(long long int llint, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return Bool(false);
}
arena long_lt(long long int llint, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return Bool(false);
}
arena long_le(long long int llint, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return Bool(false);
}
arena long_gt(long long int llint, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return Bool(false);
}
arena long_ge(long long int llint, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return Bool(false);
}

arena double_eq(double dval, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return Bool(false);
}
arena double_ne(double dval, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return Bool(false);
}
arena double_lt(double dval, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return Bool(false);
}
arena double_le(double dval, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return Bool(false);
}
arena double_gt(double dval, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return Bool(false);
}
arena double_ge(double dval, arena ar)
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
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return Bool(false);
}
