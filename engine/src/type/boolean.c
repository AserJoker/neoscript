#include "type/boolean.h"
#include "context.h"
#include "runtime.h"
#include "type.h"
#include "typedef.h"
#include "value.h"
#include <stdint.h>
void neo_boolean_init(neo_runtime rt) {
  neo_type neo_boolean =
      create_neo_type(NEO_TYPE_BOOLEAN, sizeof(int8_t), NULL);
  neo_runtime_define_type(rt, neo_boolean);
}
int8_t neo_value_get_boolean(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_TYPE_BOOLEAN);
  return *(int8_t *)neo_value_get_data(value);
}
void neo_value_set_boolean(neo_context ctx, neo_value value, int8_t val) {
  CHECK_TYPE(NEO_TYPE_BOOLEAN);
  *(int8_t *)neo_value_get_data(value) = val;
}
neo_value create_neo_boolean(neo_context ctx, int8_t val) {
  neo_type neo_boolean =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_TYPE_BOOLEAN);
  return neo_context_create_value(ctx, neo_boolean, &val);
}