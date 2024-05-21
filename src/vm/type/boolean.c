#include "vm/type/boolean.h"
#include "engine/atom.h"
#include "engine/context.h"
#include "engine/runtime.h"
#include "engine/type.h"
#include "engine/type/exception.h"
#include "engine/value.h"
#include "vm/type.h"
#include <stdint.h>

void neo_boolean_init(void *target, void *source, void *_) {
  int8_t *dst = (int8_t *)target;
  int8_t *src = (int8_t *)source;
  *dst = *src;
}

void neo_init_boolean(neo_runtime runtime) {
  neo_type_hook hook = {neo_boolean_init,  NULL, NULL, NULL,
                        neo_atom_copy_val, NULL};
  neo_type neo_boolean =
      create_neo_type(NEO_VM_TYPE_BOOLEAN, sizeof(int8_t), &hook);
  neo_runtime_define_type(runtime, neo_boolean);
}

neo_value create_neo_boolean(neo_context ctx, int8_t value) {
  neo_type type =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_VM_TYPE_BOOLEAN);
  if (!type) {
    neo_context_throw(ctx,
                      create_neo_exception(ctx, "unsupport value type boolean",
                                           NULL, __FILE__, __LINE__, 1));
  }
  return neo_context_create_value(ctx, type, &value);
}

int8_t neo_value_to_boolean(neo_context ctx, neo_value value) {
  if (neo_value_get_type_name(value) != NEO_VM_TYPE_BOOLEAN) {
    neo_context_throw(ctx,
                      create_neo_exception(ctx, "unsupport value type boolean",
                                           NULL, __FILE__, __LINE__, 1));
  }
  int8_t *data = (int8_t *)neo_value_get_data(value);
  return *data;
}