#include "vm/type/int32.h"
#include "engine/atom.h"
#include "engine/context.h"
#include "engine/runtime.h"
#include "engine/type.h"
#include "engine/type/exception.h"
#include "engine/value.h"
#include "vm/type.h"
#include <stdint.h>
#include <stdio.h>

void neo_int32_init(void *target, void *source, void *_) {
  int32_t *dst = (int32_t *)target;
  int32_t *src = (int32_t *)source;
  *dst = *src;
}

void neo_init_int32(neo_runtime runtime) {
  neo_type_hook hook = {neo_int32_init,    NULL, NULL, NULL,
                        neo_atom_copy_val, NULL};
  neo_type neo_int32 =
      create_neo_type(NEO_VM_TYPE_INT32, sizeof(int32_t), &hook);
  neo_runtime_define_type(runtime, neo_int32);
}

neo_value create_neo_int32(neo_context ctx, int32_t value) {
  neo_type type =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_VM_TYPE_INT32);
  if (!type) {
    neo_context_throw(ctx,
                      create_neo_exception(ctx, "unsupport value type int32",
                                           NULL, __FILE__, __LINE__, 1));
  }
  return neo_context_create_value(ctx, type, &value);
}

int32_t neo_value_to_int32(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_VM_TYPE_INT32);
  int32_t *data = (int32_t *)neo_value_get_data(value);
  return *data;
}