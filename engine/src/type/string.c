#include "type/string.h"
#include "atom.h"
#include "common/include/cstring.h"
#include "context.h"
#include "runtime.h"
#include "type.h"
#include "typedef.h"
#include "value.h"
#include <stdlib.h>
#include <string.h>
typedef struct _neo_string_impl *neo_string_impl;
struct _neo_string_impl {
  cstring data;
};
static void neo_init_string(void *target, void *source, void *_) {
  neo_string_impl dst = (neo_string_impl)target;
  neo_string_impl src = (neo_string_impl)source;
  dst->data = cstring_clone(src->data);
}
static void neo_dispose_string(void *target, void *_) {
  neo_string_impl dst = (neo_string_impl)target;
  free(dst->data);
}
static int8_t neo_convert_string(void *data, uint32_t type, void *output,
                                 void *_) {
  switch (type) {
  case NEO_TYPE_BOOLEAN:
    *(int8_t *)output = strlen(((neo_string_impl)data)->data) != 0;
    return 1;
  case NEO_TYPE_STRING: {
    *(cstring *)output = cstring_clone(((neo_string_impl)data)->data);
    return 1;
  }
  }
  return 0;
}
void neo_string_init(neo_runtime runtime) {
  neo_type_hook hook = {neo_init_string,   0, neo_dispose_string, 0,
                        neo_atom_copy_val, 0, neo_convert_string, 0};
  neo_type neo_string =
      create_neo_type(NEO_TYPE_STRING, sizeof(struct _neo_string_impl), &hook);
  neo_runtime_define_type(runtime, neo_string);
}
const cstring neo_value_get_string(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_TYPE_STRING);
  return ((neo_string_impl)neo_value_get_data(value))->data;
}
void neo_value_set_string(neo_context ctx, neo_value value, const cstring val) {
  CHECK_TYPE(NEO_TYPE_STRING);
  neo_string_impl impl = (neo_string_impl)neo_value_get_data(value);
  if (impl->data != val) {
    free(impl->data);
    impl->data = cstring_clone(val);
  }
}

neo_value create_neo_string(neo_context ctx, const cstring val) {
  neo_type type =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_TYPE_STRING);
  struct _neo_string_impl impl = {(cstring)val};
  return neo_context_create_value(ctx, type, &impl);
}