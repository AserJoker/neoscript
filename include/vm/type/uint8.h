#ifndef __NEO_VM_UINT8__
#define __NEO_VM_UINT8__
#include "engine/runtime.h"
void neo_init_uint8(neo_runtime runtime);
neo_value create_neo_uint8(neo_context ctx, uint8_t value);
uint8_t neo_value_to_uint8(neo_context ctx, neo_value value);
#endif