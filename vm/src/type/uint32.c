#include "type/uint32.h"
#include "atom.h"
#include "context.h"
#include "runtime.h"
#include "type/exception.h"
#include "typedef.h"
#include "value.h"
#include <stdint.h>
#include <stdio.h>

void neo_uint32_init(void *target, void *source, void *_) {
  uint32_t *dst = (uint32_t *)target;
  uint32_t *src = (uint32_t *)source;
  *dst = *src;
}

void neo_init_uint32(neo_runtime runtime) {
  neo_type_hook hook = {neo_uint32_init,   NULL, NULL, NULL,
                        neo_atom_copy_val, NULL};
  neo_type neo_uint32 =
      create_neo_type(NEO_VM_TYPE_UINT32, sizeof(uint32_t), &hook);
  neo_runtime_define_type(runtime, neo_uint32);
}
neo_value create_neo_uint32(neo_context ctx, uint32_t value) {
  neo_type type =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_VM_TYPE_UINT32);
  if (!type) {
    neo_context_throw(ctx,
                      create_neo_exception(ctx, "unsupport value type uint32",
                                           NULL, __FILE__, __LINE__, 1));
  }
  return neo_context_create_value(ctx, type, &value);
}

uint32_t neo_value_to_uint32(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_VM_TYPE_UINT32);
  uint32_t *data = (uint32_t *)neo_value_get_data(value);
  return *data;
}