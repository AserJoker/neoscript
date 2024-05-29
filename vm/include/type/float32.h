#ifndef __NEO_VM_FLOAT32__
#define __NEO_VM_FLOAT32__
#include "runtime.h"
void neo_init_float32(neo_runtime runtime);
neo_value create_neo_float32(neo_context ctx, float value);
float neo_value_to_float32(neo_context ctx, neo_value value);
#endif