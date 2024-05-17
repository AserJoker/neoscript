#ifndef __NEO_ENGINE_RUNTIME__
#define __NEO_ENGINE_RUNTIME__

#include "engine/type.h"
#include <stddef.h>

#define NEO_TYPE_EXCEPTION -1

typedef struct _neo_runtime *neo_runtime;

neo_runtime create_neo_runtime();
void free_neo_runtime(neo_runtime runtime);

void neo_runtime_define_type(neo_runtime self, neo_type type);
neo_type neo_runtime_get_type(neo_runtime self, uint32_t name);

#endif