#include "type/promise.h"
#include "atom.h"
#include "common/include/cstring.h"
#include "context.h"
#include "runtime.h"
#include "scope.h"
#include "type.h"
#include "typedef.h"
#include "value.h"
#include <stdio.h>

typedef struct _neo_promise_impl {
  neo_atom value;
  PromiseStatus status;
} *neo_promise_impl;

static void neo_init_promise(void *target, void *_, void *__) {
  neo_promise_impl dst = (neo_promise_impl)target;
  dst->status = PROMISE_PENDDING;
  dst->value = NULL;
}
static int8_t neo_convert_promise(void *data, uint32_t type, void *output,
                                  void *_) {
  switch (type) {
  case NEO_TYPE_BOOLEAN:
    *(int8_t *)output = 1;
    return 1;
  case NEO_TYPE_STRING: {
    neo_promise_impl impl = (neo_promise_impl)data;
    switch (impl->status) {
    case PROMISE_FULFILLED:
      *(cstring *)output = cstring_clone("[Promise fulfilled]");
      return 1;
    case PROMISE_PENDDING:
      *(cstring *)output = cstring_clone("[Promise pendding]");
      return 1;
    case PROMISE_REJECTED:
      *(cstring *)output = cstring_clone("[Promise rejected]");
      return 1;
    }
  }
  }
  return 0;
}
void neo_promise_init(neo_runtime runtime) {
  neo_type_hook hook = {neo_init_promise,    0, 0, 0, neo_atom_copy_ref, 0,
                        neo_convert_promise, 0};
  neo_type neo_promise =
      create_neo_type(NEO_TYPE_PROMISE, sizeof(struct _neo_type_hook), &hook);
  neo_runtime_define_type(runtime, neo_promise);
}
neo_value create_neo_promise(neo_context ctx) {
  neo_type neo_promise =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_TYPE_PROMISE);
  return neo_context_create_value(ctx, neo_promise, NULL);
}
neo_value neo_promise_get_value(neo_value value, neo_context ctx) {
  CHECK_TYPE(NEO_TYPE_PROMISE);
  neo_promise_impl impl = (neo_promise_impl)neo_value_get_data(value);
  neo_scope scope = neo_context_get_scope(ctx);
  if (!impl->value) {
    return neo_context_get_null(ctx);
  }
  return create_neo_value(scope, impl->value);
}
PromiseStatus neo_promise_get_status(neo_value value, neo_context ctx) {
  CHECK_TYPE(NEO_TYPE_PROMISE);
  neo_promise_impl impl = (neo_promise_impl)neo_value_get_data(value);
  return impl->status;
}
int8_t neo_promise_ready(neo_value value, neo_context ctx, neo_value data) {
  CHECK_TYPE(NEO_TYPE_PROMISE);
  neo_promise_impl impl = (neo_promise_impl)neo_value_get_data(value);
  if (impl->status == PROMISE_PENDDING) {
    impl->status = PROMISE_FULFILLED;
    neo_atom atom = neo_value_get_atom(value);
    neo_atom data_atom = neo_value_get_atom(data);
    impl->value = data_atom;
    neo_atom_add_ref(data_atom, atom);
    return 1;
  }
  return 0;
}
int8_t neo_promise_error(neo_value value, neo_context ctx, neo_value data) {
  CHECK_TYPE(NEO_TYPE_PROMISE);
  neo_promise_impl impl = (neo_promise_impl)neo_value_get_data(value);
  if (impl->status == PROMISE_PENDDING) {
    impl->status = PROMISE_REJECTED;
    neo_atom atom = neo_value_get_atom(value);
    neo_atom data_atom = neo_value_get_atom(data);
    impl->value = data_atom;
    neo_atom_add_ref(data_atom, atom);
    return 1;
  }
  return 0;
}