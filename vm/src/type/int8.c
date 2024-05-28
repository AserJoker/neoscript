#include "type/int8.h"
#include "atom.h"
#include "context.h"
#include "runtime.h"
#include "type/exception.h"
#include "typedef.h"
#include "value.h"
#include <stdint.h>
#include <stdio.h>

void neo_int8_init(void *target, void *source, void *_) {
  int8_t *dst = (int8_t *)target;
  int8_t *src = (int8_t *)source;
  *dst = *src;
}

void neo_init_int8(neo_runtime runtime) {
  neo_type_hook hook = {neo_int8_init,     NULL, NULL, NULL,
                        neo_atom_copy_val, NULL};
  neo_type neo_int8 = create_neo_type(NEO_VM_TYPE_INT8, sizeof(int8_t), &hook);
  neo_runtime_define_type(runtime, neo_int8);
}

neo_value create_neo_int8(neo_context ctx, int8_t value) {
  neo_type type =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_VM_TYPE_INT8);
  if (!type) {
    neo_context_throw(ctx,
                      create_neo_exception(ctx, "unsupport value type int8",
                                           NULL, __FILE__, __LINE__, 1));
  }
  return neo_context_create_value(ctx, type, &value);
}

int8_t neo_value_to_int8(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_VM_TYPE_INT8);
  int8_t *data = (int8_t *)neo_value_get_data(value);
  return *data;
}