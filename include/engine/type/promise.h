#ifndef __NEO_ENGINE_TYPE_PROMISE__
#define __NEO_ENGINE_TYPE_PROMISE__
#include "engine/runtime.h"
#include "engine/type.h"
void neo_init_promise(neo_runtime runtime);

typedef enum PromiseStatus {
  PROMISE_FULFILLED,
  PROMISE_PENDDING,
  PROMISE_REJECTED
} PromiseStatus;

neo_value create_neo_promise(neo_context ctx);
neo_value neo_promise_get_value(neo_value self, neo_context ctx);
PromiseStatus neo_promise_get_status(neo_value self, neo_context ctx);
int8_t neo_promise_ready(neo_value self, neo_context ctx, neo_value data);
int8_t neo_promise_error(neo_value self, neo_context ctx, neo_value error);
#endif