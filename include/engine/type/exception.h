#ifndef __NEO_ENGINE_TYPE_EXCEPTION__
#define __NEO_ENGINE_TYPE_EXCEPTION__
#include "engine/context.h"
#include "engine/runtime.h"
#include "engine/value.h"

void neo_exception_init(neo_runtime rt);
neo_value create_neo_exception(neo_context ctx, const char *message);
const char *neo_exception_get_message(neo_context ctx, neo_value exception);
#endif