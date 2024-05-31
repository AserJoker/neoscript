#ifndef __NEO_VM_TYPE_CUSTOM__
#define __NEO_VM_TYPE_CUSTOM__
#include "runtime.h"
#include "type.h"
void neo_init_custom(neo_runtime runtime);
neo_value neo_create_custom(neo_context ctx, void *data,
                            void (*auto_free)(void *));
void neo_custom_set_value(neo_context ctx, neo_value value, void *val);
void *neo_value_to_custom(neo_context ctx, neo_value value);
#endif