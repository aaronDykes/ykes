#ifndef _ARENA_MATH_UTIL_H
#define _ARENA_MATH_UTIL_H
#include "arena_memory.h"

static arena add_arena_char(char ch, arena ar);
static arena sub_arena_char(char ch, arena ar);
static arena mul_arena_char(char ch, arena ar);
static arena div_arena_char(char ch, arena ar);
static arena mod_arena_char(char ch, arena ar);

static arena add_arena_size(size_t size, arena ar);
static arena sub_arena_size(size_t size, arena ar);
static arena mul_arena_size(size_t size, arena ar);
static arena div_arena_size(size_t size, arena ar);
static arena mod_arena_size(size_t size, arena ar);

static arena add_arena_int(int ival, arena ar);
static arena sub_arena_int(int ival, arena ar);
static arena mul_arena_int(int ival, arena ar);
static arena div_arena_int(int ival, arena ar);
static arena mod_arena_int(int ival, arena ar);

static arena add_arena_long(long long int llint, arena ar);
static arena sub_arena_long(long long int llint, arena ar);
static arena mul_arena_long(long long int llint, arena ar);
static arena div_arena_long(long long int llint, arena ar);
static arena mod_arena_long(long long int llint, arena ar);

static arena add_arena_double(double dval, arena ar);
static arena sub_arena_double(double dval, arena ar);
static arena mul_arena_double(double dval, arena ar);
static arena div_arena_double(double dval, arena ar);

static arena char_eq(char ch, arena ar);
static arena char_ne(char ch, arena ar);
static arena char_lt(char ch, arena ar);
static arena char_le(char ch, arena ar);
static arena char_gt(char ch, arena ar);
static arena char_ge(char ch, arena ar);

static arena int_eq(int ival, arena ar);
static arena int_ne(int ival, arena ar);
static arena int_lt(int ival, arena ar);
static arena int_le(int ival, arena ar);
static arena int_gt(int ival, arena ar);
static arena int_ge(int ival, arena ar);

static arena long_eq(long long int llint, arena ar);
static arena long_ne(long long int llint, arena ar);
static arena long_lt(long long int llint, arena ar);
static arena long_le(long long int llint, arena ar);
static arena long_gt(long long int llint, arena ar);
static arena long_ge(long long int llint, arena ar);

static arena double_eq(double dval, arena ar);
static arena double_ne(double dval, arena ar);
static arena double_lt(double dval, arena ar);
static arena double_le(double dval, arena ar);
static arena double_gt(double dval, arena ar);
static arena double_ge(double dval, arena ar);

static arena size_eq(size_t size, arena ar);
static arena size_ne(size_t size, arena ar);
static arena size_lt(size_t size, arena ar);
static arena size_le(size_t size, arena ar);
static arena size_gt(size_t size, arena ar);
static arena size_ge(size_t size, arena ar);

static arena _access_ints(int *Ints, arena a, int len);
static arena _access_longs(long long int *Longs, arena a, int len);
static arena _access_doubles(double *Doubles, arena a, int len);
static arena _access_string(char *String, arena a, int len);
static arena _access_strings(char **Strings, arena a, int len);

#endif
