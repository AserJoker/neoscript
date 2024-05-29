#include "atom.h"
#include "context.h"
#include "runtime.h"
#include "type/exception.h"
#include "type/float32.h"
#include "typedef.h"
#include "value.h"
#include <stdint.h>
#include <stdio.h>

void neo_float32_init(void *target, void *source, void *_) {
  float *dst = (float *)target;
  float *src = (float *)source;
  *dst = *src;
}

void neo_init_float32(neo_runtime runtime) {
  neo_type_hook hook = {neo_float32_init,  NULL, NULL, NULL,
                        neo_atom_copy_val, NULL};
  neo_type neo_float32 =
      create_neo_type(NEO_VM_TYPE_FLOAT32, sizeof(float), &hook);
  neo_runtime_define_type(runtime, neo_float32);
}

neo_value create_neo_float32(neo_context ctx, float value) {
  neo_type type =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_VM_TYPE_FLOAT32);
  if (!type) {
    neo_context_throw(ctx,
                      create_neo_exception(ctx, "unsupport value type float32",
                                           NULL, __FILE__, __LINE__, 1));
  }
  return neo_context_create_value(ctx, type, &value);
}

float neo_value_to_float32(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_VM_TYPE_FLOAT32);
  float *data = (float *)neo_value_get_data(value);
  return *data;
}