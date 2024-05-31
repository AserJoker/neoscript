#ifndef __NEO_VM_TYPE_INTEGER__
#define __NEO_VM_TYPE_INTEGER__
#include "runtime.h"
#include "type.h"
#include <stdint.h>
void neo_init_integer(neo_runtime runtime);
neo_value create_neo_integer(neo_context ctx, int64_t value);
int64_t neo_value_to_integer(neo_context ctx, neo_value value);
void neo_integer_set_value(neo_context ctx, neo_value value, int64_t val);
#endif