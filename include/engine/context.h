#ifndef __NEO_ENGINE_CONTEXT__
#define __NEO_ENGINE_CONTEXT__
#include "engine/scope.h"
#include "runtime.h"
typedef struct _neo_context *neo_context;
neo_context create_neo_context(neo_runtime rt);
void free_neo_context(neo_context ctx);

neo_runtime neo_context_get_runtime(neo_context self);

void neo_context_push_scope(neo_context self);
void neo_context_pop_scope(neo_context self);
neo_scope neo_context_get_scope(neo_context self);
#endif