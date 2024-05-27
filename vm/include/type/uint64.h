#ifndef __NEO_VM_UINT64__
#define __NEO_VM_UINT64__
#include "runtime.h"
void neo_init_uint64(neo_runtime runtime);
neo_value create_neo_uint64(neo_context ctx, uint64_t value);
uint64_t neo_value_to_uint64(neo_context ctx, neo_value value);
#endif