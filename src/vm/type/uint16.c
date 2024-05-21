#include "vm/type/uint16.h"
#include "engine/atom.h"
#include "engine/context.h"
#include "engine/runtime.h"
#include "engine/type.h"
#include "engine/type/exception.h"
#include "engine/value.h"
#include "vm/type.h"
#include <stdint.h>
#include <stdio.h>

void neo_uint16_init(void *target, void *source, void *_) {
  uint16_t *dst = (uint16_t *)target;
  uint16_t *src = (uint16_t *)source;
  *dst = *src;
}

void neo_init_uint16(neo_runtime runtime) {
  neo_type_hook hook = {neo_uint16_init,   NULL, NULL, NULL,
                        neo_atom_copy_val, NULL};
  neo_type neo_uint16 =
      create_neo_type(NEO_VM_TYPE_UINT16, sizeof(uint16_t), &hook);
  neo_runtime_define_type(runtime, neo_uint16);
}
neo_value create_neo_uint16(neo_context ctx, uint16_t value) {
  neo_type type =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_VM_TYPE_UINT16);
  if (!type) {
    neo_context_throw(ctx,
                      create_neo_exception(ctx, "unsupport value type uint16",
                                           NULL, __FILE__, __LINE__, 1));
  }
  return neo_context_create_value(ctx, type, &value);
}

uint16_t neo_value_to_uint16(neo_context ctx, neo_value value) {
  if (neo_value_get_type_name(value) != NEO_VM_TYPE_UINT16) {
    char buf[1024] = {0};
    sprintf(buf, "cannot get uint16 value from:0x%x",
            neo_value_get_type_name(value));
    neo_context_throw(
        ctx, create_neo_exception(ctx, buf, NULL, __FILE__, __LINE__, 1));
  }
  uint16_t *data = (uint16_t *)neo_value_get_data(value);
  return *data;
}