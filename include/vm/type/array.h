#ifndef __NEO_VM_TYPE_ARRAY__
#define __NEO_VM_TYPE_ARRAY__
#include "engine/runtime.h"
#include "engine/type.h"

void neo_init_array(neo_runtime runtime);
neo_value create_neo_array(neo_context ctx, size_t length);
neo_value *neo_value_to_array(neo_context ctx, neo_value value);
size_t neo_array_get_length(neo_context ctx, neo_value value);
neo_value neo_array_get_index(neo_context ctx, neo_value value, int32_t index);
void neo_array_set_index(neo_context ctx, neo_value value, int32_t index,
                         neo_value item);
#endif