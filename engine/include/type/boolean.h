#ifndef __NEO_ENGINE_TYPE_BOOLEAN__
#define __NEO_ENGINE_TYPE_BOOLEAN__
#include "runtime.h"
#include "type.h"
#define NEO_TRUE 1
#define NEO_FALSE 0
void neo_boolean_init(neo_runtime rt);
int8_t neo_value_get_boolean(neo_context ctx, neo_value value);
void neo_value_set_boolean(neo_context ctx, neo_value value, int8_t val);
neo_value create_neo_boolean(neo_context ctx, int8_t val);
#endif