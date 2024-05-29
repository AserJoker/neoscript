#include "type/float64.h"
#include "atom.h"
#include "context.h"
#include "runtime.h"
#include "type/exception.h"
#include "typedef.h"
#include "value.h"
#include <stdint.h>
#include <stdio.h>

void neo_float64_init(void *target, void *source, void *_) {
  double *dst = (double *)target;
  double *src = (double *)source;
  *dst = *src;
}

void neo_init_float64(neo_runtime runtime) {
  neo_type_hook hook = {neo_float64_init,  NULL, NULL, NULL,
                        neo_atom_copy_val, NULL};
  neo_type neo_float64 =
      create_neo_type(NEO_VM_TYPE_FLOAT64, sizeof(double), &hook);
  neo_runtime_define_type(runtime, neo_float64);
}

neo_value create_neo_float64(neo_context ctx, double value) {
  neo_type type =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_VM_TYPE_FLOAT64);
  if (!type) {
    neo_context_throw(ctx,
                      create_neo_exception(ctx, "unsupport value type float64",
                                           NULL, __FILE__, __LINE__, 1));
  }
  return neo_context_create_value(ctx, type, &value);
}

double neo_value_to_float64(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_VM_TYPE_FLOAT64);
  double *data = (double *)neo_value_get_data(value);
  return *data;
}