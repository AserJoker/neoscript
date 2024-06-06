#ifndef __NEO_COMMON_STRINGS__
#define __NEO_COMMON_STRINGS__
#include "common/include/list.h"
#include <stdint.h>
char *strings_clone(const char *str);
char *strings_from_int(int val);
int8_t strings_compare(const char *s1, const char *s2);
char *strings_join(neo_list list, const char *spliter);
#endif