#ifndef __NEO_VM_INT8__
#define __NEO_VM_INT8__
#include "runtime.h"
#include "type.h"
void neo_init_int8(neo_runtime runtime);
neo_value create_neo_int8(neo_context ctx, int8_t value);

int8_t neo_value_to_int8(neo_context ctx, neo_value value);
#endif