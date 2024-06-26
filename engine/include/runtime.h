#ifndef __NEO_ENGINE_RUNTIME__
#define __NEO_ENGINE_RUNTIME__

#include "type.h"
#include <stddef.h>
#include <stdio.h>

typedef struct _neo_runtime *neo_runtime;

typedef neo_value (*neo_operator_fn)(neo_context ctx, uint32_t opt, int argc,
                                     neo_value *argv);

neo_runtime create_neo_runtime();
void free_neo_runtime(neo_runtime runtime);

void neo_runtime_define_type(neo_runtime self, neo_type type);
neo_type neo_runtime_get_type(neo_runtime self, uint32_t name);
#define CHECK_TYPE(TYPE)                                                       \
  do {                                                                         \
    neo_value create_neo_exception(neo_context ctx, const cstring message,     \
                                   neo_value caused, const cstring filename,   \
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