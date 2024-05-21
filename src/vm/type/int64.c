#include "vm/type/int64.h"
#include "engine/atom.h"
#include "engine/context.h"
#include "engine/runtime.h"
#include "engine/type.h"
#include "engine/type/exception.h"
#include "engine/value.h"
#include "vm/type.h"
#include <stdint.h>

void neo_int64_init(void *target, void *source, void *_) {
  int64_t *dst = (int64_t *)target;
  int64_t *src = (int64_t *)source;
  *dst = *src;
}

void neo_init_int64(neo_runtime runtime) {
  neo_type_hook hook = {neo_int64_init,    NULL, NULL, NULL,
                        neo_atom_copy_val, NULL};
  neo_type neo_int64 =
      create_neo_type(NEO_VM_TYPE_INT64, sizeof(int64_t), &hook);
  neo_runtime_define_type(runtime, neo_int64);
}
neo_value create_neo_int64(neo_context ctx, int64_t value) {
  neo_type type =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_VM_TYPE_INT64);
  if (!type) {
    neo_context_throw(ctx,
                      create_neo_exception(ctx, "unsupport value type int64",
                                           NULL, __FILE__, __LINE__, 1));
  }
  return neo_context_create_value(ctx, type, &value);
}

int64_t neo_value_to_int64(neo_context ctx, neo_value value) {
  if (neo_value_get_type_name(value) != NEO_VM_TYPE_INT64) {
    neo_context_throw(ctx,
                      create_neo_exception(ctx, "unsupport value type boolean",
                                           NULL, __FILE__, __LINE__, 1));
  }
  int64_t *data = (int64_t *)neo_value_get_data(value);
  return *data;
}