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
static arena arena_num(long long int range)
{
    if (range < INT32_MAX && range > INT32_MIN)
        return arena_int((int)range);
    if (range < INT64_MAX && range > INT64_MIN)
        return arena_llint(range);
    log_err("ERROR: numeric value out of range");
    return arena_null();
}

arena add_arena_char(char ch, arena ar)
{
    long long int test = 0;
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        test = (long long int)(ch + ar.as.ival);
        return (test < 255 && test > 0) ? arena_char((char)test) : arena_num(test);
    case ARENA_CHAR_CONST:
        test = (long long int)(ch + ar.as.ch);
        return (test < 255 && test > 0) ? arena_char((char)test) : arena_int((int)test);
    case ARENA_DOUBLE_CONST:
        return arena_double((double)ch + ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_num(ch + ar.as.llint);
    case ARENA_STR:
        return prepend_char_to_str(arena_char(ch), ar);
    }
    return ar;
}
arena sub_arena_char(char ch, arena ar)
{
    long long int test = 0;
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        test = (ch - ar.as.ival);
        return (test > 0) ? arena_char((char)test) : arena_int((int)test);
    case ARENA_CHAR_CONST:
        test = (ch - ar.as.ch);
        return (test > 0) ? arena_char((char)test) : arena_int((int)test);
    case ARENA_DOUBLE_CONST:
        return arena_double((double)ch - ar.as.dval);
    case ARENA_LLINT_CONST:
        test = (ch - ar.as.llint);
        return (test > 0) ? arena_char((char)test) : arena_num(test);
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
    case ARENA_INT_CONST:
        test = (ch * ar.as.ival);
        return (test < 255 && test > 0) ? arena_char((char)test) : arena_num(test);
    case ARENA_CHAR_CONST:
        test = (ch * ar.as.ch);
        return (test < 255 && test > 0) ? arena_char((char)test) : arena_int((int)test);
    case ARENA_DOUBLE_CONST:
        return arena_double((double)ch * ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_num(ch * ar.as.llint);
    default:
        log_err("ERROR: multiplication type mismatch\n");
    }
    return ar;
}
arena div_arena_char(char ch, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        if (ar.as.ival == 0)
            goto ERR;
        return arena_double((double)ch / ar.as.ival);
    case ARENA_CHAR_CONST:
        if (ar.as.ch == 0)
            goto ERR;
        return arena_double((double)ch / ar.as.ch);
    case ARENA_DOUBLE_CONST:
        if (ar.as.dval == 0)
            goto ERR;
        return arena_double((double)ch / ar.as.dval);
    case ARENA_LLINT_CONST:
        if (ar.as.llint == 0)
            goto ERR;
        return arena_double((double)ch / ar.as.llint);
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
    case ARENA_INT_CONST:
        return arena_int(ch % ar.as.ch);
    case ARENA_CHAR_CONST:
        return arena_char(ch % ar.as.ch);
    case ARENA_LLINT_CONST:
        return arena_num((long long int)ch % ar.as.llint);
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
        return prepend_int_to_str(arena_int(ival), ar);
    case ARENA_INT_CONST:
        return arena_num(ival + ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_double((double)ival + ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_num(ival + ar.as.llint);
    case ARENA_CHAR_CONST:
        return arena_num(ival + ar.as.ch);
    default:
        log_err("ERROR: addition type mismatch\n");
    }
    return ar;
}
arena sub_arena_int(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_num(ival - ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_double((double)ival - ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_num(ival - ar.as.llint);
    case ARENA_CHAR_CONST:
        return arena_num(ival - ar.as.ch);
    default:
        log_err("ERROR: subtraction type mismatch\n");
    }
    return ar;
}
arena mul_arena_int(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_num(ival * ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_double((double)ival * ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_num(ival * ar.as.llint);
    case ARENA_CHAR_CONST:
        return arena_num(ival * ar.as.ch);
    default:
        log_err("ERROR: multiplication type mismatch\n");
    }
    return ar;
}
arena div_arena_int(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        if (ar.as.ival == 0)
            goto ERR;
        return arena_double((double)ival / ar.as.ival);
    case ARENA_CHAR_CONST:
        if (ar.as.ch == 0)
            goto ERR;
        return arena_double((double)ival / ar.as.ch);
    case ARENA_DOUBLE_CONST:
        if (ar.as.dval == 0)
            goto ERR;
        return arena_double((double)ival / ar.as.dval);
    case ARENA_LLINT_CONST:
        if (ar.as.llint == 0)
            goto ERR;
        return arena_double((double)ival / ar.as.llint);
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
    case ARENA_INT_CONST:
        return arena_int(ival % ar.as.ival);
    case ARENA_CHAR_CONST:
        return arena_char(ival % ar.as.ch);
    case ARENA_LLINT_CONST:
        return arena_llint((long long int)ival % ar.as.llint);
    default:
        log_err("ERROR: modulo type mismatch\n");
    }
    return ar;
}

arena add_arena_llint(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_CHAR_CONST:
        return arena_num(llint + ar.as.ch);
    case ARENA_INT_CONST:
        return arena_num(llint + ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_double((double)llint + ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_num(llint + ar.as.llint);
    case ARENA_STR:
        return prepend_llint_to_str(arena_llint(llint), ar);
    default:
        log_err("ERROR: subtraction type mismatch\n");
    }
    return ar;
}
arena sub_arena_llint(long long int llint, arena ar)
{

    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_llint(llint - ar.as.ival);
    case ARENA_CHAR_CONST:
        return arena_llint(llint - ar.as.ch);
    case ARENA_DOUBLE_CONST:
        return arena_double((double)llint - ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_llint(llint - ar.as.llint);
    default:
        log_err("ERROR: subtraction type mismatch\n");
    }
    return ar;
}
arena mul_arena_llint(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_CHAR_CONST:
        return arena_num(llint * ar.as.ch);
    case ARENA_INT_CONST:
        return arena_num(llint * ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_double((double)llint * ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_num(llint * ar.as.llint);
    default:
        log_err("ERROR: multiplication type mismatch\n");
    }
    return ar;
}
arena div_arena_llint(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        if (ar.as.ival == 0)
            goto ERR;
        return arena_double((double)llint / ar.as.ival);
    case ARENA_CHAR_CONST:
        if (ar.as.ch == 0)
            goto ERR;
        return arena_double((double)llint / ar.as.ch);
    case ARENA_DOUBLE_CONST:
        if (ar.as.dval == 0)
            goto ERR;
        return arena_double((double)llint / ar.as.dval);
    case ARENA_LLINT_CONST:
        if (ar.as.llint == 0)
            goto ERR;
        return arena_double((double)llint / ar.as.llint);
    default:
        log_err("ERROR: division type mismatch\n");
    }
ERR:
    log_err("ERROR: divide by zero\n");

    return ar;
}
arena mod_arena_llint(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_int(llint % ar.as.ival);
    case ARENA_CHAR_CONST:
        return arena_char(llint % ar.as.ch);
    case ARENA_LLINT_CONST:
        return arena_llint(llint % ar.as.llint);
    default:
        log_err("ERROR: modulo type mismatch\n");
    }
    return ar;
}

arena add_arena_double(double dval, arena ar)
{
    switch (ar.type)
    {
    case ARENA_CHAR_CONST:
        return arena_double(dval + ar.as.ch);
    case ARENA_INT_CONST:
        return arena_double(dval + ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_double(dval + ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_double(dval + ar.as.llint);
    default:
        log_err("ERROR: addition type mismatch\n");
    }
    return ar;
}
arena sub_arena_double(double dval, arena ar)
{
    switch (ar.type)
    {
    case ARENA_CHAR_CONST:
        return arena_double(dval - ar.as.ch);
    case ARENA_INT_CONST:
        return arena_double(dval - ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_double(dval - ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_double(dval - ar.as.llint);
    default:
        log_err("ERROR: subtraction type mismatch\n");
    }
    return ar;
}
arena mul_arena_double(double dval, arena ar)
{
    switch (ar.type)
    {
    case ARENA_CHAR_CONST:
        return arena_double(dval * ar.as.ch);
    case ARENA_INT_CONST:
        return arena_double(dval * ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_double(dval * ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_double(dval * ar.as.llint);
    default:
        log_err("ERROR: multiplication type mismatch\n");
    }
    return ar;
}
arena div_arena_double(double dval, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        if (ar.as.ival == 0)
            goto ERR;
        return arena_double(dval / ar.as.ival);
    case ARENA_CHAR_CONST:
        if (ar.as.ch == 0)
            goto ERR;
        return arena_double(dval / ar.as.ch);
    case ARENA_DOUBLE_CONST:
        if (ar.as.dval == 0)
            goto ERR;
        return arena_double(dval / ar.as.dval);
    case ARENA_LLINT_CONST:
        if (ar.as.llint == 0)
            goto ERR;
        return arena_double(dval / ar.as.llint);
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
    case ARENA_INT_CONST:
        return arena_bool(ch == ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_bool(ch == ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_bool(ch == ar.as.llint);
    case ARENA_CHAR_CONST:
        return arena_bool(ch == ar.as.ch);
    case ARENA_NULL:
        return arena_bool(ch == '\0');
    default:
        log_err("ERROR: comparison type mismatch\n");
    }
    return arena_bool(false);
}
arena char_ne(char ch, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(ch != ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_bool(ch != ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_bool(ch != ar.as.llint);
    case ARENA_CHAR_CONST:
        return arena_bool(ch != ar.as.ch);
    case ARENA_NULL:
        return arena_bool(ch != '\0');
    default:
        log_err("ERROR: comparison type mismatch\n");
    }
    return arena_bool(false);
}
arena char_lt(char ch, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(ch < ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_bool(ch < ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_bool(ch < ar.as.llint);
    case ARENA_CHAR_CONST:
        return arena_bool(ch < ar.as.ch);
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return arena_bool(false);
}
arena char_le(char ch, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(ch <= ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_bool(ch <= ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_bool(ch <= ar.as.llint);
    case ARENA_CHAR_CONST:
        return arena_bool(ch <= ar.as.ch);
    default:
        log_err("ERROR: comparison type mismatch");
    }

    return arena_bool(false);
}
arena char_gt(char ch, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(ch > ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_bool(ch > ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_bool(ch > ar.as.llint);
    case ARENA_CHAR_CONST:
        return arena_bool(ch > ar.as.ch);
    default:
        log_err("ERROR: comparison type mismatch");
    }

    return arena_bool(false);
}
arena char_ge(char ch, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(ch >= ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_bool(ch >= ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_bool(ch >= ar.as.llint);
    case ARENA_CHAR_CONST:
        return arena_bool(ch >= ar.as.ch);
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return arena_bool(false);
}

arena int_eq(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(ival == ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_bool(ival == ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_bool(ival == ar.as.llint);
    case ARENA_CHAR_CONST:
        return arena_bool(ival == ar.as.ch);
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return arena_bool(false);
}
arena int_ne(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(ival != ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_bool(ival != ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_bool(ival != ar.as.llint);
    case ARENA_CHAR_CONST:
        return arena_bool(ival != ar.as.ch);
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return arena_bool(false);
}
arena int_lt(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(ival < ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_bool(ival < ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_bool(ival < ar.as.llint);
    case ARENA_CHAR_CONST:
        return arena_bool(ival < ar.as.ch);
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return arena_bool(false);
}
arena int_le(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(ival <= ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_bool(ival <= ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_bool(ival <= ar.as.llint);
    case ARENA_CHAR_CONST:
        return arena_bool(ival <= ar.as.ch);
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return arena_bool(false);
}
arena int_gt(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(ival > ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_bool(ival > ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_bool(ival > ar.as.llint);
    case ARENA_CHAR_CONST:
        return arena_bool(ival > ar.as.ch);
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return arena_bool(false);
}
arena int_ge(int ival, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(ival >= ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_bool(ival >= ar.as.dval);
    case ARENA_LLINT_CONST:
        return arena_bool(ival >= ar.as.llint);
    case ARENA_CHAR_CONST:
        return arena_bool(ival >= ar.as.ch);
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return arena_bool(false);
}

arena llint_eq(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(llint == ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_bool(llint == ar.as.dval);
    case ARENA_CHAR_CONST:
        return arena_bool(llint == ar.as.ch);
    case ARENA_LLINT_CONST:
        return arena_bool(llint == ar.as.llint);
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return arena_bool(false);
}
arena llint_ne(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(llint != ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_bool(llint != ar.as.dval);
    case ARENA_CHAR_CONST:
        return arena_bool(llint != ar.as.ch);
    case ARENA_LLINT_CONST:
        return arena_bool(llint != ar.as.llint);
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return arena_bool(false);
}
arena llint_lt(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(llint < ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_bool(llint < ar.as.dval);
    case ARENA_CHAR_CONST:
        return arena_bool(llint < ar.as.ch);
    case ARENA_LLINT_CONST:
        return arena_bool(llint < ar.as.llint);
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return arena_bool(false);
}
arena llint_le(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(llint <= ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_bool(llint <= ar.as.dval);
    case ARENA_CHAR_CONST:
        return arena_bool(llint <= ar.as.ch);
    case ARENA_LLINT_CONST:
        return arena_bool(llint <= ar.as.llint);
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return arena_bool(false);
}
arena llint_gt(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(llint > ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_bool(llint > ar.as.dval);
    case ARENA_CHAR_CONST:
        return arena_bool(llint > ar.as.ch);
    case ARENA_LLINT_CONST:
        return arena_bool(llint > ar.as.llint);
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return arena_bool(false);
}
arena llint_ge(long long int llint, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(llint >= ar.as.ival);
    case ARENA_DOUBLE_CONST:
        return arena_bool(llint >= ar.as.dval);
    case ARENA_CHAR_CONST:
        return arena_bool(llint >= ar.as.ch);
    case ARENA_LLINT_CONST:
        return arena_bool(llint >= ar.as.llint);
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return arena_bool(false);
}

arena double_eq(double dval, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(dval == ar.as.ival);
    case ARENA_CHAR_CONST:
        return arena_bool(dval == ar.as.ch);
    case ARENA_LLINT_CONST:
        return arena_bool(dval == ar.as.llint);
    case ARENA_DOUBLE_CONST:
        return arena_bool(dval == ar.as.dval);
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return arena_bool(false);
}
arena double_ne(double dval, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(dval != ar.as.ival);
    case ARENA_CHAR_CONST:
        return arena_bool(dval != ar.as.ch);
    case ARENA_LLINT_CONST:
        return arena_bool(dval != ar.as.llint);
    case ARENA_DOUBLE_CONST:
        return arena_bool(dval != ar.as.dval);
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return arena_bool(false);
}
arena double_lt(double dval, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(dval < ar.as.ival);
    case ARENA_CHAR_CONST:
        return arena_bool(dval < ar.as.ch);
    case ARENA_LLINT_CONST:
        return arena_bool(dval < ar.as.llint);
    case ARENA_DOUBLE_CONST:
        return arena_bool(dval < ar.as.dval);
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return arena_bool(false);
}
arena double_le(double dval, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(dval <= ar.as.ival);
    case ARENA_CHAR_CONST:
        return arena_bool(dval <= ar.as.ch);
    case ARENA_LLINT_CONST:
        return arena_bool(dval <= ar.as.llint);
    case ARENA_DOUBLE_CONST:
        return arena_bool(dval <= ar.as.dval);
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return arena_bool(false);
}
arena double_gt(double dval, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(dval > ar.as.ival);
    case ARENA_CHAR_CONST:
        return arena_bool(dval > ar.as.ch);
    case ARENA_LLINT_CONST:
        return arena_bool(dval > ar.as.llint);
    case ARENA_DOUBLE_CONST:
        return arena_bool(dval > ar.as.dval);
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return arena_bool(false);
}
arena double_ge(double dval, arena ar)
{
    switch (ar.type)
    {
    case ARENA_INT_CONST:
        return arena_bool(dval >= ar.as.ival);
    case ARENA_CHAR_CONST:
        return arena_bool(dval >= ar.as.ch);
    case ARENA_LLINT_CONST:
        return arena_bool(dval >= ar.as.llint);
    case ARENA_DOUBLE_CONST:
        return arena_bool(dval >= ar.as.dval);
    default:
        log_err("ERROR: comparison type mismatch");
    }
    return arena_bool(false);
}
