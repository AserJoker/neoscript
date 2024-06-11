#include "type/null.h"
#include "common/include/cstring.h"
#include "context.h"
#include "runtime.h"
#include "type.h"
#include "typedef.h"

static int8_t neo_convert_null(void *data, uint32_t type, void *output,
                               void *_) {
  switch (type) {
  case NEO_TYPE_BOOLEAN:
    *(int8_t *)output = 0;
    return 1;
  case NEO_TYPE_NUMBER:
    *(double *)output = 0;
    return 1;
  case NEO_TYPE_STRING:
    *(cstring *)output = cstring_clone("null");
    return 1;
  }
  return 0;
}
void neo_null_init(neo_runtime rt) {
  neo_type neo_null = create_neo_type(NEO_TYPE_NULL, 0, NULL);
  neo_type_hook *hook = neo_type_get_hook(neo_null);
  hook->convert = &neo_convert_null;
  neo_runtime_define_type(rt, neo_null);
}
neo_value create_neo_null(neo_context ctx) {
  neo_type neo_null =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_TYPE_NULL);
  return neo_context_create_value(ctx, neo_null, NULL);
}