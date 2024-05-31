#include "type/float.h"
#include "context.h"
#include "runtime.h"
#include "type.h"
#include "type/exception.h"
#include "typedef.h"
#include "value.h"

void neo_init_float(neo_runtime runtime) {
  neo_type neo_float = create_neo_type(NEO_VM_TYPE_FLOAT, sizeof(double), NULL);
  neo_runtime_define_type(runtime, neo_float);
}
neo_value create_neo_float(neo_context ctx, double value) {
  neo_type neo_float =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_VM_TYPE_FLOAT);
  if (!neo_float) {
    neo_context_throw_exception(ctx, "unsupport value type float");
  }
  return neo_context_create_value(ctx, neo_float, &value);
}
double neo_value_to_float(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_VM_TYPE_FLOAT);
  return *(double *)neo_value_get_data(value);
}
void neo_float_set_value(neo_context ctx, neo_value value, double val) {
  CHECK_TYPE(NEO_VM_TYPE_FLOAT);
  *(double *)neo_value_get_data(value) = val;
}