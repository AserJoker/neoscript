#include "vm/type/function.h"
#include "engine/atom.h"
#include "engine/closure.h"
#include "engine/context.h"
#include "engine/runtime.h"
#include "engine/type.h"
#include "engine/type/exception.h"
#include "engine/value.h"
#include "vm/type.h"
#include <stdio.h>

typedef struct _neo_function_impl *neo_function_impl;
struct _neo_function_impl {
  neo_closure closure;
};
void neo_function_init(void *target, void *source, void *_) {
  neo_function_impl dst = (neo_function_impl)target;
  neo_function_impl src = (neo_function_impl)source;
  dst->closure = src->closure;
}
void neo_function_dispose(void *target, void *_) {
  neo_function_impl dst = (neo_function_impl)target;
  free_neo_closure(dst->closure);
}
void neo_init_function(neo_runtime runtime) {
  neo_type_hook hook = {neo_function_init, 0, neo_function_dispose, 0,
                        neo_atom_copy_ref, 0};
  neo_type neo_function = create_neo_type(
      NEO_VM_TYPE_FUNCTION, sizeof(struct _neo_function_impl), &hook);
  neo_runtime_define_type(runtime, neo_function);
}
neo_value create_neo_function(neo_context ctx, neo_closure closure) {
  neo_type neo_function =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_VM_TYPE_FUNCTION);
  if (!neo_function) {
    neo_context_throw(ctx,
                      create_neo_exception(ctx, "unsupport value type function",
                                           NULL, __FILE__, __LINE__, 1));
  }
  struct _neo_function_impl impl = {closure};
  return neo_context_create_value(ctx, neo_function, &impl);
}
neo_closure neo_value_to_closure(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_VM_TYPE_FUNCTION);
  neo_function_impl impl = (neo_function_impl)neo_value_get_data(value);
  return impl->closure;
}