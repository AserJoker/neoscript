#include "type/closure.h"
#include "atom.h"
#include "common/include/map.h"
#include "common/include/strings.h"
#include "context.h"
#include "runtime.h"
#include "scope.h"
#include "type.h"
#include "typedef.h"
#include "value.h"
#include <assert.h>
#include <stdlib.h>
typedef struct _neo_closure_impl *neo_closure_impl;
struct _neo_closure_impl {
  neo_function func;
  neo_map values;
  char *name;
  void *arg;
};

static void neo_init_closure(void *target, void *source, void *_) {
  neo_closure_impl dst = (neo_closure_impl)target;
  neo_closure_impl src = (neo_closure_impl)source;

  dst->func = src->func;
  dst->values = create_neo_map((neo_compare_fn)strings_compare, free, NULL);
  dst->name = strings_clone(src->name);
  dst->arg = NULL;
}
static void neo_dispose_closure(void *target, void *_) {
  neo_closure_impl closure = (neo_closure_impl)target;
  if (closure->name) {
    free(closure->name);
  }
  free_neo_map(closure->values);
}
static int8_t neo_convert_closure(void *data, uint32_t type, void *output,
                                  void *_) {
  switch (type) {
  case NEO_TYPE_BOOLEAN:
    *(int8_t *)output = 1;
    return 1;
  case NEO_TYPE_STRING: {
    char buf[128];
    neo_closure_impl closure = (neo_closure_impl)data;
    sprintf(buf, "[Function %s]", closure->name ? closure->name : "<no name>");
    *(char **)output = strings_clone(buf);
    return 1;
  }
  }
  return 0;
}
void neo_closure_init(neo_runtime runtime) {
  neo_type_hook hook = {neo_init_closure,  0, neo_dispose_closure, 0,
                        neo_atom_copy_ref, 0, neo_convert_closure, 0};
  neo_type neo_closure = create_neo_type(
      NEO_TYPE_FUNCTION, sizeof(struct _neo_closure_impl), &hook);
  neo_runtime_define_type(runtime, neo_closure);
}
neo_value create_neo_closure(neo_context ctx, neo_function func,
                             const char *name) {
  neo_type neo_closure =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_TYPE_FUNCTION);
  struct _neo_closure_impl impl = {func, NULL, (char *)name};
  return neo_context_create_value(ctx, neo_closure, &impl);
}
neo_function neo_closure_get_function(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_TYPE_FUNCTION);
  neo_closure_impl impl = (neo_closure_impl)neo_value_get_data(value);
  return impl->func;
}

void neo_closure_add(neo_context ctx, neo_value value, const char *name,
                     neo_value val) {
  CHECK_TYPE(NEO_TYPE_FUNCTION);
  neo_closure_impl impl = (neo_closure_impl)neo_value_get_data(value);
  neo_atom atom = neo_value_get_atom(val);
  neo_atom old = neo_map_get(impl->values, (void *)name);
  if (old == atom) {
    return;
  }
  neo_atom root = neo_value_get_atom(value);
  if (old) {
    neo_atom_remove_ref(old, root);
  }
  neo_atom_add_ref(atom, root);
  neo_map_set(impl->values, strings_clone(name), atom);
}
neo_value neo_closure_get(neo_context ctx, neo_value value, const char *name) {
  CHECK_TYPE(NEO_TYPE_FUNCTION);
  neo_closure_impl impl = (neo_closure_impl)neo_value_get_data(value);
  neo_atom atom = neo_map_get(impl->values, (void *)name);
  if (atom) {
    return create_neo_value(neo_context_get_scope(ctx), atom);
  }
  return neo_context_get_null(ctx);
}
neo_list neo_closure_get_keys(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_TYPE_FUNCTION);
  neo_closure_impl impl = (neo_closure_impl)neo_value_get_data(value);
  return neo_map_keys(impl->values);
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