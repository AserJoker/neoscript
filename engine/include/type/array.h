#ifndef __NEO_ENGINE_TYPE_ARRAY__
#define __NEO_ENGINE_TYPE_ARRAY__
#include "runtime.h"
#include "type.h"
void neo_array_init(neo_runtime runtime);
neo_value create_neo_array(neo_context ctx);
uint32_t neo_array_get_length(neo_context ctx, neo_value array);
neo_value neo_array_get_index(neo_context ctx, neo_value array, uint32_t index);
void neo_array_set_index(neo_context ctx, neo_value array, uint32_t index,
                         neo_value item);
#endif