#ifndef __NEO_ENGINE_TYPE_EXCEPTION__
#define __NEO_ENGINE_TYPE_EXCEPTION__
#include "engine/runtime.h"
#include "engine/type.h"
#include "util/list.h"
void neo_init_exception(neo_runtime runtime);

neo_value create_neo_exception(neo_context ctx, const char *message,
                               neo_value caused, const char *filename, int line,
                               int column);

const char *neo_exception_get_message(neo_value self);
neo_value neo_exception_get_caused(neo_value self, neo_context ctx);
neo_list neo_exception_get_stack(neo_value self);
#define neo_context_throw_exception(ctx, message)                              \
  neo_context_throw(                                                           \
      ctx, create_neo_exception(ctx, message, NULL, __FILE__, __LINE__, 1))
#endif