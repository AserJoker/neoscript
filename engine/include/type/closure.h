#ifndef __NEO_ENGINE_CLOSURE__
#define __NEO_ENGINE_CLOSURE__
#include "common/include/list.h"
#include "engine/include/runtime.h"
#include "engine/include/type.h"
#include "common/include/cstring.h"

void neo_closure_init(neo_runtime runtime);

neo_value create_neo_closure(neo_context ctx, neo_function func,
                             const cstring name);

void neo_closure_set_arg(neo_context ctx, neo_value self, void *arg);
void *neo_closure_get_arg(neo_context ctx, neo_value self);

neo_function neo_closure_get_function(neo_context ctx, neo_value self);

void neo_closure_add(neo_context ctx, neo_value self, const cstring name,
                     neo_value val);
neo_value neo_closure_get(neo_context ctx, neo_value self, const cstring name);
neo_list neo_closure_get_keys(neo_context ctx, neo_value self);

const cstring neo_closure_get_name(neo_context ctx, neo_value self);

#endif