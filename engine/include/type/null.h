#ifndef __NEO_ENGINE_TYPE_NULL__
#define __NEO_ENGINE_TYPE_NULL__

#include "context.h"
#include "runtime.h"
#include "value.h"

void neo_null_init(neo_runtime rt);
neo_value create_neo_null(neo_context ctx);
#endif