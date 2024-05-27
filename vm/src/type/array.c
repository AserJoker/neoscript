#include "type/array.h"
#include "atom.h"
#include "context.h"
#include "runtime.h"
#include "type/exception.h"
#include "typedef.h"
#include "value.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct _neo_array_impl {
  neo_atom *buffer;
  size_t length;
} *neo_array_impl;
void neo_array_init(void *target, void *source, void *_) {
  neo_array_impl dst = (neo_array_impl)target;
  neo_array_impl src = (neo_array_impl)source;
  dst->buffer = src->buffer;
  dst->length = src->length;
}
void neo_array_dispose(void *target, void *_) {
  neo_array_impl dst = (neo_array_impl)target;
  free(dst->buffer);
}
void neo_init_array(neo_runtime runtime) {
  neo_type_hook hook = {neo_array_init,    NULL, neo_array_dispose, NULL,
                        neo_atom_copy_ref, NULL};
  neo_type neo_array =
      create_neo_type(NEO_VM_TYPE_ARRAY, sizeof(struct _neo_array_impl), &hook);
  neo_runtime_define_type(runtime, neo_array);
}
neo_value create_neo_array(neo_context ctx, size_t length) {
  neo_type neo_array =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_VM_TYPE_ARRAY);
  if (!neo_array) {
    neo_context_throw_exception(ctx, "unsupport value type array");
  }
  neo_atom *atoms = (neo_atom *)malloc(sizeof(neo_atom) * length);
  memset(atoms, 0, sizeof(neo_atom) * length);
  struct _neo_array_impl impl = {atoms, length};
  return neo_context_create_value(ctx, neo_array, &impl);
}
neo_value *neo_value_to_array(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_VM_TYPE_ARRAY);
  neo_array_impl impl = (neo_array_impl)neo_value_get_data(value);
  neo_value *result = (neo_value *)malloc(sizeof(neo_value) * impl->length);
  for (size_t i = 0; i < impl->length; i++) {
    if (impl->buffer[i]) {
      result[i] = create_neo_value(neo_context_get_scope(ctx), impl->buffer[i]);
    } else {
      result[i] = neo_context_get_null(ctx);
    }
  }
  return result;
}
size_t neo_array_get_length(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_VM_TYPE_ARRAY);
  neo_array_impl impl = (neo_array_impl)neo_value_get_data(value);
  return impl->length;
}
neo_value neo_array_get_index(neo_context ctx, neo_value value, int32_t index) {
  CHECK_TYPE(NEO_VM_TYPE_ARRAY);
  neo_array_impl impl = (neo_array_impl)neo_value_get_data(value);
  if (index >= impl->length) {
    return neo_context_get_null(ctx);
  }
  return create_neo_value(neo_context_get_scope(ctx), impl->buffer[index]);
}
void neo_array_set_index(neo_context ctx, neo_value value, int32_t index,
                         neo_value item) {
  CHECK_TYPE(NEO_VM_TYPE_ARRAY);
  neo_array_impl impl = (neo_array_impl)neo_value_get_data(value);
  neo_atom atom = neo_value_get_atom(value);
  neo_atom item_atom = neo_value_get_atom(item);
  if (index < impl->length) {
    if (item_atom != impl->buffer[index]) {
      neo_atom_add_ref(item_atom, atom);
      neo_atom_remove_ref(impl->buffer[index], atom);
      impl->buffer[index] = atom;
    }
  } else {
    neo_atom *buffer = (neo_atom *)malloc(sizeof(neo_atom) * (index + 1));
    memset(buffer, 0, sizeof(neo_atom) * (index + 1));
    for (int i = 0; i < impl->length; i++) {
      buffer[i] = impl->buffer[i];
    }
    neo_atom_add_ref(item_atom, atom);
    buffer[index] = item_atom;
    free(impl->buffer);
    impl->buffer = buffer;
    impl->length = index + 1;
  }
}