#include "type/uint64.h"
#include "atom.h"
#include "context.h"
#include "runtime.h"
#include "type/exception.h"
#include "typedef.h"
#include "value.h"
#include <stdint.h>
#include <stdio.h>

void neo_uint64_init(void *target, void *source, void *_) {
  uint64_t *dst = (uint64_t *)target;
  uint64_t *src = (uint64_t *)source;
  *dst = *src;
}
void neo_init_uint64(neo_runtime runtime) {
  neo_type_hook hook = {neo_uint64_init,   NULL, NULL, NULL,
                        neo_atom_copy_val, NULL};
  neo_type neo_uint64 =
      create_neo_type(NEO_VM_TYPE_UINT64, sizeof(uint64_t), &hook);
  neo_runtime_define_type(runtime, neo_uint64);
}
neo_value create_neo_uint64(neo_context ctx, uint64_t value) {
  neo_type type =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_VM_TYPE_UINT64);
  if (!type) {
    neo_context_throw(ctx,
                      create_neo_exception(ctx, "unsupport value type uint64",
                                           NULL, __FILE__, __LINE__, 1));
  }
  return neo_context_create_value(ctx, type, &value);
}

uint64_t neo_value_to_uint64(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_VM_TYPE_UINT64);
  uint64_t *data = (uint64_t *)neo_value_get_data(value);
  return *data;
}