#ifndef __NEO_VM_UINT32__
#define __NEO_VM_UINT32__
#include "runtime.h"
void neo_init_uint32(neo_runtime runtime);
neo_value create_neo_uint32(neo_context ctx, uint32_t value);
uint32_t neo_value_to_uint32(neo_context ctx, neo_value value);
#endif