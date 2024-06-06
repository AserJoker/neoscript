#include "type/number.h"
#include "common/include/strings.h"
#include "context.h"
#include "runtime.h"
#include "type.h"
#include "typedef.h"
#include "value.h"
static int8_t neo_convert_number(void *data, uint32_t type, void *output,
                                 void *_) {
  switch (type) {
  case NEO_TYPE_BOOLEAN:
    *(int8_t *)output = *(double *)data != 0;
    return 1;
  case NEO_TYPE_NUMBER:
    *(double *)output = *(double *)data;
    return 1;
  case NEO_TYPE_STRING: {
    char buf[64];
    sprintf(buf, "%g", *(double *)data);
    *(char **)output = strings_clone(buf);
    return 1;
  }
  }
  return 0;
}
void neo_number_init(neo_runtime runtime) {
  neo_type type = create_neo_type(NEO_TYPE_NUMBER, sizeof(double), NULL);
  neo_type_hook *hook = neo_type_get_hook(type);
  hook->convert = &neo_convert_number;
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