#include "vm/type/int16.h"
#include "engine/atom.h"
#include "engine/context.h"
#include "engine/runtime.h"
#include "engine/type.h"
#include "engine/type/exception.h"
#include "engine/value.h"
#include "vm/type.h"
#include <stdint.h>

void neo_int16_init(void *target, void *source, void *_) {
  int16_t *dst = (int16_t *)target;
  int16_t *src = (int16_t *)source;
  *dst = *src;
}

void neo_init_int16(neo_runtime runtime) {
  neo_type_hook hook = {neo_int16_init,    NULL, NULL, NULL,
                        neo_atom_copy_val, NULL};
  neo_type neo_int16 =
      create_neo_type(NEO_VM_TYPE_INT16, sizeof(int16_t), &hook);
  neo_runtime_define_type(runtime, neo_int16);
}

neo_value create_neo_int16(neo_context ctx, int16_t value) {
  neo_type type =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_VM_TYPE_INT16);
  if (!type) {
    neo_context_throw(ctx,
                      create_neo_exception(ctx, "unsupport value type int16",
                                           NULL, __FILE__, __LINE__, 1));
  }
  return neo_context_create_value(ctx, type, &value);
}

int16_t neo_value_to_int16(neo_context ctx, neo_value value) {
  if (neo_value_get_type_name(value) != NEO_VM_TYPE_INT16) {
    neo_context_throw(ctx,
                      create_neo_exception(ctx, "unsupport value type boolean",
                                           NULL, __FILE__, __LINE__, 1));
  }
  int16_t *data = (int16_t *)neo_value_get_data(value);
  return *data;
}