#ifndef __NEO_ENGINE_RUNTIME__
#define __NEO_ENGINE_RUNTIME__

#include "engine/type.h"
#include <stddef.h>

#define NEO_TYPE_NULL 0
#define NEO_TYPE_EXCEPTION -1

typedef struct _neo_runtime *neo_runtime;

typedef neo_value (*neo_operator_fn)(neo_context ctx, uint32_t opt, int argc,
                                     neo_value *argv);

neo_runtime create_neo_runtime();
void free_neo_runtime(neo_runtime runtime);

void neo_runtime_define_type(neo_runtime self, neo_type type);
neo_type neo_runtime_get_type(neo_runtime self, uint32_t name);

void neo_runtime_define_operator(neo_runtime self, uint32_t opt,
                                 neo_operator_fn fn);
neo_operator_fn neo_runtime_get_operator(neo_runtime self, uint32_t opt);

#endif