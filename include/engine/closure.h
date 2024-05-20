#ifndef __NEO_ENGINE_CLOSURE__
#define __NEO_ENGINE_CLOSURE__
#include "engine/type.h"
typedef struct _neo_closure *neo_closure;

neo_closure create_neo_closure(neo_context ctx, neo_function func,
                               const char *name);
void free_neo_closure(neo_closure closure);

neo_value neo_closure_get(neo_context ctx, neo_closure self, int32_t index);

neo_function neo_closure_get_function(neo_closure self);
int32_t neo_closure_add(neo_closure self, neo_value val);
const char *neo_closure_get_name(neo_closure self);
#endif