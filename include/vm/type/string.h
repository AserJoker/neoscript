#ifndef __NEO_VM_TYPE_STRING__
#define __NEO_VM_TYPE_STRING__
#include "engine/runtime.h"
void neo_init_string(neo_runtime runtime);
neo_value create_neo_string(neo_context ctx, const char *value);
const char *neo_value_to_string(neo_context ctx, neo_value value);
#endif