#ifndef _ARENA_MATH_H
#define _ARENA_MATH_H
#include "arena_string.h"

arena add_arena_char(char ch, arena ar);
arena sub_arena_char(char ch, arena ar);
arena mul_arena_char(char ch, arena ar);
arena div_arena_char(char ch, arena ar);
arena mod_arena_char(char ch, arena ar);

arena add_arena_int(int ival, arena ar);
arena sub_arena_int(int ival, arena ar);
arena mul_arena_int(int ival, arena ar);
arena div_arena_int(int ival, arena ar);
arena mod_arena_int(int ival, arena ar);

arena add_arena_long(long long int llint, arena ar);
arena sub_arena_long(long long int llint, arena ar);
arena mul_arena_long(long long int llint, arena ar);
arena div_arena_long(long long int llint, arena ar);
arena mod_arena_long(long long int llint, arena ar);

arena add_arena_double(double dval, arena ar);
arena sub_arena_double(double dval, arena ar);
arena mul_arena_double(double dval, arena ar);
arena div_arena_double(double dval, arena ar);

arena char_eq(char ch, arena ar);
arena char_ne(char ch, arena ar);
arena char_lt(char ch, arena ar);
arena char_le(char ch, arena ar);
arena char_gt(char ch, arena ar);
arena char_ge(char ch, arena ar);

arena int_eq(int ival, arena ar);
arena int_ne(int ival, arena ar);
arena int_lt(int ival, arena ar);
arena int_le(int ival, arena ar);
arena int_gt(int ival, arena ar);
arena int_ge(int ival, arena ar);

arena long_eq(long long int llint, arena ar);
arena long_ne(long long int llint, arena ar);
arena long_lt(long long int llint, arena ar);
arena long_le(long long int llint, arena ar);
arena long_gt(long long int llint, arena ar);
arena long_ge(long long int llint, arena ar);

arena double_eq(double dval, arena ar);
arena double_ne(double dval, arena ar);
arena double_lt(double dval, arena ar);
arena double_le(double dval, arena ar);
arena double_gt(double dval, arena ar);
arena double_ge(double dval, arena ar);

#endif
