#ifndef __NEO_ENGINE_TYPE_STRING__
#define __NEO_ENGINE_TYPE_STRING__
#include "engine/include/runtime.h"
#include "engine/include/type.h"
#include "common/include/cstring.h"
void neo_string_init(neo_runtime runtime);
const cstring neo_value_get_string(neo_context ctx, neo_value value);
void neo_value_set_string(neo_context ctx, neo_value value, const cstring val);
neo_value create_neo_string(neo_context ctx, const cstring val);
#endif