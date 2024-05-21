#ifndef __NEO_VM_INT64__
#define __NEO_VM_INT64__
#include "engine/runtime.h"
void neo_init_int64(neo_runtime runtime);
neo_value create_neo_int64(neo_context ctx, int64_t value);
int64_t neo_value_to_int64(neo_context ctx, neo_value value);
#endif