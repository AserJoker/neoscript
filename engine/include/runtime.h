#ifndef __NEO_ENGINE_RUNTIME__
#define __NEO_ENGINE_RUNTIME__

#include "type.h"
#include <stddef.h>
#include <stdio.h>

#define NEO_TYPE_NULL 0
#define NEO_TYPE_EXCEPTION -1
#define NEO_TYPE_PROMISE -2
#define NEO_TYPE_FUNCTION -3

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
#define CHECK_TYPE(TYPE)                                                       \
  do {                                                                         \
    neo_value create_neo_exception(neo_context ctx, const char *message,       \
                                   neo_value caused, const char *filename,     \
                                   int line, int column);                      \
    if (neo_value_get_type_name(value) != TYPE) {                              \
      char buf[1024] = {0};                                                    \
      sprintf(buf, "cannot get %s value from:0x%x", #TYPE,                     \
              neo_value_get_type_name(value));                                 \
      neo_context_throw(                                                       \
          ctx, create_neo_exception(ctx, buf, NULL, __FILE__, __LINE__, 1));   \
    }                                                                          \
  } while (0)
#endif