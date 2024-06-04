#include "type/null.h"
#include "context.h"
#include "runtime.h"
#include "type.h"
#include "typedef.h"

void neo_null_init(neo_runtime rt) {
  neo_type neo_null = create_neo_type(NEO_TYPE_NULL, 0, NULL);
  neo_runtime_define_type(rt, neo_null);
}
neo_value create_neo_null(neo_context ctx) {
  neo_type neo_null =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_TYPE_NULL);
  return neo_context_create_value(ctx, neo_null, NULL);
}