#ifndef _OBJECT_MATH_H
#define _OBJECT_MATH_H
#include "object.h"

element _neg(element n);
element _add(element a, element b);
element _sub(element a, element b);
element _mul(element a, element b);
element _div(element a, element b);
element _mod(element a, element b);
element _eq(element a, element b);
element _ne(element a, element b);
element _lt(element a, element b);
element _le(element a, element b);
element _gt(element a, element b);
element _ge(element a, element b);
element _or(element a, element b);
element _and(element a, element b);
element _inc(element b);
element _dec(element b);
element _sqr(element a);

#endif
