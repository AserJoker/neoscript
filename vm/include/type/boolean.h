#ifndef __NEO_VM_TYPE_BOOLEAN__
#define __NEO_VM_TYPE_BOOLEAN__

#include "runtime.h"
#include "type.h"
void neo_init_boolean(neo_runtime runtime);
neo_value create_neo_boolean(neo_context ctx, int8_t value);

int8_t neo_value_to_boolean(neo_context ctx, neo_value value);

#define TRUE 1
#define FALSE 0

#endif