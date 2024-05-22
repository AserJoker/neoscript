#ifndef __NEO_VM_TYPE_FUNCTION__
#define __NEO_VM_TYPE_FUNCTION__
#include "engine/closure.h"
#include "engine/runtime.h"
#include "engine/type.h"
void neo_init_function(neo_runtime runtime);
neo_value create_neo_function(neo_context ctx, neo_closure closure);
neo_closure neo_value_to_closure(neo_context ctx, neo_value value);
#define neo_context_create_function(ctx, func)                                 \
  create_neo_function(ctx, create_neo_closure(ctx, func, #func))
#endif