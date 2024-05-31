#include "type/integer.h"
#include "context.h"
#include "runtime.h"
#include "type.h"
#include "type/exception.h"
#include "typedef.h"
#include "value.h"
#include <stdint.h>

void neo_init_integer(neo_runtime runtime) {
  neo_type neo_integer =
      create_neo_type(NEO_VM_TYPE_INTEGER, sizeof(int64_t), NULL);
  neo_runtime_define_type(runtime, neo_integer);
}
neo_value create_neo_integer(neo_context ctx, int64_t value) {
  neo_type neo_integer =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_VM_TYPE_INTEGER);
  if (!neo_integer) {
    neo_context_throw_exception(ctx, "unsupport value type integer");
  }
  return neo_context_create_value(ctx, neo_integer, &value);
}
int64_t neo_value_to_integer(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_VM_TYPE_INTEGER);
  return *(int64_t *)neo_value_get_data(value);
}
void neo_integer_set_value(neo_context ctx, neo_value value, int64_t val) {
  CHECK_TYPE(NEO_VM_TYPE_INTEGER);
  *(int64_t *)neo_value_get_data(value) = val;
}