#include "type/number.h"
#include "context.h"
#include "runtime.h"
#include "type.h"
#include "typedef.h"
#include "value.h"

void neo_number_init(neo_runtime runtime) {
  neo_type type = create_neo_type(NEO_TYPE_NUMBER, sizeof(double), NULL);
  neo_runtime_define_type(runtime, type);
}
double neo_value_get_number(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_TYPE_NUMBER);
  return *(double *)neo_value_get_data(value);
}
void neo_value_set_number(neo_context ctx, neo_value value, double val) {
  CHECK_TYPE(NEO_TYPE_NUMBER);
  *(double *)neo_value_get_data(value) = val;
}
neo_value create_neo_number(neo_context ctx, double val) {
  neo_type type =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_TYPE_NUMBER);
  return neo_context_create_value(ctx, type, &val);
}