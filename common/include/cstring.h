#ifndef __NEO_COMMON_CSTRING__
#define __NEO_COMMON_CSTRING__
#include "common/include/list.h"
#include <stdint.h>
#define cstring char *
cstring cstring_clone(const cstring str);
cstring cstring_from_int(int val);
int8_t cstring_compare(const cstring s1, const cstring s2);
cstring cstring_join(neo_list list, const cstring spliter);
#endif