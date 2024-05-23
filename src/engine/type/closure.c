#include "engine/type/closure.h"
#include "engine/atom.h"
#include "engine/context.h"
#include "engine/runtime.h"
#include "engine/scope.h"
#include "engine/type.h"
#include "engine/type/exception.h"
#include "engine/value.h"
#include "util/list.h"
#include "util/strings.h"
#include <assert.h>
#include <stdlib.h>
typedef struct _neo_closure_impl *neo_closure_impl;
struct _neo_closure_impl {
  neo_function func;
  neo_list values;
  char *name;
  void *arg;
};

void neo_closure_init(void *target, void *source, void *_) {
  neo_closure_impl dst = (neo_closure_impl)target;
  neo_closure_impl src = (neo_closure_impl)source;

  dst->func = src->func;
  dst->values = create_neo_list((neo_free_fn)free_neo_atom);
  dst->name = strings_clone(src->name);
  dst->arg = NULL;
}
void free_neo_closure(void *target, void *_) {
  neo_closure_impl closure = (neo_closure_impl)target;
  if (closure->name) {
    free(closure->name);
  }
  free_neo_list(closure->values);
}

void neo_init_closure(neo_runtime runtime) {
  neo_type_hook hook = {neo_closure_init,  0, free_neo_closure, 0,
                        neo_atom_copy_ref, 0};
  neo_type neo_closure = create_neo_type(
      NEO_TYPE_FUNCTION, sizeof(struct _neo_closure_impl), &hook);
  neo_runtime_define_type(runtime, neo_closure);
}
neo_value create_neo_closure(neo_context ctx, neo_function func,
                             const char *name) {
  neo_type neo_closure =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_TYPE_FUNCTION);
  if (!neo_closure) {
    neo_context_throw_exception(ctx, "unsupport value type promise");
  }
  struct _neo_closure_impl impl = {func, NULL, (char *)name};
  return neo_context_create_value(ctx, neo_closure, &impl);
}
neo_function neo_closure_get_function(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_TYPE_FUNCTION);
  neo_closure_impl impl = (neo_closure_impl)neo_value_get_data(value);
  return impl->func;
}
int32_t neo_closure_add(neo_context ctx, neo_value value, neo_value val) {
  CHECK_TYPE(NEO_TYPE_FUNCTION);
  neo_closure_impl impl = (neo_closure_impl)neo_value_get_data(value);
  neo_atom atom = neo_value_get_atom(val);
  neo_atom root = neo_value_get_atom(value);
  neo_atom_add_ref(atom, root);
  int32_t index = neo_list_length(impl->values);
  neo_list_push(impl->values, atom);
  return index;
}
neo_value neo_closure_get(neo_context ctx, neo_value value, int32_t index) {
  CHECK_TYPE(NEO_TYPE_FUNCTION);
  neo_closure_impl impl = (neo_closure_impl)neo_value_get_data(value);
  neo_list_node node = neo_list_node_next(neo_list_head(impl->values));
  for (int i = 0; i < index; i++) {
    if (node == neo_list_tail(impl->values)) {
      return neo_context_get_null(ctx);
    }
    node = neo_list_node_next(node);
  }
  neo_atom atom = neo_list_node_get(node);
  if (atom) {
    return create_neo_value(neo_context_get_scope(ctx), atom);
  }
  return neo_context_get_null(ctx);
}
const char *neo_closure_get_name(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_TYPE_FUNCTION);
  neo_closure_impl impl = (neo_closure_impl)neo_value_get_data(value);
  return impl->name;
}

void neo_closure_set_arg(neo_context ctx, neo_value value, void *arg) {
  CHECK_TYPE(NEO_TYPE_FUNCTION);
  neo_closure_impl impl = (neo_closure_impl)neo_value_get_data(value);
  impl->arg = arg;
}
void *neo_closure_get_arg(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_TYPE_FUNCTION);
  neo_closure_impl impl = (neo_closure_impl)neo_value_get_data(value);
  return impl->arg;
}

size_t neo_closure_get_length(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_TYPE_FUNCTION);
  neo_closure_impl impl = (neo_closure_impl)neo_value_get_data(value);
  return neo_list_length(impl->values);
}