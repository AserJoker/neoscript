#ifndef __NEO_ENGINE_TYPE_NUMBER__
#define __NEO_ENGINE_TYPE_NUMBER__
#include "engine/include/runtime.h"
#include "engine/include/type.h"
void neo_number_init(neo_runtime runtime);
double neo_value_get_number(neo_context ctx, neo_value value);
void neo_value_set_number(neo_context ctx, neo_value value, double val);
neo_value create_neo_number(neo_context ctx, double val);
#endif