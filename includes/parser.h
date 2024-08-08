#ifndef _PARSER_H
#define _PARSER_H
#include "parser_common.h"

parser parse(const char *src, const char *file);
uint8_t is_unary(token_t type);
#endif
