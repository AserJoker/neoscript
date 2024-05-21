#ifndef __NEO_VM_INT16__
#define __NEO_VM_INT16__
#include "engine/runtime.h"
void neo_init_int16(neo_runtime runtime);
neo_value create_neo_int16(neo_context ctx, int16_t value);

int16_t neo_value_to_int16(neo_context ctx, neo_value value);
#endif