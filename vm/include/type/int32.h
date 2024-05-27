#ifndef __NEO_VM_INT32__
#define __NEO_VM_INT32__
#include "runtime.h"
void neo_init_int32(neo_runtime runtime);
neo_value create_neo_int32(neo_context ctx, int32_t value);
int32_t neo_value_to_int32(neo_context ctx, neo_value value);
#endif