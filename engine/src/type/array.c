#include "type/array.h"
#include "atom.h"
#include "context.h"
#include "runtime.h"
#include "type.h"
#include "typedef.h"
#include "value.h"
#include <stdlib.h>
typedef struct _neo_array_impl {
  neo_atom *items;
  uint32_t size;
} *neo_array_impl;
static void neo_init_array(void *target, void *_, void *__) {
  neo_array_impl dst = (neo_array_impl)target;
  dst->size = 0;
  dst->items = NULL;
}
static void neo_dispose_array(void *target, void *_) {
  neo_array_impl dst = (neo_array_impl)target;
  free(dst->items);
}
void neo_array_init(neo_runtime runtime) {
  neo_type_hook hook = {neo_init_array,    0, neo_dispose_array, 0,
                        neo_atom_copy_ref, 0};
  neo_type neo_array =
      create_neo_type(NEO_TYPE_ARRAY, sizeof(struct _neo_array_impl), &hook);
  neo_runtime_define_type(runtime, neo_array);
}
neo_value create_neo_array(neo_context ctx) {
  neo_type neo_array =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_TYPE_ARRAY);
  return neo_context_create_value(ctx, neo_array, NULL);
}
uint32_t neo_array_get_length(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_TYPE_ARRAY);
  neo_array_impl impl = (neo_array_impl)neo_value_get_data(value);
  return impl->size;
}
neo_value neo_array_get_index(neo_context ctx, neo_value value,
                              uint32_t index) {
  CHECK_TYPE(NEO_TYPE_ARRAY);
  neo_array_impl impl = (neo_array_impl)neo_value_get_data(value);
  if (index <= impl->size) {
    neo_atom result = impl->items[index];
    if (result) {
      return create_neo_value(neo_context_get_scope(ctx), result);
    }
  }
  return neo_context_get_null(ctx);
}
void neo_array_set_index(neo_context ctx, neo_value value, uint32_t index,
                         neo_value item) {
  CHECK_TYPE(NEO_TYPE_ARRAY);
  neo_array_impl impl = (neo_array_impl)neo_value_get_data(value);
  if (index >= impl->size) {
    neo_atom *items = malloc(sizeof(neo_atom) * (index + 1));
    for (uint32_t i = 0; i < impl->size; i++) {
      items[i] = impl->items[i];
    }
    items[index] = NULL;
    free(impl->items);
    impl->items = items;
    impl->size = index + 1;
  }
  neo_atom item_atom = neo_value_get_atom(item);
  if (item_atom != impl->items[index]) {
    neo_atom array_atom = neo_value_get_atom(value);
    neo_atom_add_ref(item_atom, array_atom);
    if (impl->items[index] != NULL) {
      neo_atom_remove_ref(impl->items[index], array_atom);
      impl->items[index] = item_atom;
    }
  }
}