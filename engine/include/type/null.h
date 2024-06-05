#ifndef __NEO_ENGINE_TYPE_NULL__
#define __NEO_ENGINE_TYPE_NULL__

#include "engine/include/context.h"
#include "engine/include/runtime.h"
#include "engine/include/type.h"

void neo_null_init(neo_runtime rt);
neo_value create_neo_null(neo_context ctx);
#endif