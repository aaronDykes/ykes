#ifndef _OBJECT_STRING_H
#define _OBJECT_STRING_H
#include "object.h"

element lltoa(long long int n);
element append(element s, element ar);

element string_eq(element s, element c);
element string_ne(element s, element c);
element string_gt(element s, element c);
element string_ge(element s, element c);
element string_lt(element s, element c);
element string_le(element s, element c);

#endif
