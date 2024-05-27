#ifndef __NEO_VM_UINT16__
#define __NEO_VM_UINT16__
#include "runtime.h"
void neo_init_uint16(neo_runtime runtime);
neo_value create_neo_uint16(neo_context ctx, uint16_t value);
uint16_t neo_value_to_uint16(neo_context ctx, neo_value value);
#endif